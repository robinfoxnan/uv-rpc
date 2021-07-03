#include "../include/DataPacket.h"
#include "../include/SimpleMsgDispatcher.h"

#include "../include/CommonHeader.h"


namespace robin
{
	class TcpConnection;

	// uv_process_tcp_read_req函数中每次通知读之前都是先回调申请内存，然后通知回调；每次申请64*1024B
	// 对于同一个连接来说，这里的通知顺序同步，也就是说数据是顺序到达的；不存在乱序，所以不用加锁和处理乱序
	// 当前定义的头长是8字节，
	// 1) 当前连接中，检查是否有剩余的数据,如果有，则需要处理
	// 2）当前连接的剩余数据处理完毕后，再处理新数据
	// 总体来说，就是把数据流，切割为一个个数据包，调用子类的解析函数
	void SimpleMsgDispatcher::onMessage(void * client, char *buf, unsigned long len)
	{
		TcpConnection * conn = (TcpConnection *)((uv_stream_t *)client)->data;

		uint16_t dataLen = 0;
		unsigned long left = len;
		int need = 0;
		
		std::vector<uint8_t> & vecbuf = conn->getVecBuf();
		size_t sz = vecbuf.size();
		if (sz > 0)
		{
			if (sz >= sizeof(DATA_HEADER))                   // 比头长要大，先处理vecbuf中的数据
			{
				char * lastBuf = (char *)vecbuf.data();
				DATA_HEADER * header = (DATA_HEADER *)lastBuf;
				dataLen = ntohs(header->len);
				assert(sz < sizeof(DATA_HEADER) + dataLen);  // 这里不应该够一个数据包，否则应该之前就处理完了；
				if (sz < sizeof(DATA_HEADER) + dataLen)
				{
					// 这里不应该执行到！！
				}
				else
				{
					// 这里need是大于等于0
					need = sizeof(DATA_HEADER) + dataLen - sz;
					copyToVec(vecbuf, buf, need);

					// 处理数据包
					onMessageParse(header, (char *)vecbuf.data(), vecbuf.size(), conn);    
					// 清理之前的残留数据包
					vecbuf.clear();    

					buf = buf + need;
					left = len - need;
					// 新来的数据剩下的部分
					doMessageInNewBuf(vecbuf, buf, left, conn);   
				}

				
			}
			else   // 上次剩余的字节连头长都不够，得借助一个临时的结构体
			{
				DATA_HEADER tmpHeader;
				memcpy((char *)&tmpHeader, (char *)vecbuf.data(), sz);
				memcpy((char *)&tmpHeader + sz, buf, sizeof(DATA_HEADER) - sz);
				char * tmpBuf =  buf + sizeof(DATA_HEADER) - sz;
				left = len - (sizeof(DATA_HEADER) - sz);
				dataLen = ntohs(tmpHeader.len);
				if (dataLen > left)    // 悲催的，新来加在一起还不够解析一次；
				{
					copyToVec(vecbuf, buf, len);
				}
				else                   // 解析一次，之后的交给另一个函数
				{
					onMessageParse(&tmpHeader, tmpBuf, dataLen, conn); 
					                   // 处理数据包
					vecbuf.clear();    // 清理之前的残留数据包

					tmpBuf += dataLen;
					left -= dataLen;
					doMessageInNewBuf(vecbuf, tmpBuf, left, conn);
				}

			}
			
		}
		else  // 这段直接处理新发来的数据包 
		{ 
			doMessageInNewBuf(vecbuf, buf, len, conn);
		}
	}

	void SimpleMsgDispatcher::doMessageInNewBuf(std::vector<uint8_t> & vecbuf, char *buf, unsigned long len, TcpConnection * conn)
	{
		if (len == 0)
			return;

		unsigned long left = len;
		uint16_t dataLen = 0;
		while (left >= sizeof(DATA_HEADER))                     // 这里多轮循环，主要是一个缓冲区可以有多个包；
		{
			DATA_HEADER * header = (DATA_HEADER *)buf;
			dataLen = ntohs(header->len);

			if (left == (dataLen + sizeof(DATA_HEADER)))        // 刚刚好；
			{

				buf = buf + sizeof(DATA_HEADER);
				onMessageParse(header, buf, dataLen, conn);     // 处理数据包
				left = 0;
				break;
			}
			else if (left > (dataLen + sizeof(DATA_HEADER)))    // 收到的数据太长了，处理完之后，还要拷贝出去一点点
			{
				buf = buf + sizeof(DATA_HEADER);
				onMessageParse(header, buf, dataLen, conn);     // 处理数据包

				left = left - (dataLen + sizeof(DATA_HEADER));
				buf = buf + dataLen;
				continue;
			}
			else
			{
				break;
			}
		}

		// 把剩余的拷贝到连接中
		if (left > 0)
		{
			copyToVec(vecbuf, buf, left);
		}
	}
	// copy the left data to TcpConnection vecBuf
	void SimpleMsgDispatcher::copyToVec(std::vector<uint8_t> & vecbuf, char *buf, unsigned long len)
	{
		// use this or that
		/*for (unsigned long i = 0; i < len; i++)
		{
			vecbuf.push_back(buf[i]);
		}*/

		vecbuf.reserve(vecbuf.size() + len);
		memcpy(vecbuf.data()+ vecbuf.size(), buf, len);
	}


}