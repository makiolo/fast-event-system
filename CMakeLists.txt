PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 2.8)

set(CMAKI_REPOSITORY "http://artifacts.myftp.biz:8080")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmaki/cmaki.cmake")
cmaki_setup()

cmaki_find_package(boost-headers)
cmaki_find_package(boost-coroutine2)
cmaki_find_package(boost-filesystem)
cmaki_find_package(boost-signals)
cmaki2_library(fes clock.cpp PTHREADS)

add_subdirectory(tests)
