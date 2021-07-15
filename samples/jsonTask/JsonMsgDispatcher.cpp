#include "JsonMsgDispatcher.h"
#include <string>
#include <iostream>
#include "WorkerPool.h"
#include "jsonDecode.h"

namespace robin
{
	// 这里需要引入conn，传入task，否则计算结束后就不知道数据该回传给谁了；
	void JsonMsgDispatcher::onMessageParse(DATA_HEADER * header, char *buf, unsigned long len, TcpConnection *conn)
	{
		std::string str(buf, len);
		std::cout << str << std::endl;

		// 解析数据
		std::shared_ptr<ITask> task = JsonDecode::instance()->decodeJson(header, buf, len);
		if (task->errorCode != 0)  // 解析失败
		{

		}
		else
		{
			task->setConnection(conn);

			WorkerPool::addToWorkQueue(task);
		}
		
	}
}