// testSimpleDispatcher.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "../include/CommonHeader.h"

#include "../include/EventLoop.h"
#include "../include/TcpConnection.h"
#include "../include/DataPacket.h"
#include "../include/SimpleMsgDispatcher.h"

#if defined(WIN32) || defined(_WIN64)
#include "../../utils/cmdsetting_w32.h"
#else
#endif

#include <iostream>
using namespace robin;

char msg[64 * 1024];
EventLoopPtr loop = std::make_shared<EventLoop>();
TcpConnection conn(loop);

class TestParser : public SimpleMsgDispatcher
{
public:
	virtual void onMessageParse(DATA_HEADER * header, char *buf, unsigned long len, TcpConnection * conn) override
	{
		int index = header->encode;
		bool ret = true;
		for (int i =0; i<len; i++)
		{
			if (buf[i] != '#')
			{
				ret = false;
				break;
			}
		}
		if (header->reserve[0]!= '*' || header->reserve[1] != '*'  || header->reserve[2] != '*')
			ret = false;
		if (ret)
		{
			printf("packet %d len %u is ok.\n", index, len);
		}
		else
		{
			printf("packet %d len %u is error-----------------.\n", index, len);
		}
		
	}
};
TestParser parser;

void fillPacket(char * buf, int packetLen, int index)
{
	for (int i=0; i< packetLen; i++)
	{
		buf[i] = '#';
	}

	DATA_HEADER *header = (DATA_HEADER *)buf;
	header->setLen(packetLen - sizeof(DATA_HEADER));
	header->encode = (uint8_t)index;
	header->reserve[0] = '*';
	header->reserve[1] = '*';
	header->reserve[2] = '*';

}

// split n packets to vecBuf & msg
void InitBuffer(char * buffer, int len, int left)
{
	CharVector & vecbuf = conn.getVecBuf();
	vecbuf.clear();
	for (int i=0; i<left; i++)
	{
		vecbuf.push_back(buffer[i]);
	}

	memcpy(msg, buffer + left, len - left);
}

// nPackets should less than 256
// split packets and call parse 1 times
// 1) & 2) & 6) will see in rolling log file
void FixedPackets1(int datalen,  int nPackets)
{
	conn.getUvClient()->data = &conn;
	size_t packetLen = datalen + sizeof(DATA_HEADER);
	size_t len = nPackets * packetLen;
	char * buffer = new char[len];
	if (buffer == nullptr)
		return;

	// fill the packets
	char *buf = buffer;
	for (int i=0; i<nPackets; i++)
	{
		fillPacket(buf, packetLen, i+1);
		buf = buf + packetLen;
	}

	// fill to the vecBuf
	printf("now test for %d packets, %d bytes per packet\n", nPackets, datalen);
	for (int j=0; j< packetLen; j++)   // test packetlen times
	{
		printf("--------%d bytes in vecbuf, %zd bytes in msg-----------\n", j, len-j);
		InitBuffer(buffer, len, j);
		parser.onMessage(conn.getUvClient(), msg, len - j);
	}

	delete[] buffer;
}
////////////////////////////////////////////////////////////////////////////////
// nPackets should less than 256
// split packets and call parse 2 times
void FixedPackets2(int datalen, int nPackets)
{
	conn.getUvClient()->data = &conn;
	size_t packetLen = datalen + sizeof(DATA_HEADER);
	size_t len = nPackets * packetLen;
	char * buffer = new char[len];
	if (buffer == nullptr)
		return;

	// fill the packets
	char *buf = buffer;
	for (int i = 0; i < nPackets; i++)
	{
		fillPacket(buf, packetLen, i + 1);
		buf = buf + packetLen;
	}

	// fill to the vecBuf
	printf("now test for %d packets, %d bytes per packet\n", nPackets, datalen);
	for (int left = 0; left < packetLen; left++)   // test packetlen times
	{
		for (int msgLen1 =1; msgLen1 < len- left; msgLen1 ++)
		{
			int msgLen2 = len - left - msgLen1;
			printf("--------%d B in vecbuf, %d B in msg1, %d B in msg2-----------\n", left, msgLen1, msgLen2);

			printf("call 1:\n");
			InitBuffer(buffer, left + msgLen1, left);
			parser.onMessage(conn.getUvClient(), msg, msgLen1);
			printf("call 2:\n");
			memcpy(msg, buffer + left + msgLen1, msgLen2);
			parser.onMessage(conn.getUvClient(), msg, msgLen2);

		}
		
		
	}

	delete[] buffer;
}


int main()
{
	//FixedPackets1(20, 5);
	FixedPackets2(10, 3);

	int n = get_thread_amount();
	printf("threads %d\n", n);
    std::cout << "finished!\n";
}
