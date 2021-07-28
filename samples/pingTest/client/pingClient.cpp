// 
#include "../../../include/CommonHeader.h"
#include "../../../include/EventLoop.h"
#include "../../../include/TcpConnection.h"
#include "../../../include/GlobalConfig.h"

#include "PingEncode.h"
#include "PongDispatcher.h"

#include <iostream>

using namespace robin;

#define  SEND_COUNT 50
// msg id
std::atomic<uint64_t> curNum(0);
std::atomic<uint64_t> retNum (0);
double allDelta = 0.0;
Timer timer;

// taskmap to match returned message

shared_ptr<PathlossTask> taskVec[100000];
//std::map<uint64_t, shared_ptr<PathlossTask> > taskMap;
//std::mutex taskMutex;

TcpConnectionPtr conn;

void putTask(uint64_t id, shared_ptr<PathlossTask>& task)
{
	taskVec[id] = task;
	if (id == 0)
		timer.start();
}
void findTask(uint64_t id)
{
	if (taskVec[id])
	{
		allDelta += taskVec[id]->delta();
		retNum++;
	}

	if (retNum == SEND_COUNT)
	{
		double delta = timer.stop_delta <Timer::ms > ();
		printf("total time %f ms\n", delta);
	}
}
// add task to map
//bool putTask(uint64_t id, shared_ptr<PathlossTask> task)
//{
//	std::lock_guard<std::mutex> guard(taskMutex);
//	auto it = taskMap.find(id);
//	if (it == taskMap.end())
//	{
//		taskMap.insert(std::pair<uint64_t, shared_ptr<PathlossTask> >(id, task));
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

//void findTask(uint64_t id)
//{
//	shared_ptr<PathlossTask> task;
//	task.reset();
//	{
//		std::lock_guard<std::mutex> guard(taskMutex);
//		auto it = taskMap.find(id);
//		if (it == taskMap.end())
//		{
//			printf("did not find task\n");
//			return;
//		}
//		task = it->second;
//		allDelta += task->delta();
//		retNum++;
//
//		//taskMap.erase(it);
//	}
//	if (task)
//	{
//		//task->delta();
//		//std::lock_guard<std::mutex> guardDelay(delayMutex);
//		//delayQue.push_back(task);
//
//		//printf("%jd task：%f ms\n", task->num, task->msecs);
//	}
//}

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
	while ((conn && conn->isConnected()== false))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	/*while (curNum < 30)
	{
		SendNewTask(conn);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	printf("send finished \n");*/
	while (retNum < curNum)
	{
		printf("recvd %ju \n", retNum.load());
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// print all task times
	for (size_t i=0; i<retNum; i++)
	{
		auto task = taskVec[i];
		//printf("%3jd task：jita=%f ms\n", task->num, task->msecs);
		
		printf("%3jd task：waited=%f ms, round=%f ms, net=%f\n", task->num, task->mid1, task->msecs, task->msecs - task->mid1);
	}

	printf("recvd all: %ju \n", retNum.load());
	printf("average delta: %f \n", allDelta / retNum.load());
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
	dispatcher->msgCb = [](char *buf, unsigned long len)
	{
		uint64_t id = _atoi64(buf);
		findTask(id);
	};

	// 2）set message encoder before send 
	std::shared_ptr<PingEncode> encoder = std::make_shared<PingEncode>();
	GlobalConfig::setEncoder(encoder);

	EventLoopPtr loopPtr = std::make_shared<EventLoop>();
	conn = std::make_shared<TcpConnection>(loopPtr);
	conn->setConnectCb([](int status, TcpConnectionPtr & connPtr)
	{
		if (status < 0)
		{
			printf("connect error%d\n", status);
		}
		else
		{
			printf("connect ok\n");
			SendNewTask(connPtr);

		}
	});

	conn->setSendCb([](int status, TaskPtr& ptr, TcpConnectionPtr & connPtr)
	{
		if (curNum < SEND_COUNT)
		{
			SendNewTask(connPtr);
		}
	});

	conn->connect("192.168.30.199", 80);
	conn->start();
}

std::thread clientThread;
std::thread sendThread;
int main()
{
	
	LOG_DEBUG("start");
	FORMAT_DEBUG("test format%d", 12);
	clientThread = thread(testPing);
	//std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	
	sendThread = thread(testSend);

	if (clientThread.joinable())
	{	
		clientThread.join();
	}
    std::cout << "Hello World!\n";
}

