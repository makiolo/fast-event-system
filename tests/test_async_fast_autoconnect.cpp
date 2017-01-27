#include <iostream>
#include <gtest/gtest.h>
#include "../async_fast.h"

class AsyncFastAutoConnectTest : testing::Test { };

TEST(AsyncFastAutoConnectTest, Test1)
{
	const int N = 9;
	int counter = 0;

	fes::async_fast<int> sync;
	sync.connect(sync);
	sync.connect([&counter](auto&) { ++counter; });

	sync(0);
	for (int i = 0; i < N; ++i)
	{
		sync.get();
	}

	ASSERT_EQ(counter, N);
}
