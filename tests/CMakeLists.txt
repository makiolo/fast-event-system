cmake_minimum_required(VERSION 2.8)

CREATE_TEST("test_fes" fes)
CREATE_TEST("test_sync" fes)
CREATE_TEST("test_async_fast" fes)
CREATE_TEST("test_async_fast_autoconnect" fes)
CREATE_TEST("test_async_delay" fes)
CREATE_TEST("test_bench" fes)
CREATE_TEST("test_nan" fes)

# find_package
include_directories(../verypoco/include)
if(WIN32)
	add_definitions(-DPOCO_OS_FAMILY_WINDOWS)
endif()
add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DPOCO_NO_AUTOMATIC_LIBS -DPOCO_DLL)
CREATE_TEST("test_leader_follower" fes verypoco)

