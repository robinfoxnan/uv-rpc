#include "../include/CommonHeader.h"
#include "../include/TcpServer.h"
#include "../include/GlobalConfig.h"
#include "../include/TcpConnection.h"
#include "../include/SocketAddr.h"
#include "../include/ConnectionManager.h"

namespace robin
{
#define DEFAULT_BACKLOG 128

	TcpServer::TcpServer(int loops) :nLoops(loops)
	{
		loopListen = std::make_shared<EventLoop>();
		
	}

	TcpServer::~TcpServer()
	{
	}

	void TcpServer::start()
	{
		ConnectionManager::instance()->start();
		bool ret = uv_loop_alive(loopListen->handle());
		assert(ret == true);
		printf("tcp server started:\n");
		loopListen->run();
	}

	


	void TcpServer::stop()
	{
		loopListen->stop();
		
	}
	bool TcpServer::bindAndListen(const char *ip, unsigned short port, int backlog)
	{
		localAddr = make_shared<SocketAddr>(ip, port);
		return this->bindAndListen(localAddr.get(), backlog);
	}

	bool TcpServer::bindAndListen(SocketAddr* address, int backlog)
	{
		int ret = uv_tcp_init(loopListen->handle(), &server);

		ret= uv_tcp_bind(&server, (const struct sockaddr*)address->Addr(), 0);

		if (backlog < DEFAULT_BACKLOG)
			backlog = DEFAULT_BACKLOG;

		// set this to (void * data)
		server.data = static_cast<void *>(this);
		ret = uv_listen((uv_stream_t*)&server, backlog, TcpServer::onNewConnection);
		if (ret) 
		{
			//fprintf(stderr, "Listen error %s\n", uv_strerror(r));
			return false;
		}
		printf("server is listening at %s\n", address->toStr().c_str());

		return true;
	}

	// uv_on_connection_callback
	void TcpServer::onNewConnection(uv_stream_t *server, int status)
	{
		TcpServer * tcpServer =  static_cast<TcpServer *>(server->data);

		if (status < 0) 
		{
			//LOG_ERROR();
			fprintf(stderr, "New connection error %s\n", uv_strerror(status));
			// error!
			return;
		}

		ConnectionManager::instance()->onNewConnection(server, status);
	}

	

	void TcpServer::onClose(uv_handle_t* handle)
	{
		TcpConnection * conn = static_cast<TcpConnection *>(handle->data);
		
		//std::string ip;
		//conn->getKey(ip);
		//std::cout << "close connection of " << ip << std::endl;
		delete conn;
	}
}