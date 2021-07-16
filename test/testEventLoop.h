#pragma once
#include "../include/CommonHeader.h"
#include "../include/EventLoop.h"
#include "../include/TcpConnection.h"
#include "../include/GlobalConfig.h"
#include "../include/ConnectionManager.h"

#include "PongEncode.h"
#include "PingDispatcher.h"

#include <iostream>

using namespace robin;
#include <iostream>

ConnectionManager manager;
void testConnectionManager()
{
	manager.start();
	manager.runInLoop([]()
	{
		printf("test call back1\n");
	});

	manager.runInLoop([&]()
	{
		printf("test call back2\n");
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	//char c;
	//std::cin >> c;
	manager.stop();

	std::cout << "Hello World!\n";
}
/////////////////////////////////////////////////
EventLoop loop;
void test1()
{
	loop.run();
}

void test_stop1()
{
	thread t1 = thread(test1);

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	printf("waiting...\n");
	loop.stop();
	t1.join();
}

///////////////////////////////////////////////////////
vector<EventLoopPtr> loopVector;
vector<thread> threadVector;
void testEvenetBlock()
{
	for (int i = 0; i < IO_LOOPS; i++)
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

		for (auto & t : threadVector)
		{
			if (t.joinable())
				t.join();
		}
}