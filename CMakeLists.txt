add_subdirectory(verypoco)

PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 2.8)
enable_testing()

include(utils)
ENABLE_MODERN_CPP()

add_subdirectory(animator)
add_subdirectory(multithread)
add_subdirectory(scheduler)
add_subdirectory(fes)
add_subdirectory(tests)

