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
	// manage all instances and config together
	
	using namespace std;

	class GlobalConfig
	{
	public:
		static void init(int n = 4);
		static uv_loop_t* getDefaultLoop();

		// default
		static void setMsgDispatcher(std::shared_ptr<IDispatcher>  dispatcher);
		static std::shared_ptr<IDispatcher> getMsgDispatcher();

		// add multi
		static void addDispatcher(string name, std::shared_ptr<IDispatcher> dispatcher);
		static std::shared_ptr<IDispatcher> getMsgDispatcher(string &name);

		template <typename DISPATCH>
		static std::shared_ptr<DISPATCH> getMsgDispatcher(string& name);

		// default encoder
		static void setEncoder(std::shared_ptr<IEncoder>  encoder);
		static std::shared_ptr<IEncoder> getEncoder();

		static void setEncoder(string &name, IEncoderPtr  encoder);
		static IEncoderPtr getEncoder(string &name);

		template <typename ENCODER>
		static shared_ptr<ENCODER> getEncoder(string &name);

		

		static bool addWorkType(const string & name, IWorkPtr ptr);
		static IWorkPtr getWorkType(const string & name);

		template <typename WORK>
		static shared_ptr<WORK> getWorkType(const string & name);

	private:
		// default 
		static std::shared_ptr<IDispatcher> msgDispatcher;
		static std::shared_ptr <IEncoder> msgEncoder;
		static uv_loop_t* loop;
		static string exeDir;
		
		static std::map<std::string, IWorkPtr> workMap; // different work types

		// add different dispatchers
		static std::map<std::string, IDispatcherPtr> dispatcherMap;
		static std::map<std::string, IEncoderPtr> encoderMap;
		
	};

}

