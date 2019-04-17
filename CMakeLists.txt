PROJECT(fast-event-system CXX)
cmake_minimum_required(VERSION 3.5)

# setup
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# cmaki
include("$ENV{CMAKI_PWD}/node_modules/npm-mas-mas/cmaki/cmaki.cmake")

# Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Boost
if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(Boost_DEBUG 1)
else()
	set(Boost_DEBUG 0)
endif()
find_package(Boost COMPONENTS system atomic chrono thread coroutine filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
install(DIRECTORY ${CONAN_LIB_DIRS_BOOST}/ DESTINATION ${CMAKE_BUILD_TYPE})

# Concurrentqueue
include_directories(concurrentqueue)

# Other packages (gtest, gmock)
include_directories(${CONAN_INCLUDE_DIRS})

cmaki_library(fes clock.cpp PTHREADS DEPENDS ${Boost_LIBRARIES})

cmaki_google_test(test_fes tests/test_fes.cpp DEPENDS fes ${CONAN_LIBS})
cmaki_google_test(test_sync tests/test_sync.cpp DEPENDS fes ${CONAN_LIBS})
cmaki_google_test(test_async_fast tests/test_async_fast.cpp DEPENDS fes ${CONAN_LIBS})
cmaki_google_test(test_async_fast_autoconnect tests/test_async_fast_autoconnect.cpp DEPENDS fes ${CONAN_LIBS})
cmaki_google_test(test_async_delay tests/test_async_delay.cpp DEPENDS fes ${CONAN_LIBS})
