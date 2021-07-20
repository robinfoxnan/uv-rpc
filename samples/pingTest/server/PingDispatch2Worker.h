#pragma once
#include <iostream>
#include <functional>
#include "../../../include/IDispatcher.h"
#include "../../../include/SimpleMsgDispatcher.h"
#include "../../../include/WorkerPool.h"

namespace robin
{
	using RecvCallback = std::function<void(char *buf, unsigned long len)>;

	class PingDispatch2Worker : public SimpleMsgDispatcher
	{
	public:
		RecvCallback msgCb = nullptr;

		virtual void onMessageParse(DATA_HEADER * header, char *buf, unsigned long len, TcpConnection *conn) override
		{
			if (msgCb != nullptr)
				msgCb(buf, len);
			// send string back to client
			TaskPtr task = std::make_shared<ITask>();
			task->startDetal();
			task->taskIdStr = std::string(buf, len);
			
			task->setConnection(conn);
			WorkerPool::addToWorkQueue(task);


		}
	};
}