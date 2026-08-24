cmake_minimum_required(VERSION 3.20)

project(stl_implementation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(stl_implementation STATIC
    src/bbnS.cpp
    include/stl_implementation/bbnS.h
    src/bbnV.cpp
    include/stl_implementation/bbnV.h
)

target_include_directories(stl_implementation
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

enable_testing()

add_executable(STL_test
    tests/STL_test.cpp
)

target_link_libraries(STL_test PRIVATE stl_implementation)

if (MSVC)
    # bbnS.cpp 中包含中文学习注释，明确让 MSVC 按 UTF-8 读取源文件。
    target_compile_options(stl_implementation PRIVATE /utf-8)
    target_compile_options(STL_test PRIVATE /utf-8)
endif()

add_test(NAME STL_test COMMAND STL_test)
