#include "../../../include/CommonHeader.h"
#include "../../../include/EventLoop.h"
#include "../../../include/TcpConnection.h"
#include "../../../include/GlobalConfig.h"
#include "../../../include/ConnectionManager.h"
#include "../../../include/TcpServer.h"
#include "../../../utils/Timer.h"

#include "WorkAddNum.h"
#include "PongEncode.h"
#include "PingDispatcher.h"
#include "PingDispatch2Worker.h"

#if defined(WIN32) || defined(_WIN64)
#include "../utils/cmdsetting_w32.h"
#else
#endif


using namespace robin;


// client send "ping" to server,
// server response "pong" to client
// return pong in dispatcher
void serverPong()
{
	GlobalConfig::init();
	// 1）user should rewrite this class，override virtual funciton  'onMessageParse()'；
	std::shared_ptr<PingDispatcher> dispatcher = std::make_shared<PingDispatcher>();
	GlobalConfig::setMsgDispatcher(dispatcher);


	// 1.1)or set the callback to parse data-part of the packet
	//dispatcher->msgCb = [&](char *buf, unsigned long len)
	//{
	//	uint64_t id = _atoi64(buf);
	//	//findTask(id);
	//};

	// 2）set message encoder before send 
	std::shared_ptr<PongEncode> encoder = std::make_shared<PongEncode>();
	GlobalConfig::setEncoder(encoder);

	TcpServer server(5);
	server.bindAndListen("0.0.0.0", 80, 128);

	server.start();
}

// decode in dispatcher, and do work in worker thread
void serverPongEn()
{
	GlobalConfig::init();
	// 1）user should rewrite this class，override virtual funciton  'onMessageParse()'；
	std::shared_ptr<PingDispatch2Worker> dispatcher = std::make_shared<PingDispatch2Worker>();
	GlobalConfig::setMsgDispatcher(dispatcher);

	std::shared_ptr<WorkAddNum> workerPtr = std::make_shared<WorkAddNum>();
	GlobalConfig::addWorkType("IWork", std::dynamic_pointer_cast<IWork>(workerPtr));


	// 2）set message encoder before send 
	std::shared_ptr<PongEncode> encoder = std::make_shared<PongEncode>();
	GlobalConfig::setEncoder(encoder);

	TcpServer server(5);
	server.bindAndListen("0.0.0.0", 80, 128);

	server.start();
}


int main()
{
	//std::thread serverThread = thread(serverPong);

	
	std::thread serverThread = thread(serverPongEn);
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	int n = get_thread_amount();
	printf("threads %d\n", n);



	if (serverThread.joinable())
	{
		serverThread.join();
	}
	std::cout << "server is stopped!\n";
}

