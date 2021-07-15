
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <chrono>
#include <ratio>
#include <deque>
#include <atomic>
#include <mutex>


#include <uv.h>
#include "../include/queue.h"
#include "../include/MyTimer.h"
#include "../include/spin_lock.h"


struct Student {
	Student(int a, const char * s) :age(a), name(s)
	{
	}

	Student() = default;
	QUEUE node;
	int age;
	std::string name;

};

class StudentAuto : public enable_shared_from_this<StudentAuto>
{
public:
	StudentAuto(int a, const char * s) :age(a), name(s)
	{
	}

	StudentAuto() = default;

	std::shared_ptr<StudentAuto> getptr() {
		return shared_from_this();
	}

	~StudentAuto()
	{
		//std::cout << "析构StudentAuto" << endl;
	}
	QUEUE node;
	int age;
	std::string name;

};

void test4()
{
	// 在UV插入信息

	Timer timer1;
	timer1.start();

	QUEUE queue;
	QUEUE_INIT(&queue);

	for (int i = 0; i < 10000; i++)
	{
		std::shared_ptr<StudentAuto> it = make_shared<StudentAuto>(i + 1, "robin");

		QUEUE_INIT(&(it->node));
		QUEUE_INSERT_TAIL(&queue, &(it->node));
	}

	auto duration1 = timer1.stop_delta<std::milli>();
	std::cout << "插入10000智能对象UV队列(ms)：" << duration1 << endl;
}

// 系统忙
//插入UV队列(ms)：0.1168
//头部弹出(ms)： 0.1078
//插入std::deque(ms)： 20.5516
//弹出std::deque(ms)： 32.1022

// 空闲
//插入UV队列(ms)：0.045
//头部弹出(ms)： 0.0611
//插入std::deque(ms)： 7.4969
//弹出std::deque(ms)： 7.3878
void test1()
{
	Student * students = new Student[10000];
	Student * it = students;
	for (int i = 0; i < 10000; i++)
	{
		it = students + i;
		it->age = i + 1;
		it->name = "robin";
	}

	Timer timer1;
	timer1.start();

	// 在UV插入信息
	QUEUE queue;
	QUEUE_INIT(&queue);

	for (int i = 0; i < 10000; i++)
	{
		it = students + i;

		QUEUE_INIT(&(it->node));
		QUEUE_INSERT_TAIL(&queue, &(it->node));
	}



	auto duration1 = timer1.stop_delta<std::milli>();
	std::cout << "插入UV队列(ms)：" << duration1 << endl;

	// 头部弹出
	QUEUE *p;
	timer1.start();
	for (int i = 0; i < 10000; i++)
	{
		p = QUEUE_HEAD(&queue);
		if (p != &queue)
		{
			QUEUE_REMOVE(p);
		}
	}
	duration1 = timer1.stop_delta<std::milli>();
	std::cout << "头部弹出(ms)： " << duration1 << endl;

	// 测试std::deque
	timer1.start();
	std::deque<Student *> studentQue;

	for (int i = 0; i < 10000; i++)
	{
		it = students + i;
		studentQue.push_back(it);
	}

	duration1 = timer1.stop_delta<std::milli>();
	std::cout << "插入std::deque (ms)： " << duration1 << endl;

	timer1.start();
	for (int i = 0; i < 10000; i++)
	{
		studentQue.pop_front();
	}
	duration1 = timer1.stop_delta<std::milli>();
	std::cout << "弹出std::deque (ms)： " << duration1 << endl;

	// 取出头测试
	/*QUEUE *p;
	p = QUEUE_HEAD(&queue);
	p = QUEUE_NEXT(p);
	Student *first_stu = QUEUE_DATA(p, struct Student, node);*/

	/**
	* Should output the name of wesley.
	*/
	/*printf("Received first inserted Student: %s who is %d.\n",
		first_stu->name.c_str(), first_stu->age);
*/
/*QUEUE_FOREACH(p, &queue) {
	Student *tmp = QUEUE_DATA(p, struct Student, node);
	cout << "name: " << tmp->name << " age: " << tmp->age << endl;

}*/

	delete[] students;
}
// 系统忙
//插入UV队列(ms)：17.3848
//插入std::deque(ms)： 50.7232
//弹出std::deque(ms)： 46.7094

// 系统空闲
//插入UV队列(ms)：9.0955
//插入std::deque(ms)： 19.8008
//弹出std::deque(ms)： 14.6486
void test2()
{

	// 在UV插入信息

	Timer timer1;
	timer1.start();

	Student * students = new Student[10000];
	Student * it = students;
	for (int i = 0; i < 10000; i++)
	{
		it = students + i;
		it->age = i + 1;
		it->name = "robin";
	}
	QUEUE queue;
	QUEUE_INIT(&queue);

	for (int i = 0; i < 10000; i++)
	{
		it = students + i;

		QUEUE_INIT(&(it->node));
		QUEUE_INSERT_TAIL(&queue, &(it->node));
	}

	auto duration1 = timer1.stop_delta<std::milli>();
	std::cout << "插入UV队列(ms)：" << duration1 << endl;
	/////////////////////////////////////////////////////////
	timer1.start();
	std::deque<Student> studentQue;
	for (int i = 0; i < 10000; i++)
	{
		studentQue.emplace_back(i + 1, "robin");
	}

	duration1 = timer1.stop_delta<std::milli>();
	std::cout << "插入std::deque (ms)： " << duration1 << endl;

	timer1.start();
	for (int i = 0; i < 10000; i++)
	{
		//cout << studentQue[0].age << endl;
		studentQue.pop_front();
	}
	duration1 = timer1.stop_delta<std::milli>();
	std::cout << "弹出std::deque (ms)： " << duration1 << endl;

	delete[] students;
}

//插入10000std::deque智能指针(ms)： 64.094
//插入10000std::deque对象(ms)： 55.6962
using StudentPtr = std::shared_ptr<Student>;
void test3()
{

	Timer timer1;
	timer1.start();

	std::deque<StudentPtr> studentQ;
	for (int i = 0; i < 10000; i++)
	{
		studentQ.push_back(std::make_shared<Student>(i + 1, "robin"));
	}
	auto duration1 = timer1.stop_delta<std::milli>();
	std::cout << "插入10000std::deque智能指针 (ms)： " << duration1 << endl;

	timer1.start();
	std::deque<Student> studentQue;
	for (int i = 0; i < 10000; i++)
	{
		studentQue.emplace_back(i + 1, "robin");
	}

	duration1 = timer1.stop_delta<std::milli>();
	std::cout << "插入10000std::deque对象(ms)： " << duration1 << endl;

}

// 对象与智能对象对比
void test5()
{
	Timer timer1;
	timer1.start();

	// 内存泄露
	Student * it = nullptr;
	for (int i = 0; i < 10000; i++)
	{
		it = new Student(i + 1, "robin");
	}

	auto duration1 = timer1.stop_delta<std::milli>();
	std::cout << "生成10000 简单实体(ms)： " << duration1 << endl;

	timer1.start();

	std::shared_ptr<Student> ptr[10000];
	for (int i = 0; i < 10000; i++)
	{
		ptr[i] = make_shared<Student>(i + 1, "robin");
		//std::shared_ptr<Student> it = make_shared<Student>(i + 1, "robin");
	}

	auto duration2 = timer1.stop_delta<std::milli>();
	std::cout << "生成10000 智能实体(ms)： " << duration2 << endl;
}

///////////////////////////////////////////////////////////////////
spin_lock splock;
std::mutex myMutex;
int num = 0;
std::atomic<int> count1(0);
void addFunc1()
{
	for (int i = 0; i < 1000000; ++i)
	{
		//std::lock_guard<spin_lock> lock(splock); // 就是这里不一样！！！
		//num++;
		count1++;
	}
}

void addFunc2()
{
	for (int i = 0; i < 1000000; ++i)
	{
		std::lock_guard<std::mutex> lock(myMutex); // 就是这里不一样！！！
		num++;

	}
}

void testThreads()
{
	Timer timer1;
	timer1.start();
	thread t1(addFunc1);
	thread t2(addFunc1);
	thread t3(addFunc1);
	t1.join();
	t2.join();
	t3.join();
	auto duration2 = timer1.stop_delta<std::milli>();
	std::cout << "耗时(ms)： " << duration2 << endl;
	std::cout << count1 << endl;
}
