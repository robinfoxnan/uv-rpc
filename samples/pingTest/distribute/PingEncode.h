#pragma once
#include "../../../include/ITask.h"
#include "../../../include/IEncoder.h"
#include "../../../include/datapacket.h"
#include "PathlossTask.h"

namespace robin
{
	class PingEncode : public IEncoder
	{
		// tcp is stream oriented, must have a header. or something split the packet one to one
		//char tempbuf[100];
		
		virtual void encodeTask(CharVector & buffer, TaskPtr task) override
		{
			std::shared_ptr<PathlossTask> task1 = std::dynamic_pointer_cast<PathlossTask>(task);

			// same sa before
			buffer.clear();
			buffer.reserve(2048);
			
			// packet header 
			for (int i = 0; i < sizeof(DATA_HEADER); i++)
				buffer.push_back('#');

			// add data
			char * p = (char *)buffer.data() + sizeof(DATA_HEADER);
			snprintf(p, 100, "%ju", task1->num);

			// fill data of header
			DATA_HEADER * header = (DATA_HEADER*)buffer.data();
			header->type = 0x10;
			header->encode = 0xf0;
			header->version = 0x10;

			buffer.resize(256);
			header->len = htons(buffer.size() - sizeof(DATA_HEADER));
		}
	};
}
