add_subdirectory(verypoco)

PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 2.8)
enable_testing()

get_filename_component(FES_CURRENT_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
set(CMAKE_MODULE_PATH ${FES_CURRENT_DIR}/cmake)
set(CMAKE_PREFIX_PATH ${FES_CURRENT_DIR}/cmake)
include(utils)
ENABLE_MODERN_CPP()

add_subdirectory(animator)
add_subdirectory(multithread)
add_subdirectory(scheduler)
add_subdirectory(fes)
add_subdirectory(tests)

