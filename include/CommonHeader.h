#pragma once
// std::*
#include <string>
#include <vector>
#include <thread>
#include <map>
#include <memory>
#include <deque>
#include <iostream>
#include <functional>
#include <atomic>
#include <mutex>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include <iostream>
#include <assert.h>

using namespace std;
namespace robin
{
	// used in Async Event callback queue
	using DefaultCallback = std::function<void()>;


}
#define DEFAUTL_BUF_SZ 1024 *64
#define DEFAULT_VEC_SZ 1024

#if defined (WIN32) || defined(_WIN64)

// static lib
#pragma comment(lib, "uv_a.lib")

#pragma comment(lib,"ws2_32.lib")
#pragma comment (lib,"Advapi32.lib")
#pragma comment (lib,"Iphlpapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "userenv.lib")

#else  // linux

#define _atoi64(val)     strtoll(val, NULL, 10)

#endif
