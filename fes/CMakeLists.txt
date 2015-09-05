cmake_minimum_required(VERSION 2.8)

# find_package
include_directories(../verypoco/include)
if(WIN32)
	add_definitions(-DPOCO_OS_FAMILY_WINDOWS)
endif()
add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DPOCO_NO_AUTOMATIC_LIBS -DPOCO_DLL)

IF(WIN32)
	GENERATE_LIB(fes DEPENDENCIES verypoco multithread)
ELSE()
	GENERATE_LIB(fes DEPENDENCIES verypoco multithread stdc++)
ENDIF()

