#include "stl_implementation/bbnS.h"
#include "stl_implementation/bbnV.h"
#include "stl_implementation/bbnL.h"
#include "stl_implementation/bbnSQ.h"
#include "stl_implementation/bbnPQ.h"

#include <cstring>
#include<iostream>
#include<algorithm>

int main()
{
	bbn::string value("hello");
	value.push_back(' ');
	value += "world";
	if (std::strcmp(value.c_str(), "hello world") != 0)
	{
		return 1;
	}

	value.insert(5, ',');
	if (std::strcmp(value.c_str(), "hello, world") != 0)
	{
		return 2;
	}

	value.erase(5, 1);
	if (value.find('w') != 6 || value.rfind("lo") != 3)
	{
		return 3;
	}

	bbn::string copy(value);
	copy.resize(5);
	if (std::strcmp(copy.c_str(), "hello") != 0 || copy.size() != 5)
	{
		return 4;
	}

	bbn::vector<int> v(10, 1);
	for (int e : v)
	{
		std::cout << e << ' ';
	}

	std::cout << std::endl;

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

	return  0 ;

}
