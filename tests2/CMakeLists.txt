cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(Boost COMPONENTS graph coroutine context filesystem system regex REQUIRED)
IF(Boost_FOUND)
	include_directories(SYSTEM ${Boost_INCLUDE_DIRS} /usr/include)
	CREATE_TEST(coro "fes")
	target_link_libraries(coro ${Boost_LIBRARIES} /usr/lib/x86_64-linux-gnu/libssh.so)
endif()

