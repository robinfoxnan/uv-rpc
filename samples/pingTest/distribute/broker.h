#pragma once
#include "../../../include/CommonHeader.h"
#include "../../../include/EventLoop.h"
#include "../../../include/TcpConnection.h"

namespace robin
{
	// work node(rpc server) list item, used as a struct, put workLoad and pointer into heap
	// there are two kinds of ways to use it:
	// 1) n client connect to one same server, test through output
	// 2) n client connect to different server node, as a cluster manager
	class NodeItem               
	{
	public:
		NodeItem(string ip_, short port_) : ip(ip_), port(port_), bOk(false), index(0), workLoad(0), conn(nullptr)
		{
		};
		~NodeItem()
		{
			printf("delete node item index= %ju, use count=%d \n", index, conn.use_count());
			conn.reset();
		}
		string ip;
		unsigned short port;
		bool  bOk;
		size_t index;
		uint32_t workLoad;
		TcpConnectionPtr conn;
	};

// all clients run in these loops 
#define CLINET_LOOPS 6

	class Broker
	{
	public:
		Broker();
		~Broker();

		void init();
		void start(int n = CLINET_LOOPS);
		void stop();

		void addNode(string &ip, unsigned short port);
		void sendNewTask(TcpConnectionPtr& conn);
		
		void reConnect(TcpConnectionPtr & conn);
		void reConnect(NodeItem & item);
		void reConnect(size_t idx);
		void printInfo();
		EventLoop * getLoop()
		{
			if (loopVector.size() > 0)
				return loopVector[0].get();
			return nullptr;
		}

		

	private:
		
		void onSent(int status, TaskPtr& ptr, TcpConnectionPtr& connPtr);
		void onConnect(int status, size_t index);
		/*atomic<uint64_t> sentCmd;
		atomic<uint64_t> recvCmd;*/
		uint64_t taskId;
		std::mutex    taskIdMutex;

		atomic<bool> bStop;

		vector<NodeItem> clients;  // used to connect to nodes
		vector<EventLoopPtr> loopVector;   // loops and threads
		vector<thread> threadVector;       // 
	};
}