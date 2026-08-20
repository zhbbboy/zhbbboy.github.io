cmake_minimum_required(VERSION 3.20)

project(object_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(object_pool STATIC
    src/project.cpp
)

target_include_directories(object_pool
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

enable_testing()

add_executable(object_pool_smoke_test
    tests/smoke_test.cpp
)

target_link_libraries(object_pool_smoke_test PRIVATE object_pool)

add_test(NAME object_pool.smoke COMMAND object_pool_smoke_test)
