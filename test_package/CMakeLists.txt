PROJECT(fast-event-system CXX)
cmake_minimum_required(VERSION 3.5)
include("$ENV{CMAKI_PWD}/node_modules/npm-mas-mas/cmaki/cmaki.cmake")
cmaki_setup()
cmaki_find_package_boost()

# include_directories(${CONAN_INCLUDE_DIRS_FAST-EVENT-SYSTEM})

cmaki_google_test(test_fes test_fes.cpp DEPENDS fes)
# cmaki_google_test(test_sync test_sync.cpp DEPENDS fes)
# cmaki_google_test(test_async_fast test_async_fast.cpp DEPENDS fes)
# cmaki_google_test(test_async_fast_autoconnect test_async_fast_autoconnect.cpp DEPENDS fes)
# cmaki_google_test(test_async_delay test_async_delay.cpp DEPENDS fes)

