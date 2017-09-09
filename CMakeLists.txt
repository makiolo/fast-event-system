PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 3.0)

set(CMAKI_REPOSITORY "http://artifacts.myftp.biz:8080")
include("$ENV{CMAKI_PWD}/node_modules/cmaki/cmaki.cmake")
cmaki_setup()

cmaki_find_package(boost-headers)
cmaki_find_package(boost-coroutine2)
cmaki_find_package(boost-filesystem)

include_directories(concurrentqueue)

cmaki2_library(fes clock.cpp PTHREADS)

add_subdirectory(tests)
