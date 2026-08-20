# STL Implementation

此项目用于学习并实现 STL 风格的容器、迭代器、算法和内存管理组件。

当前阶段只提供可构建、可测试的项目骨架。后续实现时将在 `include/stl_implementation/` 中定义公开接口，在 `src/` 中放置实现，并在 `tests/` 中补充功能、边界条件和复杂度相关测试。

在本目录中构建与测试：

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=clang++
cmake --build build
ctest --test-dir build --output-on-failure
```
