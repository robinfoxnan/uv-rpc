#pragma once

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class NodeWeight
{
public:
	NodeWeight(int32_t w): weight(w)
	{

	}
	~NodeWeight()
	{

	}
	int32_t weight;
	void * data;
};

class NodeCmp
{
public:
	bool operator()(NodeWeight& node1, NodeWeight& node2)
	{
		if (node1.weight > node2.weight)
			return true;
		else
			return false;
	}
};



class NodeHeap
{
public:
	void printHeap() {
		for (auto t : vec)
			cout << t.weight << ", ";
		cout << endl;
	}

	void makeHeap()
	{
		if (vec.size() < 1)
			return;
		std::make_heap(vec.begin(), vec.end(), cmp);
	}
	
	void push(NodeWeight& node)
	{
		vec.emplace_back(node);
	}

	void push(NodeWeight&& node)
	{
		vec.emplace_back(std::forward<NodeWeight>(node));
	}

	void pushHeap(NodeWeight& node)
	{
		vec.emplace_back(node);
		std::push_heap(vec.begin(), vec.end(), cmp);
	}

	NodeWeight * getTopRef()
	{
		if (vec.size() < 1)
			return nullptr;

		return &vec[0];
	}
		
	void popHeap()
	{
		if (vec.size() < 1)
			return ;

		//删除堆顶元素
		std::pop_heap(vec.begin(), vec.end(), cmp);
		//9 10 20 30 15 22 6  不为小顶堆 这个pop_heap操作后，实际上是把堆顶元素放到了末尾
		//printHeap(min);
		vec.pop_back();//这才彻底在底层vector数据容器中删除
		return ;
	}

private:
	NodeCmp cmp;
	std::vector<NodeWeight> vec;
};

/*
void test()
{
	NodeHeap minHeap;
	std::vector<int> vec = {10,30,22,25,6,15,9};
	for (auto t : vec)
	{
		minHeap.push(NodeWeight(t));
	}

	minHeap.makeHeap();
	minHeap.printHeap();  //6, 10, 9, 25, 30, 15, 22,
	
	

	// test for dispatch 
	NodeWeight * top = minHeap.getTopRef();
	if (top)
	{
		top->weight = 100;
		minHeap.makeHeap();
		minHeap.printHeap();  //9, 10, 15, 25, 30, 100, 22,
	}

	NodeWeight node(2);
	minHeap.pushHeap(node);
	minHeap.printHeap();  // 2, 9, 15, 10, 30, 100, 22, 25,

	minHeap.popHeap();
	minHeap.printHeap();  // 9, 10, 15, 25, 30, 100, 22,
}

*/


