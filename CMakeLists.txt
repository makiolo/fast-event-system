PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 2.8)
enable_testing()

add_subdirectory(verypoco)

if(WIN32)
	add_definitions(/EHsc)
	#add_definitions(/GR-)
	#add_definitions(/D_HAS_EXCEPTIONS=0)
else()
	#add_definitions(-fno-rtti)
	#add_definitions(-fno-exceptions)
	#add_definitions(-fpermissive)
	#add_definitions(-g3)
	add_definitions(-Wall -Wextra -Werror)
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

if(WIN32)
	add_definitions(-DPOCO_OS_FAMILY_WINDOWS)
endif()
add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DPOCO_NO_AUTOMATIC_LIBS -DPOCO_DLL)

INCLUDE_DIRECTORIES(include/)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/verypoco/include)
file(GLOB SOURCE_CODE src/multithread/*.cpp)
source_group( "multithread_src" FILES ${SOURCE_CODE} )
ADD_LIBRARY(fast-event-system SHARED src/fast-event-system/fes.cpp src/scheduler/sas.cpp ${SOURCE_CODE})
IF(WIN32)
	TARGET_LINK_LIBRARIES(fast-event-system "${CMAKE_CURRENT_SOURCE_DIR}/build/Debug/verypoco.lib")
ELSE()
	TARGET_LINK_LIBRARIES(fast-event-system verypoco stdc++)
ENDIF()

file(GLOB HEADER_CODE include/scheduler/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/scheduler")
source_group( "scheduler" FILES ${HEADER_CODE} )

file(GLOB HEADER_CODE include/fast-event-system/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/fast-event-system")
source_group( "fast-event-system" FILES ${HEADER_CODE} )
            
file(GLOB HEADER_CODE include/concurrentqueue/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/concurrentqueue")
source_group( "concurrentqueue" FILES ${HEADER_CODE} )
            
file(GLOB HEADER_CODE include/concurrentqueue/internal/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/concurrentqueue/internal")
source_group( "concurrentqueue_internal" FILES ${HEADER_CODE} )
            
file(GLOB HEADER_CODE include/animator/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/fast-event-system/animator")
source_group( "fast-event-system_animator" FILES ${HEADER_CODE} )
	    
file(GLOB HEADER_CODE include/multithread/*.h)
install(    FILES ${HEADER_CODE}
            DESTINATION "include/multithread")
source_group( "multithread" FILES ${HEADER_CODE} )

macro(CREATE_TEST TESTNAME TESTDEPENDS)
	ADD_EXECUTABLE(${TESTNAME} tests/${TESTNAME}.cpp)
	target_link_libraries(${TESTNAME} ${TESTDEPENDS})
	ADD_TEST(NAME ${TESTNAME} COMMAND ${TESTNAME} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

CREATE_TEST("test_fes" "fast-event-system")
CREATE_TEST("test_sync" "fast-event-system")
CREATE_TEST("test_async_fast" "fast-event-system")
CREATE_TEST("test_async_fast_autoconnect" "fast-event-system")
CREATE_TEST("test_async_delay" "fast-event-system")
CREATE_TEST("test_leader_follower" "fast-event-system")
CREATE_TEST("test_bench" "fast-event-system")

