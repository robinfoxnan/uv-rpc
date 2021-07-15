#pragma once
#include "../../../include/ITask.h"
#include "../../../include/IEncoder.h"
#include "../../../include/datapacket.h"
#include "../../../include/CommonHeader.h"

namespace robin
{
	class PongEncode : public IEncoder
	{
	
		virtual void encodeTask(CharVector& buffer, TaskPtr task) override
		{
			// same as before
			buffer.clear();
			buffer.reserve(1024);

			// packet header 
			for (int i = 0; i < sizeof(DATA_HEADER); i++)
				buffer.push_back('#');

			buffer.capacity();
			buffer.size();
			// add data string
			for (size_t i = 0; i < task->taskIdStr.size(); i++)
			{
				buffer.push_back(task->taskIdStr[i]);
			}
			// add '\0'
			buffer.push_back('\0');
	
			// fill data of header
			DATA_HEADER * header = (DATA_HEADER*)buffer.data();
			header->type = 0x10;
			header->encode = 0xf0;
			header->version = 0x10;
			header->len = htons(buffer.size() - sizeof(DATA_HEADER));
			
			
		}
	};
}
