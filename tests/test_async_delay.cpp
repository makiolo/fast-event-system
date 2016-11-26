#include <iostream>
#include <gtest/gtest.h>
#include "../async_delay.h"

class DISABLED_AsyncDelayTest : testing::Test { };

TEST(DISABLED_AsyncDelayTest, Test1)
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
		sync(5, "hello world", 11.0);
		sync.update();
	}
	// kill only if autodisconnection failed
	// TODO: exception here
	sync(6, "kill", 12.0);
	sync.update();

	ASSERT_TRUE(is_dispatched);
}
