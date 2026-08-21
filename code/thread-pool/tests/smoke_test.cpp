#include "thread_pool/project.hpp"
#include <iostream>
#include <chrono>
#include <iostream>
#include <string>

int add(int a, int b)
{
	std::this_thread::sleep_for(
		std::chrono::milliseconds(500));

	return a + b;
}

int main()
{
	ThreadPool pool(4);

	// 1. 无返回值任务
	pool.enqueue(
		[]()
		{
			std::cout
				<< "void task, thread = "
				<< std::this_thread::get_id()
				<< '\n';
		});

	// 2. 有返回值任务
	auto future1 =
		pool.enqueue(add, 10, 20);

	// 3. lambda + int 返回值
	auto future2 =
		pool.enqueue(
			[](int a, int b)
			{
				return a * b;
			},
			5,
			6);

	// 4. 返回 string
	auto future3 =
		pool.enqueue(
			[](std::string name)
			{
				return std::string("Hello ") + name;
			},
			std::string("ThreadPool"));

	// 5. 提交大量任务
	std::vector<std::future<int>> results;

	for (int i = 0; i < 10; ++i)
	{
		results.emplace_back(
			pool.enqueue(
				[i]()
				{
					std::cout
						<< "Task "
						<< i
						<< " executing on thread "
						<< std::this_thread::get_id()
						<< '\n';

					std::this_thread::sleep_for(
						std::chrono::milliseconds(200));

					return i * i;
				}));
	}

	std::cout
		<< "future1 = "
		<< future1.get()
		<< '\n';

	std::cout
		<< "future2 = "
		<< future2.get()
		<< '\n';

	std::cout
		<< "future3 = "
		<< future3.get()
		<< '\n';

	for (auto& result : results)
	{
		std::cout
			<< "result = "
			<< result.get()
			<< '\n';
	}

	return 0;
}