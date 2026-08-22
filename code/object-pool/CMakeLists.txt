cmake_minimum_required(VERSION 3.20)

project(object_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(object_pool STATIC
    src/objectPool.cpp
    include/object_pool/objectPool.h
)

target_include_directories(object_pool
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

enable_testing()

add_executable(objectPool_test
    tests/objectPool_test.cpp
)

target_link_libraries(objectPool_test PRIVATE object_pool)

add_test(NAME objectPool_test COMMAND objectPool_test)
