#include "../include/DataPacket.h"
#include "../include/SimpleMsgDispatcher.h"

#include "../include/CommonHeader.h"
#pragma warning(disable:4267)
#pragma warning(disable:4244)

namespace robin
{
	class TcpConnection;

	// uv_process_tcp_read_req函数中每次通知读之前都是先回调申请内存，然后通知回调；每次申请64*1024B
	// 对于同一个连接来说，这里的通知顺序同步，也就是说数据是顺序到达的；不存在乱序，所以不用加锁和处理乱序
	// 当前定义的头长是8字节，
	// 1) 当前连接中，检查是否有剩余的数据,如果有，则需要处理
	// 2）当前连接的剩余数据处理完毕后，再处理新数据
	// 总体来说，就是把数据流，切割为一个个数据包，调用子类的解析函数
	void SimpleMsgDispatcher::onMessage(void * client, char *buf, ssize_t len)
	{
		TcpConnection * conn = (TcpConnection *)((uv_stream_t *)client)->data;

		uint16_t dataLen = 0;
		unsigned long left = len;
		int need = 0;
		
		CharVector & vecbuf = conn->getVecBuf();
		size_t sz = vecbuf.size();
		// 1) 剩余 >= 头长，同时 >= 完整包长：不存在，错误；
		// 2) 剩余 >= 头长，同时 < 完整包，新来可以补足一个完整包：拷贝补足,解析并清空，处理剩余的
		// 3) 剩余 >= 头长，同时 < 完整包，新来还不够一个完整包：直接拷贝，不处理了
		//
		// 4) 剩余 < 头长， 同时 新数据也不能补足一个头长：直接拷贝不处理
		// 5) 剩余 < 头长,  新数据可以补足一个头，但是不够一个包长：直接拷贝不处理
		// 6) 剩余 < 头长， 新数据可以补足一个头，也够一个包长：拷贝补足一个包长,解析并清空，处理剩下新来的
		if (sz > 0)
		{
			if (sz >= sizeof(DATA_HEADER))                   // 比头长要大，则在vecbuf中补足数据，然后处理vecbuf中的数据
			{
				char * lastBuf = (char *)vecbuf.data();
				DATA_HEADER * header = (DATA_HEADER *)lastBuf;
				dataLen = header->getLen();

				// 这里不应该够一个数据包，否则应该之前就处理完了；
				assert(sz < sizeof(DATA_HEADER) + dataLen);  

				if (sz >= sizeof(DATA_HEADER) + dataLen)     // 1) but i fixed this to work as ok
				{
					// 这里不应该执行到！！
					printf("error with SimpleMsgDispatcher::onMessage!!!!\n");
					LOG_ERROR("error with SimpleMsgDispatcher::onMessage!!!! case(1)");
					size_t tmpLen = vecbuf.capacity();
					CharVector tmpVec(tmpLen);
					vecbuf.copyTo(tmpVec);
					vecbuf.clear();
					onMessage(client, tmpVec.data(), tmpVec.size());
					onMessage(client, buf, len);
				}
				else
				{
					// 这里need是大于等于0
					need = sizeof(DATA_HEADER) + dataLen - sz;
					assert(need > 0);
					if (len >= need)                        // 2)
					{
						LOG_DEBUG("SimpleMsgDispatcher::onMessage case(2)");
						copyToVec(vecbuf, buf, need);

						// 处理数据包
						onMessageParse(header, (char *)vecbuf.data()+ sizeof(DATA_HEADER), dataLen, conn);
						// 清理之前的残留数据包
						vecbuf.clear();

						// 新来的数据扣除已经损耗的
						buf = buf + need;
						left = len - need;
						// 新来的数据剩下的部分
						doMessageInNewBuf(vecbuf, buf, left, conn);
					}
					else   // 3) 悲催，新来的还是不够一个包    
					{
						LOG_DEBUG("SimpleMsgDispatcher::onMessage case(3)");
						copyToVec(vecbuf, buf, len);
					}	   
				}	
			}
			else   // 上次剩余的字节连头长都不够，得借助一个临时的结构体
			{
				int need = sizeof(DATA_HEADER) - sz;
				assert(need > 0);
				if (len < need)   // 4）剩余加新来的都不够一个头长，概率不大
				{
					LOG_DEBUG("SimpleMsgDispatcher::onMessage case(4)");
					copyToVec(vecbuf, buf, len);
				}
				else  // 新来的终于可以补足一个头部了，呵呵
				{
					DATA_HEADER tmpHeader;
					
					memcpy((char *)&tmpHeader, (char *)vecbuf.data(), sz);
					memcpy((char *)&tmpHeader + sz, buf, need);
					char * tmpBuf = buf + need;
					left = len - need;
					dataLen = tmpHeader.getLen();
					if (dataLen > left)    // 5) 悲催的，新来加在一起还不够解析一次完整数据包啊；
					{
						LOG_DEBUG("SimpleMsgDispatcher::onMessage case(5)");
						copyToVec(vecbuf, buf, len);
					}
					else                   // 6) 解析一次，之后的交给另一个函数
					{
						LOG_DEBUG("SimpleMsgDispatcher::onMessage case(6)");
						onMessageParse(&tmpHeader, tmpBuf, dataLen, conn);
						// 处理数据包
						vecbuf.clear();    // 清理之前的残留数据包

						tmpBuf += dataLen;
						left -= dataLen;
						doMessageInNewBuf(vecbuf, tmpBuf, left, conn);
					}
				} // end of got more than a header
			}
			
		}
		else  // 这段直接处理新发来的数据包 
		{ 
			//LOG_DEBUG("SimpleMsgDispatcher::onMessage only in coming buffer.");
			doMessageInNewBuf(vecbuf, buf, len, conn);
		}
	}

	void SimpleMsgDispatcher::doMessageInNewBuf(CharVector & vecbuf, char *buf, unsigned long len, TcpConnection * conn)
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
	void SimpleMsgDispatcher::copyToVec(CharVector & vecbuf, char *buf, unsigned long len)
	{
		vecbuf.append(buf, len);
	}


}