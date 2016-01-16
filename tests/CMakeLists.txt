cmake_minimum_required(VERSION 2.8)

cmaki_test(fes test_fes.cpp DEPENDS fes)
cmaki_test(sync test_sync.cpp DEPENDS fes)
cmaki_test(async_fast test_async_fast.cpp DEPENDS fes)
cmaki_test(async_fast_autoconnect test_async_fast_autoconnect.cpp DEPENDS fes)
cmaki_test(async_delay test_async_delay.cpp DEPENDS fes)
if(NOT WIN32)
	cmaki_test(annotations test_annotations.cpp DEPENDS fes)
endif()

