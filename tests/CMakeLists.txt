cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(Boost REQUIRED)
IF(Boost_FOUND)
	include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
	CREATE_TEST("test_boost" fes ${Boost_LIBRARIES})
endif()

CREATE_TEST("test_fes" fes)
CREATE_TEST("test_sync" fes)
CREATE_TEST("test_async_fast" fes)
CREATE_TEST("test_async_fast_autoconnect" fes)
CREATE_TEST("test_async_delay" fes)
CREATE_TEST("test_bench" fes)
CREATE_TEST("test_nan" fes)
CREATE_TEST("test_sequence" fes)
CREATE_TEST("test_parallel" fes)
CREATE_TEST("test_parallel_sequence" fes)

# find_package
include_directories(SYSTEM ../verypoco/include)
if(WIN32)
	add_definitions(-DPOCO_OS_FAMILY_WINDOWS)
endif()
add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DPOCO_NO_AUTOMATIC_LIBS -DPOCO_DLL)
CREATE_TEST("test_leader_follower" fes verypoco)

