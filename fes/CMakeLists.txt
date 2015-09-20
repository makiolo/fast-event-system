cmake_minimum_required(VERSION 2.8)

# find_package
include_directories(SYSTEM ../verypoco/include)
if(WIN32)
	add_definitions(-DPOCO_OS_FAMILY_WINDOWS)
endif()
add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DPOCO_NO_AUTOMATIC_LIBS -DPOCO_DLL)

# boost
FIND_PACKAGE(Boost REQUIRED)
IF(Boost_FOUND)
	include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
	IF(NOT WIN32)
		set(LIBS_EXTRA stdc++)
	ENDIF()
	GENERATE_LIB(fes DEPENDENCIES ${LIBS_EXTRA} verypoco multithread)
	target_link_libraries(fes ${Boost_LIBRARIES})
endif()

