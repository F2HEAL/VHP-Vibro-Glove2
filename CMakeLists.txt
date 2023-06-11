cmake_minimum_required(VERSION 3.0.0)
project (sstream-test)

SET(CXX_EXTRA_FLAGS "-O -Wall -std=gnu++11 -ggdb")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CXX_EXTRA_FLAGS}")

add_executable(sstream-test SStream-test.cpp)
