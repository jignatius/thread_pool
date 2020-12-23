cmake_minimum_required(VERSION 3.15)
project(threadpool)

set(CMAKE_CXX_STANDARD 17)

find_package(pthread)

add_executable(threadpool main.cpp threadpool.h packagedpool.h)

target_link_libraries(threadpool PUBLIC
        pthread)