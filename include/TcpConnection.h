#pragma once

#include "../utils/Timer.h"

#include "../include/CommonHeader.h"
#include "../include/BufferPool.h"
#include "../include/EventLoop.h"
#include "../include/SocketAddr.h"

namespace robin
{
	class EventLoop;
	class TcpServer;
	class SocketAddr;
	class ITask;
	using TaskPtr = std::shared_ptr<ITask>;


	using ConnectCallback = std::function<void(int)>;
	using SendCallback = std::function<void(int, TaskPtr)>;
	

	class TcpConnection
	{
	public:

		using CloseCallback = std::function<void(TcpConnection *)>;
		// used by alloc callback
		static void onAllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
		static void onFreeReadBuffer(uv_buf_t *buf);

	public:
		TcpConnection(const EventLoopPtr& loop, bool tcpNoDelay = false);
		~TcpConnection();

		void   start();
		void   stop();

		// key likes "ip:port"
		string getKey();
		uv_loop_t * getLoop();

		uv_tcp_t * getUvClient() { return &remote; }
		
		//std::vector<uint8_t> & getVecBuf() { return buf; }
		CharVector & getVecBuf() { return buf; }

		// here send is usually beside of the loop thread,so post event to send
		void sendMsg(TaskPtr task);
		static void afterWrite(uv_write_t *req, int status);
		void pushbackQue(write_req_vec_t * req);
		void invokeSend();

		bool connect(SocketAddr* address);
		bool connect(const char *ip, unsigned short port);
		void close(int reason = 0);
		static void onConnect(uv_connect_t* req, int status);
		static void onRead(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
		static void onClose(uv_handle_t* handle);

		inline bool isConnected() {
			bool ret = bConnected; return ret;
		 }
		inline bool isClient() { bool ret = bClient; return ret; }

		inline void setConnectCb(ConnectCallback cb) { this->connCb = cb; }
		inline void setSendCb(SendCallback cb) { this->sendCb = cb; }
		inline void setCloseCb(CloseCallback cb) { this->closeCb = cb; }
		inline ConnectCallback getConnectCb() { return this->connCb; }
		inline SendCallback    getSendCb() { return this->sendCb; }
		inline CloseCallback   getCloseCb() { return this->closeCb;  }

		//void setCloseReason(int status) { this->closeReason = status; }
		int  getCloseReason() { return closeReason;  }

		Timer timer;

	private:
		TcpConnection() {}
		EventLoopPtr loopPtr;

		// store remote information when working as a server part
		uv_tcp_t  remote;         

		// work as a client
		uv_tcp_t  local;
		uv_connect_t connect_req;  
		atomic<bool> bClient;
		bool bNoDelay;
		atomic<bool> bConnected;
		string infoKey;
		int    closeReason;

		shared_ptr<SocketAddr> remoteAddr;

		
		// tcp is stream-oriented£¬so should have a vector to contain part of packet
		// we can't get just a packet each time, sometimes many packets, sometimes part of packet
		//std::vector<uint8_t> buf;
		CharVector   buf;
		ConnectCallback connCb = nullptr;
		SendCallback    sendCb = nullptr;
		CloseCallback   closeCb = nullptr;

	private:
		// these 4 are used in sending progress too.
		write_req_vec_t * encodeTask(TaskPtr& task);
		write_req_vec_t * popQue();
		void pushinQue(TaskPtr& task);
		void realSend();

		// send: put the task in the que, send to pool thread a lamda event
		//std::deque<TaskPtr> taskQue;  // see, taskptr in req
		std::deque< write_req_vec_t *> sendQue;
		std::mutex taskMutex;
		
	};
	using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
}
