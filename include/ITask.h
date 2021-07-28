#pragma once

#include <memory>
#include <string>
#include "CommonHeader.h"
#include "../utils/Timer.h"
#include <chrono>
#include <ratio>

using namespace std;

namespace robin
{
	// declare
	class TcpConnection;
	using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
	// used as a struct
	class ITask
	{
	public:
		ITask() {  }
		virtual ~ITask() {}

		inline int getTaskType() { return taskType;  }

		inline void setConnection(TcpConnectionPtr & connection) { this->conn = connection;  }
		inline TcpConnectionPtr getConnection() 
		{ 
			return conn.lock();
		}

		int errorCode = 0;
		string errorStr;

		string   taskTypeName = "IWork";
		string   taskIdStr;
		uint64_t taskId;
		int      taskType;


		// COUNT TIMES
		inline void startDetal() {
			startTime = high_resolution_clock::now();
		
		}

		inline double markMid1() {
			mid1 = duration<double, Timer::ms>(high_resolution_clock::now() - startTime).count();
			return mid1;
		}
		inline double delta() { msecs = duration<double, Timer::ms>(high_resolution_clock::now() - startTime).count(); return msecs; }

		
		
		double msecs;
		double mid1;
		time_point<high_resolution_clock>  startTime;

	private: // must be weak pointer, or will leak memory
		std::weak_ptr<TcpConnection> conn;
	};

	using TaskPtr = std::shared_ptr<ITask>;

}