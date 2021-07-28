/*
 * robin 2021-07-28
**/

#pragma once
#include "../../../include/CommonHeader.h"
#include "../../../include/EventLoop.h"
#include "../../../include/TcpConnection.h"
#include "../../../include/GlobalConfig.h"
#include "../../../include/UvTimer.h"

#include "PingEncode.h"
#include "PongDispatcher.h"
#include "broker.h"

#include <iostream>


using namespace robin;


Broker::Broker()
{
}
Broker::~Broker()
{
	if (threadVector.size() > 0)
		stop();
}

void Broker::printInfo()
{
	static uint64_t lastRecv = 0;
	uint64_t curRecv = 0;
	uint64_t delta = 0;
	for (auto & t : clients)
	{
		curRecv += t.conn->getRecvCount();
	}

	if (lastRecv != 0)
	{
		delta = curRecv - lastRecv;
		lastRecv = curRecv;
		
	}
	else
	{
		lastRecv = curRecv;
	}
	
	printf("recv %ju, speed:%ju \n", curRecv, delta);
}

void Broker::init()
{
	GlobalConfig::init();
	// 1£©user should rewrite this class£¬override virtual funciton  'onMessageParse()'£»
	std::shared_ptr<PongDispatcher> dispatcher = std::make_shared<PongDispatcher>();
	GlobalConfig::setMsgDispatcher(dispatcher);

	// 1.1)or set the callback to parse data-part of the packet
	dispatcher->msgCb = [](char *buf, unsigned long len)
	{
		uint64_t id = _atoi64(buf);
		//findTask(id);
		//this->recvCmd++;
	};

	// 2£©set message encoder before send 
	std::shared_ptr<PingEncode> encoder = std::make_shared<PingEncode>();
	GlobalConfig::setEncoder(encoder);

	start(CLINET_LOOPS);
}

void Broker::start(int n)
{
	bStop = false;
	if (n < CLINET_LOOPS)
		n = CLINET_LOOPS;
	if (n < 1)
		n = 1;

	if (loopVector.size() > 0)
		return;

	for (int i = 0; i < n; i++)
	{
		EventLoopPtr ptr = std::make_shared<EventLoop>();
		loopVector.push_back(ptr);
		ssize_t index = loopVector.size();
		threadVector.push_back(std::thread([ptr, index]()
		{

			printf("client io thread %jd start\n", index);
			ptr->run();
			printf("client io thread %jd exit\n", index);
		}));
	}
}

void Broker::stop()
{
	bStop = true;
	printf("try to stop sockets\n\n");
	for (auto &item : clients)
	{
		/*item.conn->setClosed(false);
		item.conn->setCloseCb([item](TcpConnectionPtr & conn)
		{
			item.conn->setClosed(true);
		});*/
		item.conn->closeSafe();
	}

	// check all sockets are closed
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		bool bFinish = true;
		for (auto &item : clients)
		{
			if (item.conn->isClosed() == false)
			{
				
				bFinish = false;
				break;
			}
		}
		if (bFinish)
			break;	
	}
	clients.clear();


	printf("try to stop io loops\n\n");
	for (auto & ptr : loopVector)
	{
		ptr->stop();
	}
	printf("waiting loop thread exit...\n");
	for (auto & t : threadVector)
	{
		if (t.joinable())
			t.join();
	}
	loopVector.clear();
	threadVector.clear();
}

void Broker::reConnect(TcpConnectionPtr & conn)
{
	conn->reConnect();
}
void Broker::reConnect(size_t idx)
{
	this->clients[idx].conn->reConnect();
}
void Broker::reConnect(NodeItem & item)
{
	// set a timer
	size_t loopIndex = item.index % loopVector.size();
	EventLoopPtr &loop = loopVector[loopIndex];
	EventLoop* pLoop = loop.get();
	printf("client %zu will reconnect  to server in 5 seconds.\n", loopIndex);
	size_t idx = item.index;
	
	loop->runInLoopEn([this, pLoop, idx]()
	{
		
		UvTimer * timer1 = new UvTimer(pLoop, 5000, 10000, [=](UvTimer* timer)
		{
			timer->stop();
			timer->close([](UvTimer* timer)
			{
				delete timer;
			});

			this->reConnect(idx);
		});
		timer1->start();

	});
}

void Broker::addNode(string &ip, unsigned short port)
{
	if (loopVector.size() < 1)
		start(CLINET_LOOPS);

	clients.emplace_back(ip, port);
	size_t index = clients.size() - 1;
	clients[index].index = index;

	size_t loopIndex = index % loopVector.size();
	EventLoopPtr loop = loopVector[loopIndex];

	 clients[index].conn = std::make_shared<TcpConnection>(loop);
	 TcpConnectionPtr &conn = clients[index].conn;
	// int n = conn.use_count();
	 conn->setConnectCb([this, index](int status, TcpConnectionPtr& connPtr)
	 {
		 this->onConnect(status, index);
	 });


	conn->setSendCb([this](int status, TaskPtr& task, TcpConnectionPtr& connPtr)
	{
		this->onSent(status, task, connPtr);

	});

	conn->setCloseCb([this](TcpConnectionPtr &conn) {
		// need reconnect?
		if (bStop)
		{
			// important
			conn->clearCb();
		}
		else
		{
			printf("some error cause close \n");
			this->reConnect(conn);
		}
	});

	loop->runInLoop([conn, ip, port]()
	{
		conn->connect(ip.c_str(), port);
	});
	
	int n = conn.use_count();
	printf("used count %d \n", n);
}
void Broker::onSent(int status, TaskPtr& ptr, TcpConnectionPtr& connPtr)
{
	this->sendNewTask(connPtr);
}
void Broker::onConnect(int status, size_t index)
{
	if (status < 0)
	{
		//printf("client %zu connect error%d\n", index, status);
		reConnect(clients[index]);
	}
	else
	{
		//printf("client %zu connect ok\n", index);
		sendNewTask(clients[index].conn);
	}
}

void Broker::sendNewTask(TcpConnectionPtr& conn)
{
	shared_ptr<PathlossTask> task = std::make_shared<PathlossTask>();
	task->setConnection(conn);
	{
		//std::lock_guard<mutex> guard(this->taskIdMutex);
		task->num = taskId++;
		
	}

	task->startTime = Timer::now();

	conn->sendMsg(dynamic_pointer_cast<ITask>(task));
}

