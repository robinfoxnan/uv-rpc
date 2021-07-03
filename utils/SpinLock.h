#pragma once
#include <iostream>
#include <atomic>
#include <thread>
using namespace std;

// a spin lock use atomic
namespace robin
{
class spin_lock {
public:
	spin_lock() = default;
	spin_lock(const spin_lock&) = delete;
	spin_lock& operator=(const spin_lock) = delete;
	void lock() {   // acquire spin lock
		while (flag.test_and_set()) {}
	}
	void unlock() {   // release spin lock
		flag.clear();
	}
private:
	atomic_flag flag;
};
}

// std::lock_guard<spin_lock> lock(splock);

//int num = 0;
//spin_lock splock;
//void addFunc() {
//	for (int i = 0; i < 1000; ++i) {
//		splock.lock();
//		++num;
//		cout << "num = " << num << endl;
//		splock.unlock();
//	}
//}
//
//int main() {
//	cout << "started" << endl;
//	thread t1(addFunc);
//	thread t2(addFunc);
//	thread t3(addFunc);
//	t1.join();
//	t2.join();
//	t3.join();
//	return 0;
//}
