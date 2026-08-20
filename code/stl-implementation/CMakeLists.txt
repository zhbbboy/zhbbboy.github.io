cmake_minimum_required(VERSION 3.20)

project(stl_implementation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(stl_implementation STATIC
    src/project.cpp
)

target_include_directories(stl_implementation
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

enable_testing()

add_executable(stl_implementation_smoke_test
    tests/smoke_test.cpp
)

target_link_libraries(stl_implementation_smoke_test PRIVATE stl_implementation)

add_test(NAME stl_implementation.smoke COMMAND stl_implementation_smoke_test)
