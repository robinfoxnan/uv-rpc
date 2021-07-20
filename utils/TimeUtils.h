#pragma once
#include <chrono>
#include <string>
#include <time.h>
#include <time.h>
#include <ctime>

#include <iostream>
#include <iomanip>      // std::put_time
#include <sstream>
#include <sys/timeb.h>

namespace robin
{
	// 格式化时间字符串
	/*
	localtime_r也是用来获取系统时间，运行于linux平台下
	函数原型为struct tm *localtime_r(const time_t *timep, struct tm *result);
	localtime_s也是用来获取系统时间，运行于windows平台下，与localtime_r只有参数顺序不一样
	*/
	std::string getTimeNow()
	{
		//printf("----%s----%d---\n", __FUNCTION__, __LINE__);
		
		auto now = std::chrono::system_clock::now();
		time_t tt = std::chrono::system_clock::to_time_t(now);
		struct tm tm;

#if defined(WIN32) || defined(_WIN32)
		localtime_s(&tm, &tt);  // 转成tm结构
#elif  defined(__linux) || defined(__linux__) 
		localtime_r(&tt, &tm);	
#endif	
		int minsecs = tt & 1000;
		std::ostringstream stream;
		stream << std::put_time(&tm, "%F %T");
		stream << ".";
		stream << minsecs;
		return stream.str();
	}

	std::time_t getTimeStamp()
	{
		std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
		auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
		std::time_t timestamp = tmp.count();
		//std::time_t timestamp = std::chrono::system_clock::to_time_t(tp);
		return timestamp;
	}

	void getTimeStamp(uint64_t & sec, uint32_t & msec)
	{
		timeb now;
		ftime(&now);
		sec = now.time;
		msec = now.millitm;
		//return now.time * 1000 + now.millitm;
	}


	/*
	 * 关于时间的几个库函数查看手册得知asctime(), ctime(), gmtime()， localtime()都是不安全的，
	 * 因为这几个函数返回一个指针，这个指针指向一段静态内存区，所以是线程不安全的，
	 * 这四个函数的线程安全版本是asctime_r(), ctime_r(),gmtime_r()， localtime_r()
	*/

	/*std::string getTimeNowold()
	{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, 80, "Now it's %I:%M%p.", timeinfo);
		return buffer;
	}
*/
}