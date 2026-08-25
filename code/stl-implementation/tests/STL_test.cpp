#include "stl_implementation/bbnS.h"
#include "stl_implementation/bbnV.h"
#include "stl_implementation/bbnL.h"

#include <cstring>
#include<iostream>
#include<algorithm>

int main()
{
	bbnS::string value("hello");
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

	bbnS::string copy(value);
	copy.resize(5);
	if (std::strcmp(copy.c_str(), "hello") != 0 || copy.size() != 5)
	{
		return 4;
	}

	bbnV::vector<int> v(10, 1);
	for (int e : v)
	{
		std::cout << e << ' ';
	}

	std::cout << std::endl;

	bbnL::list<int> lt;
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

	return copy < value ? 0 : 5;

}
