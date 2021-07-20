#pragma once
#include <iostream>
#include <functional>

#include "../../../include/IDispatcher.h"
#include "../../../include/SimpleMsgDispatcher.h"

namespace robin
{
	using RecvCallback = std::function<void(char *buf, unsigned long len)>;

	class PongDispatcher : public SimpleMsgDispatcher
	{
	public:
		RecvCallback msgCb = nullptr;

		virtual void onMessageParse(DATA_HEADER * header, char *buf, unsigned long len, TcpConnection *conn) override
		{
			if (msgCb)
			{
				msgCb(buf, len);
			}

		}
	};
}
