PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 2.8)
enable_testing()

if(WIN32)
	add_definitions(/GR-)
	#add_definitions(/D_HAS_EXCEPTIONS=0)
else()
	add_definitions(-fno-rtti)
	add_definitions(-fno-exceptions)
endif()

if (NOT DEFINED EXTRA_DEF)
	include(CheckCXXCompilerFlag)
	#CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX1Z)
	CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
	CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
	CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

	#if(COMPILER_SUPPORTS_CXX1Z)
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
	#message("-- C++1z Enabled")
	if(COMPILER_SUPPORTS_CXX14)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
		message("-- C++14 Enabled")
	elseif(COMPILER_SUPPORTS_CXX1Y)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
		message("-- C++1y Enabled")
	elseif(COMPILER_SUPPORTS_CXX11)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
		message("-- C++11 Enabled")
	elseif(COMPILER_SUPPORTS_CXX0X)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
		message("-- C++0x Enabled")
	else()
		message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
	endif()
else()
	add_definitions(${EXTRA_DEF})
endif()
if (NOT MSVC)
	SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -pedantic" )
	SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread" )
	SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -lpthread" )
endif()

# Generate .clang_complete for full completation in vim + clang_complete
set(extra_parameters "")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  set(extra_parameters ${extra_parameters} -I${dir})
endforeach()
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
foreach(dir ${dirs})
  set(extra_parameters ${extra_parameters} -D${dir})
endforeach()
STRING(REGEX REPLACE ";" "\n" extra_parameters "${extra_parameters}")
FILE(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/.clang_complete" "${extra_parameters}\n")

foreach(BUILD_TYPE ${CMAKE_BUILD_TYPE})
	INSTALL(    TARGETS ${LIBNAME}
				DESTINATION ${BUILD_TYPE}
				CONFIGURATIONS ${BUILD_TYPE})
endforeach()

INCLUDE_DIRECTORIES(include/)
ADD_LIBRARY(fast-event-system SHARED src/fast-event-system/fes.cpp src/scheduler/sas.cpp)

file(GLOB HEADER_CODE include/scheduler/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/scheduler")

file(GLOB HEADER_CODE include/fast-event-system/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/fast-event-system")
            
file(GLOB HEADER_CODE include/concurrentqueue/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/concurrentqueue")
            
file(GLOB HEADER_CODE include/concurrentqueue/internal/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/concurrentqueue/internal")
            
file(GLOB HEADER_CODE include/animator/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/animator")

ADD_EXECUTABLE(test_leader_follower tests/test_leader_follower.cpp)
target_link_libraries(test_leader_follower fast-event-system)
ADD_TEST(NAME test_leader_follower COMMAND test_leader_follower WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(test_fes tests/test_fes.cpp)
target_link_libraries(test_fes fast-event-system)
ADD_TEST(NAME test_fes COMMAND test_fes WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(test_autoasociative_memory tests/test_autoasociative_memory.cpp)
target_link_libraries(test_autoasociative_memory fast-event-system)
ADD_TEST(NAME test_autoasociative_memory COMMAND test_autoasociative_memory WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

