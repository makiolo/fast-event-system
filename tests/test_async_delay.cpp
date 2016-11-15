#include <iostream>
#include "../async_delay.h"
#include <gtest/gtest.h>

class AsyncDelayTest : testing::Test { };

TEST(AsyncDelayTest, Test1)
{
	fes::async_delay<int, std::string, double> sync;
	bool is_dispatched = false;

	// test connect in context
	{
		fes::connection<int, std::string, double> conn(
			sync.connect([&is_dispatched](int n, const std::string& str, double r)
				{
					std::cout << "received message: " << std::endl;
					std::cout << "n = " << n << std::endl;
					std::cout << "str = " << str << std::endl;
					std::cout << "r = " << r << std::endl;
					ASSERT_STRNE(str.c_str(), "kill");
					if (str == "hello world")
					{
						is_dispatched = true;
					}
				}));
		// lambda must received this
		sync(fes::deltatime(2000), 5, "hello world", 11.0);
		sync.fortime(fes::deltatime(2500));
	}
	// kill only if autodisconnection failed
	sync(6, "kill", 12.0);
	sync.update();

	ASSERT_TRUE(is_dispatched);
}
