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
		n = 2;

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

	if (loopVector.size() > 0)
	{
		uv_loop_t * loop = loopVector[0]->handle();
		loopVector[0]->runInLoop([=]()
		{
			initWorkerPool(loop);
		});
	}
}

// should init worker pool first, or the first call will be slow
void ConnectionManager::initWorkerPool(uv_loop_t * loop)
{
	uv_work_t * work_req = new uv_work_t();
	uv_queue_work(loop, work_req,
		[](uv_work_t *req)
	{
		printf("worker threads init ok...\n");
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
EventLoopPtr ConnectionManager::getNextLoop()
{
	round_robin++;
	round_robin = round_robin % IO_LOOPS;
	return loopVector[round_robin];
}

// called in TcpServer loop
void   ConnectionManager::onNewConnection(uv_stream_t *server, int status)
{
	EventLoopPtr loop = getNextLoop();

	TcpConnection *ptr = new TcpConnection(loop);
	uv_tcp_t *client = ptr->getUvClient();

	int ret = uv_tcp_init(loop->handle(), client);

	ret = uv_accept(server, (uv_stream_t*)client);

	if (ret == 0)
	{
		// add to map to manage it 
		std::string ip = ptr->getKey();
		addConnection(ip, ptr);
		uv_read_start((uv_stream_t*)client, TcpConnection::onAllocBuffer, TcpConnection::onRead);
	}
	else
	{
		uv_close((uv_handle_t*)client, TcpConnection::onClose);
	}

}

void ConnectionManager::runInLoop(DefaultCallback cb)
{
	EventLoopPtr loop = getNextLoop();
	loop->runInLoop(cb);
}

// 添加到列表
size_t ConnectionManager::addConnection(std::string &key, TcpConnection * ptr)
{
	std::lock_guard < std::mutex> lockGuard(mapMutex);
	connMap.insert(std::pair<std::string, TcpConnection*>(key, ptr));
	return connMap.size();
}

void ConnectionManager::printPool()
{
	std::lock_guard < std::mutex> lockGuard(mapMutex);
	printf("-------------connection pool-------------------\n");
	for (const auto connPtr : connMap)
	{

		printf("%s\n", connPtr.second->getKey().c_str());
	}
	printf("------------connection pool end----------------\n");
}

// 删除连接的指针，
bool ConnectionManager::freeConnection(TcpConnection * conn)
{
	string key = conn->getKey();
	return freeConnection(key);
}
bool   ConnectionManager::freeConnection(std::string & key)
{
	std::lock_guard <std::mutex> lockGuard(mapMutex);
	auto it = connMap.find(key);
	if (it != connMap.end())
	{
		delete it->second;
		connMap.erase(it);
		return true;
	}

	return false;
}

