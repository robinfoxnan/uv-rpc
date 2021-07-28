#pragma once
#include "CommonHeader.h"

namespace robin
{
	// Interface for read callback
	class TcpConnection;
	using TcpConnectionPtr = shared_ptr<TcpConnection>;

	class IDispatcher
	{
	public:
		virtual void onMessage(TcpConnectionPtr & conn, char *buf, ssize_t len) {};
	};
}