#pragma once
#include "CommonHeader.h"
#include "TcpConnection.h"
#include "EventLoop.h"

#define IO_LOOPS 6

namespace robin
{
	class ConnectionManager
	{
	public:
		ConnectionManager();
		~ConnectionManager();
		void start(int n = IO_LOOPS);
		void stop();
		void initWorkerPool(uv_loop_t * loop);

		static ConnectionManager * instance() {
				static ConnectionManager manager;
				return &manager;
			}

		EventLoopPtr & getNextLoop();

		void   onNewConnection(uv_stream_t *server, int status);
		size_t addConnection(std::string &key, TcpConnectionPtr& ptr);
		bool   freeConnection(std::string & key);
		bool   freeConnection(TcpConnectionPtr& conn);

		void runInLoop(DefaultCallback cb);

		void printPool();
		void printSpeed();

	private:
		std::map<std::string, TcpConnectionPtr> connMap;
		std::mutex mapMutex;

		size_t round_robin = 0;
		vector<EventLoopPtr> loopVector;
		vector<thread> threadVector;
	};
	
}