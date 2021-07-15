#pragma once
#include "SimpleMsgDispatcher.h"

namespace robin
{

class JsonMsgDispatcher :public SimpleMsgDispatcher
{
public:
	// 由派生类来实现
	virtual void onMessageParse(DATA_HEADER * header, char *buf, unsigned long len, TcpConnection *conn) override;
};
}
