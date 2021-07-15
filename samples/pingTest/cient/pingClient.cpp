// 
#include "../../../include/CommonHeader.h"
#include "../../../include/EventLoop.h"
#include "../../../include/TcpConnection.h"
#include "../../../include/GlobalConfig.h"

#include "PingEncode.h"
#include "PongDispatcher.h"

#include <iostream>

using namespace robin;

// msg id
std::atomic<uint64_t> curNum = 0;
std::atomic<uint64_t> retNum = 0;
// taskmap to match returned message
std::map<uint64_t, shared_ptr<PathlossTask> > taskMap;
std::mutex taskMutex;

TcpConnectionPtr conn;

// add task to map
bool putTask(uint64_t id, shared_ptr<PathlossTask> task)
{
	std::lock_guard<std::mutex> guard(taskMutex);
	auto it = taskMap.find(id);
	if (it == taskMap.end())
	{
		taskMap.insert(std::pair<uint64_t, shared_ptr<PathlossTask> >(id, task));
		return true;
	}
	else
	{
		return false;
	}
}

void findTask(uint64_t id)
{
	shared_ptr<PathlossTask> task;
	task.reset();
	{
		std::lock_guard<std::mutex> guard(taskMutex);
		auto it = taskMap.find(id);
		if (it == taskMap.end())
		{
			printf("did not find task\n");
			return;
		}
		task = it->second;
		task->delta();
		retNum++;

		//taskMap.erase(it);
	}
	if (task)
	{
		//task->delta();
		//std::lock_guard<std::mutex> guardDelay(delayMutex);
		//delayQue.push_back(task);

		//printf("%jd task：%f ms\n", task->num, task->msecs);
	}
}

// 
void SendNewTask(TcpConnectionPtr conn)
{
	shared_ptr<PathlossTask> task = std::make_shared<PathlossTask>();
	task->num = curNum;
	curNum++;
	putTask(task->num, task);

	task->startTime = Timer::now();
	
	conn->sendMsg(dynamic_pointer_cast<ITask>(task));
}



void testSend()
{
	while (curNum < 100)
	{
		if (conn && conn->isConnected())
		{
			SendNewTask(conn);
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}
	while (retNum != curNum)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	for (auto it : taskMap)
	{
		auto task = it.second;
		printf("%jd task：%f ms\n", task->num, task->msecs);
	}
}

// client send "ping" to server,
// server response "pong" to client
void testPing()
{
	GlobalConfig::init();
	// 1）user should rewrite this class，override virtual funciton  'onMessageParse()'；
	std::shared_ptr<PongDispatcher> dispatcher = std::make_shared<PongDispatcher>();
	GlobalConfig::setMsgDispatcher(dispatcher);

	// 1.1)or set the callback to parse data-part of the packet
	dispatcher->msgCb = [&](char *buf, unsigned long len)
	{
		uint64_t id = _atoi64(buf);
		findTask(id);
	};

	// 2）set message encoder before send 
	std::shared_ptr<PingEncode> encoder = std::make_shared<PingEncode>();
	GlobalConfig::setEncoder(encoder);

	EventLoopPtr loopPtr = std::make_shared<EventLoop>();
	conn = std::make_shared<TcpConnection>(loopPtr);
	conn->setConnectCb([](int status)
	{
		if (status < 0)
		{
			printf("connect error%d\n", status);
		}
		else
		{
			printf("connect ok\n");
		}
	});

	conn->connect("127.0.0.1", 80);
	conn->start();
}



int main()
{

	std::thread clientThread = thread(testPing);
//	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	
	std::thread sendThread = thread(testSend);

	if (clientThread.joinable())
	{	
		clientThread.join();
	}
    std::cout << "Hello World!\n";
}

