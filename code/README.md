# C++ 小项目区

`code/` 用来存放可以独立构建、测试和演进的 C++ 练习项目。它与仓库中的 Astro 博客相互独立，不会参与博客的构建流程。

## 项目

| 目录 | 目标 |
| --- | --- |
| `thread-pool/` | 实现可复用的线程池。 |
| `stl-implementation/` | 学习并实现 STL 风格的容器与算法。 |
| `object-pool/` | 实现对象池及其资源管理策略。 |

`code/CMakeLists.txt` 是三个项目的顶层 CMake 工程；每个子目录仍可独立配置和构建。项目统一包含：

- `include/`：公开头文件；
- `src/`：库实现；
- `tests/`：不依赖第三方框架的 CTest 测试；
- `README.md`：项目目标和下一步工作。

## 在 Visual Studio 中使用

在 Visual Studio 2022 中选择“打开本地文件夹”，并打开 `D:\my-blog\code`。Visual Studio 会识别顶层 `CMakeLists.txt`，在同一窗口显示三个库目标及其 smoke test，可统一构建和调试。

本地 IDE 状态和构建输出会被 `code/.gitignore` 忽略，不应提交。

## 统一构建与测试

建议使用已安装的 Clang 21 和 CMake。以下命令应在 `code/` 目录内执行：

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=clang++
cmake --build build
ctest --test-dir build --output-on-failure
```

如需只构建一个 smoke test，可在第二条命令中指定目标，例如 `--target thread_pool_smoke_test`。

该生成器使用本机的 `mingw32-make` 驱动构建，编译器仍为 Clang。三个项目均使用 C++20；本目录不以较旧的 MinGW GCC 6.3 作为兼容性基准。
