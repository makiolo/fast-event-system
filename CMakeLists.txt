PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 2.8)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmaki/cmaki.cmake")
cmaki_setup()
include_directories(metacommon)
cmaki_library(fes clock.cpp)
add_subdirectory(tests)

