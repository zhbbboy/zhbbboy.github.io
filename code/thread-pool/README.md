# Thread Pool

此项目用于实现一个可复用的 C++ 线程池。

当前阶段只提供可构建、可测试的项目骨架。后续实现时将在 `include/thread_pool/` 中定义公开接口，在 `src/` 中放置实现，并在 `tests/` 中补充并发、任务提交、关闭和异常传播等测试。

在本目录中构建与测试：

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=clang++
cmake --build build
ctest --test-dir build --output-on-failure
```
