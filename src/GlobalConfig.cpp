/*
 * robin 2021-07-28
 * this class is used to compose all departments together,
 * since server and client will use different encoder and decoder(dispatcher)
**/
#include "../include/GlobalConfig.h"
#include "../include/CommonHeader.h"

namespace robin
{
	// init static vars
	std::shared_ptr<IDispatcher>    GlobalConfig::msgDispatcher;
	std::map<std::string, IWorkPtr> GlobalConfig::workMap;
	std::shared_ptr<IEncoder>       GlobalConfig::msgEncoder;

	uv_loop_t* GlobalConfig::loop = nullptr;


	// init something used by libuv
	void GlobalConfig::init(int n)
	{
		// set worker threads n = 64, should changed according cpu's num
		if (n < 4)
		{
			n = 4;
		}
		char buffer[260];
		snprintf(buffer, 100, "UV_THREADPOOL_SIZE=%d", n);
#ifdef WIN32
		_putenv(buffer);           // "UV_THREADPOOL_SIZE=6"
#else
		putenv((char *)buffer);    // "UV_THREADPOOL_SIZE=6"
#endif
		// test it
		/*
		char *buffer1 = new char[100];
		size_t st = 100;
		_dupenv_s(&buffer1, &st, "UV_THREADPOOL_SIZE");
		delete buffer1;
		*/

		//char buffer[260];
		buffer[0] = '\0';
		size_t sz = sizeof(buffer);
		uv_exepath(buffer, &sz);
		string path = buffer;
		string confPath;
#ifdef WIN32
		ssize_t off = path.rfind('\\');
		confPath = path.substr(0, off);
		confPath += "\\config.json";
#else
		int off = path.rfind('/');
		confPath = path.substr(0, off);
		confPath += "/config.json";
		
#endif // WIN32
		//DEBUG_PRINT(confPath.c_str());
		//DEBUG_PRINT("\n");
		// load config here as you like

		return ;
	}

	uv_loop_t* GlobalConfig::getDefaultLoop()
	{
		if (loop == NULL)
			loop = uv_default_loop();
		return loop;
	}

	void GlobalConfig::setMsgDispatcher(std::shared_ptr<IDispatcher> dispatcher)
	{
		GlobalConfig::msgDispatcher = dispatcher;
	}

	std::shared_ptr<IDispatcher> GlobalConfig::getMsgDispatcher()
	{
		return msgDispatcher;
	}

	void GlobalConfig::setEncoder(std::shared_ptr <IEncoder> encoder)
	{
		GlobalConfig::msgEncoder = encoder;
	}

	std::shared_ptr <IEncoder> GlobalConfig::getEncoder()
	{
		return GlobalConfig::msgEncoder;
	}

	bool GlobalConfig::addWorkType(const string & name, IWorkPtr ptr)
	{
		auto it = workMap.find(name);
		if (it != workMap.end())
			return false;

		workMap.insert(std::pair<string, IWorkPtr>(name, ptr));

		return true;
	}

	IWorkPtr GlobalConfig::getWorkType(const string & name)
	{
		auto it = workMap.find(name);
		if (it != workMap.end())
			return it->second;

		return NULL;
	}
}
