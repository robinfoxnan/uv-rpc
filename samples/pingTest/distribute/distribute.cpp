/*
 * robin 2021-07-28
**/
#include "broker.h"


#include "PingEncode.h"
#include "PongDispatcher.h"
#include "../../../include/UvTimer.h"

#include <iostream>

using namespace robin;

robin::Broker *broker = nullptr;

void testPing()
{

}

volatile bool bExit = false;
void countSend()
{
	while (true)
	{
		if (bExit)
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		broker->printInfo();
	}
	
}

std::thread clientThread;
std::thread sendThread;

bool GetEventMessage(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		bExit = true;
		return true;
	}

	return false;
}


BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	return GetEventMessage(dwCtrlType);
}


int main()
{
	LOG_INFO("START HERE");
	broker = new Broker();
	broker->init();
	

	string ip = "127.0.0.1";
	broker->addNode(ip, 80);
	/*broker->addNode(ip, 80);
	broker->addNode(ip, 80);
	broker->addNode(ip, 80);
	broker->addNode(ip, 80);
	broker->addNode(ip, 80);*/
	
	BOOL fSuccess = SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	sendThread = thread(countSend);
	if (sendThread.joinable())
	{
		sendThread.join();
	}
	broker->stop();

	delete broker;
    
	cout << "client exit here\n" << endl;
}

