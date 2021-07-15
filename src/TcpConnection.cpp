#include "../include/TcpConnection.h"
#include "../include/SocketAddr.h"
#include "../include/ITask.h"
#include "../include/GlobalConfig.h"
#include "../include/ConnectionManager.h"

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
	bNoDelay(tcpNoDelay)
{
	remote.data = this;
}
TcpConnection::~TcpConnection()
{

}

void TcpConnection::start()
{
	loopPtr->run();
}

void TcpConnection::stop()
{
	loopPtr->stop();
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

	wreq->data = static_cast<void *>(this);
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
	sendQue.push_front(req);
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
// uv_loop use async event to call here
void TcpConnection::realSend()
{
	int ret = 0;
	std::deque< write_req_vec_t *> tempQue;
	//write_req_vec_t * req = popQue();
	{
		std::lock_guard<std::mutex> guard(taskMutex);
		sendQue.swap(tempQue);
	}
	
	for (size_t i = 0; i < tempQue.size(); i++)
	{
		write_req_vec_t * req = tempQue[i];
		if (req)
		{
			// send it 
			if (bClient)
			{
				ret = uv_write(req,
					(uv_stream_t*)this->connect_req.handle,
					&req->buf, 1,
					TcpConnection::afterWrite);
			}
			else
			{
				ret = uv_write(req,
					(uv_stream_t *)&remote,
					&req->buf, 1,
					TcpConnection::afterWrite);
			}

			if (ret != 0)
			{
				//printf("send failed ret=%d: %s \t\n",  ret, wreq->vecBuf.data());
				printf("send failed %d\n", ret);
			}
		}
	}
	
	
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

void TcpConnection::sendMsg(TaskPtr task)
{
	// encode & push them in que,
	pushinQue(task);
	loopPtr->runInLoop([&]() {
		// pop one, send one by one
		//printf("sendMsg callback\n");
		this->realSend();
	});
	

}

// if send failed, it must be something wrong with connection, so close the socket and notify user
void TcpConnection::afterWrite(uv_write_t *req, int status)
{
	assert(req != nullptr);
	write_req_vec_t *req_vec = (write_req_vec_t *)req;
	TcpConnection * conn = (TcpConnection *)req_vec->data;
	//TcpConnection * conn = req_vec->taskPtr->getConnection();
	assert(conn != nullptr);

	// notify user, user 
	if (conn->getSendCb() != nullptr)
		conn->getSendCb()(status, req_vec->taskPtr);

	if (status) // failed£¬
	{
		fprintf(stderr, "Write error %s\n", uv_strerror(status));
		conn->pushbackQue(req_vec);
		// close the socket;
		conn->close(status);

	}
	else
	{
		//fprintf(stderr, "Write is ok \n");
		// free buf to bufferpool
		BufferPool::instance()->putWriteBuffer(req_vec);
		// notify loop,try to send next one
		//conn->invokeSend();
		conn->realSend();
	}
}
// send finished, so complex here, then go on reading
///////////////////////////////////////////////////////////////////////
void TcpConnection::close(int reason)
{
	// connect as client, call here in loop callback
	if (bClient)
	{
		uv_close((uv_handle_t*)this->connect_req.handle, TcpConnection::onClose);
	}
	else
	{
		uv_close((uv_handle_t*)&remote, TcpConnection::onClose);
	}
	
}
bool TcpConnection::connect(const char *ip, unsigned short port)
{
	remoteAddr = make_shared<SocketAddr>(ip, port);
	return this->connect(remoteAddr.get());

}
// work as a client
bool TcpConnection::connect(SocketAddr *address)
{
	bClient = true;
	uv_loop_t *loop = loopPtr->handle();

	int ret = uv_tcp_init(loop, &local);
	local.data = this;
	connect_req.data = this;

	ret = uv_tcp_connect(&connect_req,
		&local,
		(const struct sockaddr*) address->Addr(),
		TcpConnection::onConnect);


	return true;
}
// as a client
void TcpConnection::onConnect(uv_connect_t* req, int status)
{
	TcpConnection *conn = (TcpConnection *)req->data;
	if (status < 0)
	{
		//printf("connect error %d\n", status);
		conn->bConnected = false;
		// notify up leverl user to know
		ConnectCallback cb = conn->getConnectCb();
		if (cb)
			cb(status);
		return;
	}

	int sz = 1024 * 1024;
	uv_send_buffer_size((uv_handle_t*)req->handle, &sz);

	assert(status == 0);

	assert(1 == uv_is_readable(req->handle));
	assert(1 == uv_is_writable(req->handle));
	assert(0 == uv_is_closing((uv_handle_t *)req->handle));



	// allow user to send the first HELLO packet
	ConnectCallback cb = conn->getConnectCb();
	if (cb)
		cb(status);

	uv_read_start((uv_stream_t*)req->handle,
		TcpConnection::onAllocBuffer,
		TcpConnection::onRead);

	conn->bConnected = true;

}


void  TcpConnection::onRead(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	TcpConnection * conn = (TcpConnection *)client->data;

	if (nread > 0)
	{
		std::shared_ptr<IDispatcher> dispatcher = GlobalConfig::getMsgDispatcher();
		if (dispatcher) // check it
		{
			dispatcher->onMessage(client, buf->base, nread);
		}
	}

	if (nread < 0)
	{
		if (nread == UV_EOF)
		{
			//std::cout << "connection is closed by remote!" << endl;
		}
		
		fprintf(stderr, "Read error %s\n", uv_err_name(nread));
		conn->close(nread);
	}


	// must free buffer
	if (buf == nullptr || buf->base == 0)
	{
		// when (nread == -4077)£¬note that here buf is null
	}
	else
	{	// use static function
		TcpConnection::onFreeReadBuffer(const_cast<uv_buf_t *>(buf));
	}

}

// close finished, notify user or server
void TcpConnection::onClose(uv_handle_t* handle)
{
	TcpConnection * conn = static_cast<TcpConnection *>(handle->data);
	
	assert(conn != nullptr);
	if (conn->getCloseCb())
		conn->getCloseCb()(conn);


	// as a session of the server
	if (conn->isClient() == false)
	{
		ConnectionManager::instance()->freeConnection(conn);
	}
}

