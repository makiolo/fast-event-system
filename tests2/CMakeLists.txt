cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(Boost COMPONENTS graph coroutine context filesystem system regex REQUIRED)
IF(Boost_FOUND)
	include_directories(SYSTEM ${Boost_INCLUDE_DIERS})
	CREATE_TEST(prob24 "")
	CREATE_TEST(coro "")
	target_link_libraries(prob24 ${Boost_LIBRARIES})
	target_link_libraries(coro ${Boost_LIBRARIES})
endif()

