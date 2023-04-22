cmake_minimum_required(VERSION 3.10.2)
project(bookstore)

include_directories(./third_party)
set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -Og")

add_executable(code ${src_dir} BPlusTree/main.cpp)