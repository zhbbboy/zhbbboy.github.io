cmake_minimum_required(VERSION 3.20)

project(thread_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(thread_pool STATIC
    src/project.cpp
)

target_include_directories(thread_pool
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

enable_testing()

add_executable(thread_pool_smoke_test
    tests/smoke_test.cpp
)

target_link_libraries(thread_pool_smoke_test PRIVATE thread_pool)

add_test(NAME thread_pool.smoke COMMAND thread_pool_smoke_test)
