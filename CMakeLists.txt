cmake_minimum_required(VERSION 3.15)

project(sdb CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt CONFIG REQUIRED)
find_package(Catch2 REQUIRED)

add_subdirectory(lib)
add_subdirectory(test)