#pragma once

#include <cstddef>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string_view>
#include <thread>
#include <vector>

namespace thread_pool {

inline constexpr std::string_view project_name{"thread-pool"};

} // namespace thread_pool

class ThreadPool{
private:
	//它保存线程池创建出来的所有工作线程。
	std::vector<std::thread> workers;

	//任务队列
	std::queue<std::function<void()>> tasks;

	std::mutex mutex_;

	std::condition_variable cv_;

	bool stop_{false};

public:
	ThreadPool(std::size_t num_threads)
    {
        for (size_t i = 0; i < num_threads; i++)
        {
            workers.emplace_back(
                [this]()
                {
                    while (true)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mutex_);

                            cv_.wait(lock, [this]()
                                {
                                    return stop_ || !tasks.empty();
                                });
                            if (stop_ && tasks.empty())
                            {
                                return;
                            }
                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                        task();
                    }
                });
        }
    }


    void enqueue(std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (stop_)
            {
                return;
            }
            tasks.push(std::move(task));
        }
        cv_.notify_one();
    }

    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }

        cv_.notify_all();

        for (auto& worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }


};
