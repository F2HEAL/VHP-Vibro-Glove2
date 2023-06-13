cmake_minimum_required(VERSION 3.0.0)
project (sstream-test)

include_directories(${CMAKE_SOURCE_DIR})

SET(CXX_EXTRA_FLAGS " -Wall -std=gnu++11 -ggdb")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CXX_EXTRA_FLAGS}")

add_executable(sstream-test tests/SStream-test.cpp)
add_executable(samplecache-test tests/SampleCache-test.cpp)
