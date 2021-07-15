#pragma once
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "../include/datapacket.h"
#include "../include/timeUtils.h"
#include "ITask.h"
#include "TaskLink.h"

using namespace std;
using namespace rapidjson;

namespace robin
{
	class JsonDecode
	{
	public:
		static JsonDecode * instance()
		{
			static JsonDecode jsonDecode;
			return &jsonDecode;
		}

		TaskPtr decodeJson(DATA_HEADER * header, char * json, size_t len)
		{
			std::shared_ptr<TaskLink> task = std::make_shared<TaskLink>();

			Document doc;
			doc.Parse(json, len);

			if (doc.HasParseError())
			{
				ParseErrorCode errCode = doc.GetParseError();
				size_t off = doc.GetErrorOffset();
				//fprintf(stderr, "JSON parse error: %s (%u)", GetParseError_En(errCode), off);
				string temp = string(json, off, strlen(json) - off);
				cout << temp << endl;
				task->errorCode = errCode;
				task->errorStr = temp;
			}

			bool ret = doc.HasMember("jobId");
			Value& h = doc["jobId"];
			ret = h.IsString();
			task->jobId = doc["jobId"].GetString();

			return task;
		}

		//static JobPtr decodeJob(const char * json)
		//{
		//	Document doc;
		//	doc.Parse(json);

		//	if (doc.HasParseError())
		//	{
		//		ParseErrorCode errCode = doc.GetParseError();
		//		size_t off = doc.GetErrorOffset();
		//		//fprintf(stderr, "JSON parse error: %s (%u)", GetParseError_En(errCode), off);
		//		string temp = string(json, off, strlen(json) - off);
		//		return nullptr;
		//	}

		//	JobPtr  job = std::make_shared<Job>();

		//	bool ret = doc.HasMember("jobId");
		//	Value& h = doc["jobId"];
		//	ret = h.IsString();
		//	job->jobId = doc["jobId"].GetString();

		//	ret = doc.HasMember("links");
		//	Value& valLinks = doc["links"].GetArray();
		//	for (int i=0; i<valLinks.Size(); i++)
		//	{
		//		const Value& link = valLinks[i];
		//		LinkPtr ptr = std::make_shared<Link>();
		//		ptr->jobId = job->jobId;
		//		ptr->linkIndex = link["linkIndex"].GetInt();

		//		job->links.push_back(ptr);
		//	}



		//	return job;
		//}
	};
}
