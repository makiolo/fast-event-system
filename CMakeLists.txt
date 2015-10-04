PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 2.8)
enable_testing()

IF(NOT DEFINED CMAKE_MODULE_PATH)
	set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmaki)
ENDIF()

include(utils)
ENABLE_MODERN_CPP()
GENERATE_LIB(fes DEPENDENCIES multithread)

add_subdirectory(multithread)
add_subdirectory(tests)
# add_subdirectory(tests2)

