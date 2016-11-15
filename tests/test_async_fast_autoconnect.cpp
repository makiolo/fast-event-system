#include <iostream>
#include "../async_fast.h"
#include <gtest/gtest.h>

class AsyncFastAutoConnectTest : testing::Test { };

TEST(AsyncFastAutoConnectTest, Test1)
{
	const int N = 9;
	int counter = 0;

	fes::async_fast<void> sync;
	sync.connect(sync);
	sync.connect([&counter]() { ++counter; });

	sync();
	for (int i = 0; i < N; ++i)
	{
		sync.get();
	}

	ASSERT_EQ(counter, N);
}
