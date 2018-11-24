cmake_minimum_required(VERSION 3.12)
project(test_task)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(test_task sample_app/main.cpp crashparser/crashparser.hpp)