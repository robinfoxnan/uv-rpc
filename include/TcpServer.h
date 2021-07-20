#pragma once
#include "EventLoop.h"
#include "CommonHeader.h"
#include "SocketAddr.h"
#include "BufferPool.h"
#include "IDispatcher.h"

namespace robin
{
	class TcpConnection;
	class TcpServer
	{
	public:
		TcpServer(int loops = 1);
		~TcpServer();

		void start();
		void stop();
		bool bindAndListen(const char *ip , unsigned short port, int backlog);
		bool bindAndListen(SocketAddr* address, int backlog);
		
		uv_loop_t * getLoop() { return loopListen->handle(); }
		

	public:
		static void onNewConnection(uv_stream_t *server, int status);
		static void onClose(uv_handle_t* handle);
			
	private:
		SocketAddrPtr localAddr;
		EventLoopPtr loopListen;             // used for listening
		
		size_t nLoops;
		uv_tcp_t server;

		std::mutex mapMutex;
		
	};

}
