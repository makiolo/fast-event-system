# out of project
add_subdirectory(verypoco)

PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 2.8)

get_filename_component(FES_CURRENT_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
include(${FES_CURRENT_DIR}/cmake/utils.cmake)
ENABLE_MODERN_CPP()

add_subdirectory(animator)
add_subdirectory(multithread)
add_subdirectory(scheduler)
add_subdirectory(fes)
add_subdirectory(tests)


