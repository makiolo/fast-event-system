PROJECT(FAST-EVENT-SYSTEM CXX)
cmake_minimum_required(VERSION 3.0)

include(cmaki)

cmaki_find_package(boost-headers)
cmaki_find_package(boost-system)
cmaki_find_package(boost-context)
cmaki_find_package(boost-atomic)
cmaki_find_package(boost-chrono)
cmaki_find_package(boost-thread)
cmaki_find_package(boost-coroutine2)
cmaki_find_package(boost-filesystem)

include_directories(concurrentqueue)

cmaki_library(fes clock.cpp PTHREADS)

cmaki_google_test(test_fes tests/test_fes.cpp DEPENDS fes)
cmaki_google_test(test_sync tests/test_sync.cpp DEPENDS fes)
cmaki_google_test(test_async_fast tests/test_async_fast.cpp DEPENDS fes)
cmaki_google_test(test_async_fast_autoconnect tests/test_async_fast_autoconnect.cpp DEPENDS fes)
cmaki_google_test(test_async_delay tests/test_async_delay.cpp DEPENDS fes)

