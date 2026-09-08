#include "stl_implementation/bbnS.h"
#include "stl_implementation/bbnV.h"
#include "stl_implementation/bbnL.h"
#include "stl_implementation/bbnSQ.h"
#include "stl_implementation/bbnPQ.h"
#include "stl_implementation/bbnHT.h"
#include "stl_implementation/bbnUnorderedMS.h"
#include "stl_implementation/bbnAVL.h"
#include "stl_implementation/bbnbitset.h"
#include <cstring>
#include<iostream>
#include<algorithm>

using namespace std;

// 测试unordered_set的功能
void test_set() {
	bbn::unordered_set<int> s;
	int a[] = { 4,2,6,1,3,5,15,7,16,14,3,3,15 };
	for (auto e : a) {
		s.insert(e);
	}
	for (auto e : s) {
		cout << e << " ";
	}
	cout << endl;
	bbn::unordered_set<int>::iterator it = s.begin();
	while (it != s.end()) {
		cout << *it << " ";
		++it;
	}
	cout << endl;
}

// 测试unordered_map的功能
void test_map() {
	bbn::unordered_map<string, string> dict;
	dict.insert({ "sort", "排序" });
	dict.insert({ "Left", "左边" });
	dict.insert({ "right", "右边" });
	dict["left"] = "左边,剩余";
	dict["insert"] = "插入";
	dict["string"];
	bbn::unordered_map<string, string>::iterator it = dict.begin();
	while (it != dict.end()) {
		it->second += 'x';
		cout << it->first << ":" << it->second << endl;
		++it;
	}
	cout << endl;
}

void TestAVLTree1()
{
	int a[] = { 4, 2, 6, 1, 3, 5, 15, 7, 16, 14 };
	//int a[] = { 16, 3, 7, 11, 9, 26, 18, 14, 15 };
	bbn::AVLTree<int, int > t;
	for (auto e : a)
	{
		if (e == 14)
		{
			int x = 0;
		}
		t.Insert(make_pair(e, e));
		// 1、先看是插入谁导致出现的问题
		// 2、打条件断点，画出插入前的树
		// 3、单步跟踪，对比图一一分析细节原因
		std::cout << e << "->" << t.IsBalance() << endl;
	}
	t.InOrder();
	cout << t.IsBalance() << endl;
}

void test_string()
{
	bbn::string value("hello");
	value.push_back(' ');
	value += "world";
	if (std::strcmp(value.c_str(), "hello world") != 0)
	{
		cout << 1 << endl;
		return;
	}

	value.insert(5, ',');
	if (std::strcmp(value.c_str(), "hello, world") != 0)
	{
		cout << 2 << endl;
		return;
	}

	value.erase(5, 1);
	if (value.find('w') != 6 || value.rfind("lo") != 3)
	{
		cout << 3 << endl;
		return;
	}

	bbn::string copy(value);
	copy.resize(5);
	if (std::strcmp(copy.c_str(), "hello") != 0 || copy.size() != 5)
	{
		cout << 4 << endl;
		return;
	}

	bbn::vector<int> v(10, 1);
	for (int e : v)
	{
		std::cout << e << ' ';
	}

	std::cout << std::endl;
}

void test_list()
{
	bbn::list<int> lt;
	lt.push_back(3);
	lt.push_back(5);
	lt.push_back(7);
	lt.push_back(6);
	lt.push_back(9);
	lt.push_back(10);

	for (int e : lt)
	{
		std::cout << e << ' ';
	}
	std::cout << std::endl;

	lt.push_front(2);
	lt.push_front(1);
	auto it = lt.begin();

	while (it != lt.end())
	{
		if (*it == 7)
		{
			it = lt.erase(it);
		}
		else
		{
			it++;
		}
	}


	for (int e : lt)
	{
		std::cout << e << ' ';
	}
	std::cout << std::endl;
}

void test_stack_and_queue()
{
	bbn::stack<int> st;
	st.push(1);
	st.push(2);
	st.push(3);
	st.push(4);
	st.push(5);
	std::cout << st.size() << std::endl; //4
	while (!st.empty())
	{
		std::cout << st.top() << " ";
		st.pop();
	}
	std::cout << std::endl;

	bbn::queue<int, bbn::list<int>> q;
	q.push(1);
	q.push(2);
	q.push(3);
	q.push(4);
	std::cout << q.size() << std::endl; //4
	while (!q.empty())
	{
		std::cout << q.front() << " ";
		q.pop();
	}
	std::cout << std::endl;
}

void test_priority_queue()
{
	bbn::priority_queue<int> pqL;
	pqL.push(4);
	pqL.push(1);
	pqL.push(5);
	pqL.push(7);
	pqL.push(9);
	while (!pqL.empty())
	{
		std::cout << pqL.top() << " ";
		pqL.pop();
	}
	std::cout << std::endl;

	bbn::priority_queue<int, std::vector<int>, std::greater<int>> pqG;

	pqG.push(4);
	pqG.push(1);
	pqG.push(5);
	pqG.push(7);
	pqG.push(9);
	while (!pqG.empty())
	{
		std::cout << pqG.top() << " ";
		pqG.pop();
	}
	std::cout << std::endl;
}

void test_custom_bitset() {
	try {
		// 定义一个覆盖0~99的位图
		bbn::bitset<100> bs;

		// 测试set和test
		bs.set(5);
		bs.set(10);
		bs.set(99);
		std::cout << "5是否存在：" << (bs.test(5) ? "是" : "否") << std::endl;   // 是
		std::cout << "8是否存在：" << (bs.test(8) ? "是" : "否") << std::endl;   // 否
		std::cout << "99是否存在：" << (bs.test(99) ? "是" : "否") << std::endl; // 是

		// 测试reset
		bs.reset(5);
		std::cout << "取消标记后，5是否存在：" << (bs.test(5) ? "是" : "否") << std::endl; // 否

		// 测试flip
		bs.flip(10);
		std::cout << "翻转后，10是否存在：" << (bs.test(10) ? "是" : "否") << std::endl; // 否
		bs.flip(10);
		std::cout << "再翻转后，10是否存在：" << (bs.test(10) ? "是" : "否") << std::endl; // 是

		// 测试count和any、none
		std::cout << "置1的比特位个数：" << bs.count() << std::endl; // 2（10和99）
		std::cout << "是否存在置1的比特位：" << (bs.any() ? "是" : "否") << std::endl; // 是
		std::cout << "是否所有比特位都为0：" << (bs.none() ? "是" : "否") << std::endl; // 否

		// 测试全重置
		bs.reset();
		std::cout << "全重置后，置1的比特位个数：" << bs.count() << std::endl; // 0
		std::cout << "全重置后，是否所有比特位都为0：" << (bs.none() ? "是" : "否") << std::endl; // 是

		// 测试超出范围（会抛出异常）
		// bs.set(100);
	}
	catch (const std::exception& e) {
		std::cerr << "异常：" << e.what() << std::endl;
	}
}

int main()
{
	//test_string();
	//test_list();
	//test_stack_and_queue();
	//test_priority_queue();
	//test_set();
	//test_map();
	//TestAVLTree1();
	test_custom_bitset();
	return  0;

}
