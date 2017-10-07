cmaki_google_test(test_fes test_fes.cpp DEPENDS fes)
cmaki_google_test(test_sync test_sync.cpp DEPENDS fes)
cmaki_google_test(test_async_fast test_async_fast.cpp DEPENDS fes)
cmaki_google_test(test_async_fast_autoconnect test_async_fast_autoconnect.cpp DEPENDS fes)
cmaki_google_test(test_async_delay test_async_delay.cpp DEPENDS fes)

