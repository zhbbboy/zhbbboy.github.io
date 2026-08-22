cmake_minimum_required(VERSION 3.20)

project(thread_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(thread_pool STATIC
    src/threadPool.cpp
    include/thread_pool/threadPool.h
)

target_include_directories(thread_pool
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

enable_testing()

add_executable(threadPool_test
    tests/threadPool_test.cpp
)

target_link_libraries(threadPool_test PRIVATE thread_pool)

add_test(NAME threadPool_test COMMAND threadPool_test)
