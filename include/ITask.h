#pragma once

#include <memory>
#include <string>

using namespace std;

namespace robin
{
	// declare
	class TcpConnection;

	// used as a struct
	class ITask
	{
	public:
		int getTaskType() { return taskType;  }

		void setConnection(TcpConnection * connection) { this->conn = connection;  }
		TcpConnection * getConnection() { return conn; }

		int errorCode = 0;
		string errorStr;

		string   taskTypeName = "testTask";
		string   taskIdStr;
		uint64_t taskId;
		int      taskType;

	private:
		TcpConnection *conn = nullptr;
	};

	using TaskPtr = std::shared_ptr<ITask>;

}