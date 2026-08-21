#pragma once

#include <cstddef>
#include <string_view>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace thread_pool {

inline constexpr std::string_view project_name{"thread-pool"};

} // namespace thread_pool

class ThreadPool
{
public:
	explicit ThreadPool(std::size_t threadCount)
		: stop_(false)
	{
		if (threadCount == 0)
		{
			throw std::invalid_argument(
				"threadCount must be greater than zero");
		}

		for (std::size_t i = 0; i < threadCount; ++i)
		{
			workers_.emplace_back(
				[this]()
				{
					while (true)
					{
						std::function<void()> task;

						{
							std::unique_lock<std::mutex> lock(mutex_);

							cv_.wait(
								lock,
								[this]()
								{
									return stop_ || !tasks_.empty();
								});

							// 已经停止，并且任务全部执行完
							if (stop_ && tasks_.empty())
							{
								return;
							}

							task = std::move(tasks_.front());
							tasks_.pop();
						}

						// 不持锁执行任务
						task();
					}
				});
		}
	}

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	template<typename F, typename... Args>
	auto enqueue(F&& f, Args&&... args)
		-> std::future<
		std::invoke_result_t<F, Args...>>
	{
		using ReturnType =
			std::invoke_result_t<F, Args...>;

		auto task =
			std::make_shared<
			std::packaged_task<ReturnType()>>(
				std::bind(
					std::forward<F>(f),
					std::forward<Args>(args)...));

		std::future<ReturnType> future =
			task->get_future();

		{
			std::lock_guard<std::mutex> lock(mutex_);

			if (stop_)
			{
				throw std::runtime_error(
					"enqueue on stopped ThreadPool");
			}

			tasks_.emplace(
				[task]()
				{
					(*task)();
				});
		}

		cv_.notify_one();

		return future;
	}

	~ThreadPool()
	{
		{
			std::lock_guard<std::mutex> lock(mutex_);
			stop_ = true;
		}

		cv_.notify_all();

		for (auto& worker : workers_)
		{
			if (worker.joinable())
			{
				worker.join();
			}
		}
	}

private:
	// 工作线程
	std::vector<std::thread> workers_;

	// 所有任务最终擦除成 void()
	std::queue<std::function<void()>> tasks_;

	// 保护 tasks_ 和 stop_
	std::mutex mutex_;

	// worker 等待 / 唤醒
	std::condition_variable cv_;

	// 是否正在关闭
	bool stop_;
};
