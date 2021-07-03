#pragma once


#include <chrono>
#include <ratio>

using namespace std::chrono;
namespace robin
{
class Timer {
public:
	using s = std::ratio<1, 1>;
	using ms = std::ratio<1, 1000>;
	using us = std::ratio<1, 1000000>;
	using ns = std::ratio<1, 1000000000>;
	using h_time_point = time_point<high_resolution_clock>;

public:
	Timer() : tpStart(high_resolution_clock::now()), tpStop(tpStart) {}

public:
	static h_time_point now() { return high_resolution_clock::now();}
	h_time_point start() { tpStart = high_resolution_clock::now(); return tpStart;  }
	void restart() { tpStart = high_resolution_clock::now(); }
	void stop() { tpStop = high_resolution_clock::now(); }

	template <typename span>
	auto delta() const { return duration<double, span>(high_resolution_clock::now() - tpStart).count(); }

	template <typename span>
	auto delta_restart() {
		auto ts = duration<double, span>(high_resolution_clock::now() - tpStart).count();
		start();
		return ts;
	}

	template <typename span>
	auto stop_delta() { stop(); return duration<double, span>(tpStop - tpStart).count(); }

	template <typename span>
	auto stop_delta_start() {
		stop();
		auto ts = duration<double, span>(tpStop - tpStart).count();
		start();
		return ts;
	}

private:
	time_point<high_resolution_clock> tpStart;
	time_point<high_resolution_clock> tpStop;
};
}
