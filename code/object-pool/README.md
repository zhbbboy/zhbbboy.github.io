# Object Pool

此项目用于实现对象池以及相关的资源复用与生命周期管理策略。

当前阶段只提供可构建、可测试的项目骨架。后续实现时将在 `include/object_pool/` 中定义公开接口，在 `src/` 中放置实现，并在 `tests/` 中补充获取、归还、容量限制和异常安全等测试。

在本目录中构建与测试：

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=clang++
cmake --build build
ctest --test-dir build --output-on-failure
```
