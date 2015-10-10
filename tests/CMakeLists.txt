cmake_minimum_required(VERSION 2.8)

CREATE_TEST(test_fes "fes")
CREATE_TEST(test_sync "fes")
CREATE_TEST(test_async_fast "fes")
CREATE_TEST(test_async_fast_autoconnect "fes")
CREATE_TEST(test_async_delay "fes")
CREATE_TEST(test_nan "fes")
IF(NOT WIN32)
	CREATE_TEST(test_annotations "fes")
ENDIF()


