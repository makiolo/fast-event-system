#include <iostream>
#include "../async_fast.h"
#include <gtest/gtest.h>

class AsyncFastTest : testing::Test { };

TEST(AsyncFastTest, Test1)
{
	fes::async_fast<int, std::string, double> sync;

	// test connect in context
	{
		fes::connection<int, std::string, double> conn(
			sync.connect([](int n, const std::string& str, double r)
				{
					std::cout << "received message: " << std::endl;
					std::cout << "n = " << n << std::endl;
					std::cout << "str = " << str << std::endl;
					std::cout << "r = " << r << std::endl;
					ASSERT_STRNE(str.c_str(), "kill");
				}));
		// lambda must received this
		sync(5, "hello world", 11.0);
		sync.update();
	}
	// kill only if autodisconnection failed
	sync(6, "kill", 12.0);
	sync.update();
}

TEST(AsyncFastTest, Test_fibonacci_n4134)
{
	/*
	// n4134: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4134.pdf
	generator<int> fib(int n)
	{
		int a = 0;
		int b = 1;
		while (n-- > 0)
		{
			yield a;
			auto next = a + b;
			a = b;
			b = next;
		}
	}
	*/
	
	auto fib = [](int n) {
		return fes::make_generator<int>([n](auto& yield) mutable {
			int a = 0;
			int b = 1;
			while (n-- > 0)
			{
				yield(a);
				auto next = a + b;
				a = b;
				b = next;
			}
		});
	};
	
	for (auto& v : *fib(35).get())
	{
		std::cout << v << std::endl;
		if (v > 10)
			break;
	}
}
