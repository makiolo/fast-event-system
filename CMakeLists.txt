PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 3.0)

include("$ENV{CMAKI_PWD}/node_modules/cmaki/cmaki.cmake")
cmaki_find_package(boost-headers)
cmaki_find_package(boost-coroutine2)
cmaki_find_package(boost-filesystem)

include_directories(concurrentqueue)

cmaki2_library(fes clock.cpp PTHREADS)

add_subdirectory(tests)
