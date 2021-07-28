/*
 * robin 2021-07-28
**/

#include "../include/ConnectionManager.h"
#include "../include/CommonHeader.h"

using namespace robin;

ConnectionManager::ConnectionManager()
{
}

void ConnectionManager::start(int n)
{
	if (n < IO_LOOPS)
		n = IO_LOOPS;
	if (n < 1)
		n = 1;

	if (loopVector.size() > 0)
		return;

	for (int i = 0; i < n; i++)
	{
		EventLoopPtr ptr = std::make_shared<EventLoop>();
		loopVector.push_back(ptr);
		ssize_t index = loopVector.size();
		threadVector.push_back(std::thread([=]()
		{

			printf("io thread %jd start\n", index);
			ptr->run();
			printf("io thread %jd exit\n", index);
		}));
	}

	// test workpool in each EventLoop
	for (size_t n=0; n< loopVector.size(); n++)
	{
		uv_loop_t * loop = loopVector[n]->handle();
		loop->data = (void *)n;
		loopVector[n]->runInLoop([=]()
		{
			initWorkerPool(loop);
		});
	}
}

// should init worker pool first, or the first call will be slow
void ConnectionManager::initWorkerPool(uv_loop_t * loop)
{
	uv_work_t * work_req = new uv_work_t();
	work_req->data = loop->data;
	uv_queue_work(loop, work_req,
		[](uv_work_t *req)
	{
		printf("worker threads init ok, by loop  %zu ...\n", (size_t)req->data +1);
	},
		[](uv_work_t *req, int status)
	{
		delete req;
	});
}

void ConnectionManager::stop()
{
	printf("try to stop io loops\n\n");
	for (auto & ptr : loopVector)
	{
		ptr->stop();
	}
	printf("waiting...\n");
	for (auto & t : threadVector)
	{
		if (t.joinable())
			t.join();
	}
}

ConnectionManager::~ConnectionManager()
{
}


// round robin find next loop
// not thread safe here, but whatever,
EventLoopPtr & ConnectionManager::getNextLoop()
{
	round_robin++;
	round_robin = round_robin % IO_LOOPS;
	DEBUG_PRINT("current socket gets loop %zu \n", round_robin);
	return loopVector[round_robin];
}

// called in TcpServer loop
void   ConnectionManager::onNewConnection(uv_stream_t *server, int status)
{
	EventLoopPtr &loop = getNextLoop();
	TcpConnectionPtr ptr = std::make_shared<TcpConnection>(loop);
	uv_tcp_t *client = ptr->getUvClient();
	client->data = ptr.get();

	int ret = uv_tcp_init(loop->handle(), client);
	ret = uv_accept(server, (uv_stream_t*)client);

	// add to map to manage it, and free Connection when closed, see ~TcpConnection be called or not
	std::string ip = ptr->getKey();
	addConnection(ip, ptr);
	ptr->setBufferSize((uv_handle_t*)client);
	string info = ptr->getKey() + " connected...";
	LOG_INFO(info.c_str());
	
	// must be called run in loop
	loop->runInLoopEn([=]() {
		if (ret == 0)
		{
			uv_read_start((uv_stream_t*)client, TcpConnection::onAllocBuffer, TcpConnection::onRead);	
		}
		else
		{
			uv_close((uv_handle_t*)client, TcpConnection::onClose);
		}

	});// end of run in loop
}

void ConnectionManager::runInLoop(DefaultCallback cb)
{
	EventLoopPtr &loop = getNextLoop();
	loop->runInLoop(cb);
}

// add to queue
size_t ConnectionManager::addConnection(std::string &key, TcpConnectionPtr& ptr)
{
	std::lock_guard < std::mutex> lockGuard(mapMutex);
	connMap.insert(std::pair<std::string, TcpConnectionPtr>(key, ptr));
	return connMap.size();
}

void ConnectionManager::printSpeed()
{
	std::lock_guard < std::mutex> lockGuard(mapMutex);
	if (connMap.size() > 0)
	{
		printf("-------------speed-------------------\n");
		for (const auto& connPtr : connMap)
		{
			printf("%s\t %ju, %ju\n", 
				connPtr.second->getKey().c_str(), 
				connPtr.second->getRecvCount(), 
				connPtr.second->getRecvDelta());
		}
		printf("-------------------------------------\n\n\n");
	}
	
}

void ConnectionManager::printPool()
{
	std::lock_guard < std::mutex> lockGuard(mapMutex);
	printf("-------------connection pool-------------------\n");
	for (const auto& connPtr  : connMap)
	{
		printf("%s\t %ju, %ju\n", 
			connPtr.second->getKey().c_str(),
			connPtr.second->getRecvCount(), 
			connPtr.second->getRecvDelta());
	}
	printf("------------connection pool end----------------\n");
}

// 删除连接的指针，
bool ConnectionManager::freeConnection(TcpConnectionPtr &conn)
{
	string key = conn->getKey();
	bool ret = freeConnection(key);
	printPool();
	return ret;
}

bool   ConnectionManager::freeConnection(std::string & key)
{
	std::lock_guard <std::mutex> lockGuard(mapMutex);
	auto it = connMap.find(key);
	if (it != connMap.end())
	{
		connMap.erase(it);
		return true;
	}
	return false;
}

