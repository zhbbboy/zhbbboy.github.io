#include "thread_pool/project.hpp"
#include <iostream>
int main() {
    ThreadPool pool(4);

    for (int i = 0; i < 10; i++)
    {
        pool.enqueue([i]()
            {
                std::cout
                    << "task " << i
                    << ", thread id ="
                    << std::this_thread::get_id()
                    << '\n';
            });
    }
}
