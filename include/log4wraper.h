#ifndef _MYLOG_H
#define _MYLOG_H
#include "CommonHeader.h"


#ifdef LOG4CPP
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>

#if defined (WIN32) || defined(_WIN64)

#ifdef _DEBUG
#pragma comment(lib, "log4cppD.lib")
#else
#pragma comment(lib, "log4cppLIB.lib")
#endif

#endif // end WIN32

using namespace std;
namespace robin
{

	class log4wraper
	{
	public:
		enum Priority
		{
			ERROR,
			WARN,
			INFO,
			DEBUG
		};

		~log4wraper();

		inline static log4wraper& instance()
		{
			static log4wraper mlog;
			return mlog;
		}

		void loadConf()
		{
			try {

				char buffer[260];
				buffer[0] = '\0';
				size_t sz = sizeof(buffer);
				uv_exepath(buffer, &sz);
				string path = buffer;
				string confPath;
#ifdef WIN32
				ssize_t off = path.rfind('\\');
				confPath = path.substr(0, off);
				confPath += "\\log4cpp.conf";
#else
				int off = path.rfind('/');
				confPath = path.substr(0, off);
				confPath += "/log4cpp.conf";

#endif
				// "./log4cpp.conf"
				log4cpp::PropertyConfigurator::configure(confPath);
				catConfRoot = &log4cpp::Category::getInstance("rootAppender");
				printf("load log4cpp config: %s ok!\n", confPath.c_str());
#ifdef WIN32
				//int n = get_thread_amount();
				//printf("threads %d\n", n);
#endif
			}
			catch (log4cpp::ConfigureFailure& f) {
				cout << f.what() << endl;
			}
		}

		void setPriority(Priority priority);
		inline void error(const char *msg) {
			if (catConfRoot)
				catConfRoot->error(msg);
			else
				catRef_.error(msg);
		}
		inline void warn(const char *msg) {
			if (catConfRoot)
				catConfRoot->warn(msg);
			else
				catRef_.warn(msg);
		}
		inline void debug(const char *msg) {
			if (catConfRoot)
				catConfRoot->debug(msg);
			else
				catRef_.debug(msg);
		}
		inline void info(const char *msg) {
			if (catConfRoot)
				catConfRoot->info(msg);
			else
				catRef_.info(msg);
		}



	private:
		log4wraper();
		log4wraper(const log4wraper& other) = delete;
		log4wraper & operator=(const log4wraper& other) = delete;

	private:
		log4cpp::Category &catRef_;
		log4cpp::Category * catConfRoot;

	};

	inline std::string int2string(int line)
	{
		std::ostringstream os;
		os << line;
		return os.str();
	}

}// end robin


#endif  // LOG4CPP

#endif  // _MYLOG_H
