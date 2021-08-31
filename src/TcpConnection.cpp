/*
 * robin 2021-07-28
 * this class is most important one in the package, 
 * client will use it as a client,
 * while server will use it as a normal connection to clients
**/

#include "../include/TcpConnection.h"
#include "../include/SocketAddr.h"
#include "../include/ITask.h"
#include "../include/GlobalConfig.h"
#include "../include/ConnectionManager.h"
#include "../utils/Queue.h"

#include <assert.h>
#pragma warning(disable:4267)
#pragma warning(disable:4244)
using namespace robin;

// used for read and alloc
void TcpConnection::onAllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	BufferPool * pool = BufferPool::instance();
	buf->base = pool->getReadBuffer(buf->len);
}
void TcpConnection::onFreeReadBuffer(uv_buf_t *buf)
{
	BufferPool * pool = BufferPool::instance();
	pool->putReadBuffer(buf->base);
}

///////////////////////////////////////////////////////////////////
TcpConnection::TcpConnection(const EventLoopPtr& loop, bool tcpNoDelay) :
	loopPtr(loop),
	bClient(false), 
	bConnected(false),
	bNoDelay(tcpNoDelay),
	sendQue(256),
	bClosed(false)
{
	remote.data = this;
	local.data = this;
	connect_req.data = this;
	sendQue.clear();
}
TcpConnection::~TcpConnection()
{
	// should free all vec in queue, or will memory leak
	std::lock_guard<std::mutex> guard(taskMutex);
	for (auto& t : sendQue)
	{
		BufferPool::instance()->putWriteBuffer(t);
	}
	sendQue.clear();

	printf("~TcpConnection() \n");
}

void TcpConnection::start()
{
	loopPtr->run();
}

void TcpConnection::stop()
{
	loopPtr->stop();
}

void  TcpConnection::clearCb()
{
	this->connCb = nullptr;
	this->sendCb = nullptr;
	this->closeCb = nullptr;

}
// return delta speed, between 2 time points
uint64_t  TcpConnection::getRecvDelta()
{
	uint64_t delta = (this->recvCount - this->recvCountLast);
	recvCountLast = recvCount;
	return delta;
}

uv_loop_t * TcpConnection::getLoop()
{
	return loopPtr->handle();
}
string TcpConnection::getKey()
{
	if (infoKey == "")
		SocketAddr::AddrToStr(&remote, infoKey, SocketAddr::Ipv4);

	return infoKey;
}
// encode by Encoder
write_req_vec_t * TcpConnection::encodeTask(TaskPtr& task)
{
	write_req_vec_t *wreq = BufferPool::instance()->getWriteBuffer();
	assert(wreq != nullptr);

	wreq->vecBuf.clear();
	wreq->taskPtr = task;


	// encode task to json or protobuf or something else
	GlobalConfig::getEncoder()->encodeTask(wreq->vecBuf, task);

	int ret = 0;

	// must set buf_t after encoding
	wreq->buf = uv_buf_init(wreq->vecBuf.data(), wreq->vecBuf.size());
	assert(wreq->buf.base == wreq->vecBuf.data());
	assert(wreq->buf.len == wreq->vecBuf.size());
	return wreq;
}
// on error, lib should return the packet to let user know
void TcpConnection::pushbackQue(write_req_vec_t * req)
{
	std::lock_guard<std::mutex> guard(taskMutex);
	sendQue.push_back(req);
}


// for speed, we should encode in worker thread;
void TcpConnection::pushinQue(TaskPtr& task)
{
	write_req_vec_t * req = encodeTask(task);
	std::lock_guard<std::mutex> guard(taskMutex);
	//taskQue.push_back(task);
	sendQue.push_back(req);
}

// only req* is needed
write_req_vec_t * TcpConnection::popQue()
{
	std::lock_guard<std::mutex> guard(taskMutex);
	if (sendQue.size() > 0)
	{
		write_req_vec_t * req = sendQue[0];
		sendQue.pop_front();

		//TaskPtr ptr = taskQue[0];
		//taskQue.pop_front();

		return req;
	}
	return nullptr;
}

void TcpConnection::swapQue(std::deque< write_req_vec_t *> &tempQue)
{
	std::lock_guard<std::mutex> guard(taskMutex);
	sendQue.swap(tempQue);
}

// uv_loop use async event to call here
void TcpConnection::realSend()
{
	int ret = 0;
	std::deque< write_req_vec_t *> tempQue;

	// lock auto the queue
	{
		std::lock_guard<std::mutex> guard(taskMutex);
		sendQue.swap(tempQue);
	}

	bool bError = false;
	for (size_t i = 0; i < tempQue.size(); i++)
	{
		write_req_vec_t * req = tempQue[i];
		assert(req);
		// check error while send N packets
		if (bError)
		{
			// push back to deque
			pushbackQue(req);
			continue;
		}

		// send it 
		if (bClient == true)   // as a client
		{
			ret = uv_write(req,
				(uv_stream_t*) &(this->local),
				&req->buf, 1,
				TcpConnection::afterWrite);

		}
		else  // work as server
		{
			ret = uv_write(req,
				(uv_stream_t *)&remote,
				&req->buf, 1,
				TcpConnection::afterWrite);
		}

		if (ret != 0)
		{
			bConnected = false;
			bError = true;
			//FORMAT_DEBUG("send failed %d\, %s n", ret, this->getKey().c_str());
			//printf("send failed ret=%d: %s \t\n",  ret, wreq->vecBuf.data());
			FORMAT_ERROR("send failed %d, close soon\n", ret);

			// free
			//BufferPool::instance()->putWriteBuffer(req);
			
			// push back to deque
			pushbackQue(req);	
			close(ret);
		}
	}// end for

	
}
// send a event to send next;
void TcpConnection::invokeSend()
{
	loopPtr->runInLoop([&]() {
		// pop one, send one by one
		//printf("invoke sendMsg callback\n");
		this->realSend();
	});
}

void TcpConnection::sendMsg(write_req_vec_t * req)
{
	pushbackQue(req);

	loopPtr->runInLoopEn([&]() {
		this->realSend();
	});
}

void TcpConnection::sendMsg(TaskPtr task)
{
	//char buf[260];
	//double delta = task->markMid1();
	//snprintf(buf, 260, "%s task encode_pre=%f ms", task->taskIdStr.c_str(), delta);
	//LOG_DEBUG(buf);

	// encode & push them in que,
	pushinQue(task);

	//delta = task->markMid1();
	//snprintf(buf, 260, "%s task encode_post=%f ms", task->taskIdStr.c_str(), delta);
	//LOG_DEBUG(buf);

	if (loopPtr->isRunInLoopThread())
	{
		//LOG_DEBUG("run direct...");
		this->realSend();
	}
	else
	{
		loopPtr->runInLoopEn([&]() {
			this->realSend();
		});
	}
	
}

// if send failed, it must be something wrong with connection, so close the socket and notify user
void TcpConnection::afterWrite(uv_write_t *req, int status)
{
	assert(req != nullptr);
	write_req_vec_t *req_vec = (write_req_vec_t *)req;
	TcpConnectionPtr conn = req_vec->taskPtr->getConnection();
	assert(conn);
	if (!conn)  // tcp connection is deleted before here
	{
		BufferPool::instance()->putWriteBuffer(req_vec);
		return;
	}

	// notify user, user 
	if (conn->getSendCb() != nullptr)
		conn->getSendCb()(status, req_vec->taskPtr, conn);


	if (status) // failed
	{
		conn->setConnectStatus(false);
		// should push back
		conn->pushbackQue(req_vec);

		LOG_ERROR("Write after error");
		// close the socket;
		conn->close(status);

	}
	else
	{
		conn->sentCount++;
		conn->realSend();
		BufferPool::instance()->putWriteBuffer(req_vec);
	}

	
}
// send finished, so complex here, then go on reading
///////////////////////////////////////////////////////////////////////
void TcpConnection::closeSafe(int reason)
{
	bConnected = false;
	//printf("close\n");
	if (loopPtr->isRunInLoopThread())
	{
		close(reason);
	}
	else
	{
		loopPtr->runInLoopEn([=]()
		{
			close(reason);
		});
	}
}


void TcpConnection::close(int reason)
{
	bConnected = false;

	if (bClient == true)  // as client
	{
		if (uv_is_closing((uv_handle_t * ) &(this->local)))
		{
			return;
		}
		uv_close((uv_handle_t*)&(this->local), TcpConnection::onClose);
	}
	else
	{
		if (uv_is_closing((uv_handle_t *)&remote))
		{
			return;
		}
		uv_close((uv_handle_t*)&remote, TcpConnection::onClose);
	}
}
bool TcpConnection::connect(const char *ip, unsigned short port)
{
	remoteAddr = make_shared<SocketAddr>(ip, port);
	return this->connect(remoteAddr.get());

}
bool TcpConnection::reConnect()
{
	return this->connect(remoteAddr.get());
}
// work as a client
bool TcpConnection::connect(SocketAddr *address)
{
	sentCount = 0;
	recvCount = 0;
	recvCountLast = 0;

	bClient = true;
	uv_loop_t *loop = loopPtr->handle();

	int ret = uv_tcp_init(loop, &local);
	// set
	//connect_req.weakConn = this->shared_from_this();
	connect_req.data = this;
	local.data = this;
	bClient = true;

	ret = uv_tcp_connect(&connect_req,
		&local,
		(const struct sockaddr*) address->Addr(),
		TcpConnection::onConnect);


	return true;
}

void TcpConnection::setBufferSize(uv_handle_t* handle)
{
	int sz = 1024 * 1024 * 20;
	uv_send_buffer_size((uv_handle_t*)handle, &sz);
	uv_recv_buffer_size((uv_handle_t*)handle, &sz);
}
// as a client
void TcpConnection::onConnect(uv_connect_t* req, int status)
{
	
	TcpConnection *conn = (TcpConnection *)req->data;
	TcpConnectionPtr ptr = conn->shared_from_this();
	//TcpConnectionPtr conn = ((uv_connect_t_ex *)req)->weakConn.lock();
	
	assert(conn);
	if (status < 0)
	{
		//printf("connect error %d\n", status);
		conn->bConnected = false;
		// notify up leverl user to know
		ConnectCallback cb = conn->getConnectCb();
		if (cb)
		{
			cb(status, ptr);
		}
		return;
	}

	conn->setBufferSize((uv_handle_t*)req->handle);

	assert(status == 0);

	assert(1 == uv_is_readable(req->handle));
	assert(1 == uv_is_writable(req->handle));
	assert(0 == uv_is_closing((uv_handle_t *)req->handle));

	//size_t off = offsetof(uv_connect_t, handle);
	//off = (char*)&(req->handle) - (char*)req ;
	uv_read_start((uv_stream_t*) &(conn->local),            //req->handle,
		TcpConnection::onAllocBuffer,
		TcpConnection::onRead);

	conn->bConnected = true;

	// allow user to send the first HELLO packet
	ConnectCallback cb = conn->getConnectCb();
	if (cb)
	{
		cb(status, ptr);
	}
}

void  TcpConnection::onRead(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	TcpConnection * ptr = (TcpConnection *)client->data;
	TcpConnectionPtr conn = ptr->shared_from_this();
	//size_t count = conn.use_count();
	// test the conn ptr
	assert(conn);

	if (nread <= 0)
	{

		if (nread == 0)
		{
			fprintf(stderr, "read 0 ! \n");
			//conn->close(nread);
			goto read_free;
		}
		else if (nread == UV_EOF)
		{
			//std::cout << "connection is closed by remote!" << endl;
			fprintf(stderr, "connect closed by remote! \n");
		}

		else if (nread == -4077 || nread == -104)   // windows & linux
		{
			fprintf(stderr, "connect reset by remote!\n");
		}
		else
		{
			fprintf(stderr, "Read error %s\n", uv_err_name(nread));
		}
		conn->close(nread);
		goto read_free;
	}
	

	if (nread > 0)
	{
		std::shared_ptr<IDispatcher> dispatcher = GlobalConfig::getMsgDispatcher();
		if (dispatcher) // check it
		{
			dispatcher->onMessage(conn, buf->base, nread);
		}
	}
read_free:
	// must free buffer
	if (buf == nullptr || buf->base == 0)
	{
		// when (nread == -4077) note that here buf is null
		fprintf(stderr, "read null ptr ! \n");
		conn->close(nread);
	}
	else
	{	// use static function
		TcpConnection::onFreeReadBuffer(const_cast<uv_buf_t *>(buf));
	}

}

// close finished, notify user or server
void TcpConnection::onClose(uv_handle_t* handle)
{
	TcpConnection * ptr = (TcpConnection*)handle->data;
	TcpConnectionPtr conn = ptr->shared_from_this();
	conn->setClosed(true);
	assert(conn != nullptr);


	if (!conn)
		return;
	
	if (conn->getCloseCb())
		conn->getCloseCb()(conn);


	// as a session of the server
	if (conn->isClient() == false)
	{
		printf("close socket: %s\n", conn->getKey().c_str());
		ConnectionManager::instance()->freeConnection(conn);
	}
}

