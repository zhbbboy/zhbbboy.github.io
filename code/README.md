# C++ 小项目区

`code/` 用来存放可以独立构建、测试和演进的 C++ 练习项目。它与仓库中的 Astro 博客相互独立，不会参与博客的构建流程。

## 项目

| 目录 | 目标 |
| --- | --- |
| `thread-pool/` | 实现可复用的线程池。 |
| `stl-implementation/` | 学习并实现 STL 风格的容器与算法。 |
| `object-pool/` | 实现对象池及其资源管理策略。 |

每个项目都是独立的 CMake 工程，统一包含：

- `include/`：公开头文件；
- `src/`：库实现；
- `tests/`：不依赖第三方框架的 CTest 测试；
- `README.md`：项目目标和下一步工作。

## 构建与测试

建议使用已安装的 Clang 21 和 CMake。以下命令应在某个项目目录内执行，例如 `code/thread-pool`：

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=clang++
cmake --build build
ctest --test-dir build --output-on-failure
```

该生成器使用本机的 `mingw32-make` 驱动构建，编译器仍为 Clang。三个项目均使用 C++20；本目录不以较旧的 MinGW GCC 6.3 作为兼容性基准。
