cmake_minimum_required(VERSION 3.8)
project(new_project_test)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp lsh.cpp)
add_executable(index ${SOURCE_FILES})