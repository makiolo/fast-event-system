cmake_minimum_required(VERSION 2.8)

cmaki2_gtest(test_fes test_fes.cpp DEPENDS fes)
cmaki2_gtest(test_sync test_sync.cpp DEPENDS fes)
cmaki2_gtest(test_async_fast test_async_fast.cpp DEPENDS fes)
cmaki2_gtest(test_async_fast_autoconnect test_async_fast_autoconnect.cpp DEPENDS fes)
cmaki2_gtest(test_async_delay test_async_delay.cpp DEPENDS fes)

