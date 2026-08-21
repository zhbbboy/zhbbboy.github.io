cmake_minimum_required(VERSION 3.20)

project(cpp_learning_projects LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_subdirectory(thread-pool)
add_subdirectory(stl-implementation)
add_subdirectory(object-pool)
