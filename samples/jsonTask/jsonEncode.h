#pragma once
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "../include/datapacket.h"
//#include "timeUtils.h"
#include "VecStreamWrapper.h"
#include "ITask.h"
#include "IEncoder.h"

#ifdef WIN32
#include<WinSock2.h>
#else
#include <arpa/inet.h>
#endif


using namespace rapidjson;

namespace robin
{
	class JsonEncode : public IEncoder
	{
	public:
		virtual JsonEncode * instance()  override
		{
			static JsonEncode jsonEncoder;
			return &jsonEncoder;
		}
		// 将结果直接编码到向量，然后发送
		virtual void encodeTask(std::vector<char>& buffer, TaskPtr task) override
		{
			// 先占位，防止后期移动
			for (int i =0; i<sizeof(DATA_HEADER); i++)
				buffer.push_back('#');

			VecStreamWrapper wraper(buffer);
			Writer <VecStreamWrapper> writer(wraper);


			
			// 最后添加一个0，方便对方处理
			buffer.push_back('\0');
			// 重新处理头部
			DATA_HEADER * header = (DATA_HEADER*)buffer.data();
			header->type = 0x10;
			header->encode = 0x00;
			header->version = 0x10;
			header->len = htons(buffer.size() - sizeof(DATA_HEADER));

		}
		/*
		static void testEncodeJob(std::string& retStr)
		{
	
			Job job = Job();
			job.jobId = "20210623465464";
			LinkPtr link = std::make_shared<Link>();
			link->jobId = job.jobId;
			link->linkIndex = 0;

			job.links.push_back(link);

			bool ret = encodeJob(retStr, job);
			cout << retStr;
		}
		*/

		/*
		static bool encodeJob(std::string& retStr, const Job& job)
		{
			StringBuffer s;
			Writer<StringBuffer> writer(s);

			writer.StartObject();               // Between StartObject()/EndObject(), 
			writer.Key("jobId");                // output a key,
			writer.String(job.jobId.c_str());   // follow by a value.

			writer.Key("nums");
			writer.Uint64(job.links.size());

			writer.Key("time");                // output a key,
			job.timeStr = getTimeNow();
			writer.String(job.timeStr.c_str());

			writer.Key("links");
			writer.StartArray();                // Between StartArray()/EndArray(),
			int index = 0;
			for (const auto link : job.links)
			{
				writer.StartObject();
				writer.Key("linkIndex");
				writer.Int(index);

				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();  // end of root

			// {"hello":"world","t":true,"f":false,"n":null,"i":123,"pi":3.1416,"a":[0,1,2,3]}
			retStr =  s.GetString();

			return true;
		}*/
	};
}