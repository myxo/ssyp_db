cmake_minimum_required(VERSION 3.15)

project(sdb CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory(lib)
add_subdirectory(test)
add_subdirectory(benchmark)
