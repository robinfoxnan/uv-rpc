#pragma once

#include <stdint.h>
#if (defined WIN32) || (defined _WIN64)
#include<WinSock2.h>
#else
#include <arpa/inet.h>
#endif

namespace robin
{
	
	// header is 8bytes fixed
	typedef struct header_packet
	{
		uint16_t len;             // datalen£¬len should be in network order
		uint8_t  type;            // packetType =  tasktype
		uint8_t  encode;          // encode: 0:key=value;, 1:json£¬ 2:protobuf
		uint8_t  version;         // 0x10  = 1.0
		uint8_t  reserve[3];      // "###"

		void setLen(size_t sz)
		{
			len = htons(sz);
		}
		uint16_t getLen()
		{
			return ntohs(len);
		}
	}DATA_HEADER;


} // end of robin namespace