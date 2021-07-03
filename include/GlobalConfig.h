#pragma once
#include "IDispatcher.h"
#include "IWork.h"
#include "ITask.h"
#include "IEncoder.h"
#include <memory>
#include <map>
#include <uv.h>

namespace robin
{
	// 之所以有这个类，是为了将各种全局的实例管理起来，因为uv中
	
	using namespace std;

	class GlobalConfig
	{
	public:
		static void init();
		static void setMsgDispatcher(std::shared_ptr<IDispatcher> dispatcher);
		static std::shared_ptr<IDispatcher> getMsgDispatcher();

		static void setEncoder(std::shared_ptr <IEncoder> encoder);
		static std::shared_ptr<IEncoder> getEncoder();

		static uv_loop_t* getDefaultLoop();

		static bool addWorkType(const string & name, IWorkPtr ptr);
		static IWorkPtr getWorkType(const string & name);

	private:
		static std::shared_ptr<IDispatcher> msgDispatcher;
		static uv_loop_t* loop;
		
		static std::map<std::string, IWorkPtr> workMap; // 不同的类型的工作添加到映射中，计算时候执行不同的计算
		static std::shared_ptr <IEncoder> msgEncoder;
	};

}

