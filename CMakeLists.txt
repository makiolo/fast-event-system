PROJECT(fast-event-system CXX)
cmake_minimum_required(VERSION 3.5)
include("$ENV{CMAKI_PWD}/node_modules/npm-mas-mas/cmaki/cmaki.cmake")
cmaki_setup()
# cmaki_git_clone(
# 	PROJECT_NAME    concurrentqueue
# 	GIT_URL         https://github.com/cameron314/concurrentqueue
# 	GIT_BRANCH      master
# 	DIRECTORY		${CMAKE_SOURCE_DIR}
# )
cmaki_find_package_boost()
include_directories(.)
include_directories(include)
cmaki_library(fes src/clock.cpp PTHREADS)
cmaki_google_test(test_fes tests/test_fes.cpp DEPENDS fes)
cmaki_google_test(test_sync tests/test_sync.cpp DEPENDS fes)
cmaki_google_test(test_async_fast tests/test_async_fast.cpp DEPENDS fes)
cmaki_google_test(test_async_fast_autoconnect tests/test_async_fast_autoconnect.cpp DEPENDS fes)
cmaki_google_test(test_async_delay tests/test_async_delay.cpp DEPENDS fes)

