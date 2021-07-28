#pragma once

#include "../include/IDispatcher.h"
#include "../include/DataPacket.h"
#include "../include/TcpConnection.h"

namespace robin
{

	class SimpleMsgDispatcher :public IDispatcher
	{
	public:
		virtual void onMessage(TcpConnectionPtr & conn, char *buf, ssize_t len) override;


		// parse the data part, user should do it himself
		virtual void onMessageParse(DATA_HEADER * header, char *buf, unsigned long len, TcpConnectionPtr& conn)
		{
			printf("SimpleMsgDispatcher::onMessageParse does nothing parsing work , user a child class instead!\n");
		}



	private:
		// cycling parse packet in new received
		void doMessageInNewBuf(CharVector & vecbuf, char *buf, unsigned long len, TcpConnectionPtr& conn);
		void copyToVec(CharVector & vecbuf, char *buf, unsigned long len);
	};

}

