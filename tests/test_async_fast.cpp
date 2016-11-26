#include <iostream>
#include <gtest/gtest.h>
#include "../async_fast.h"

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

	auto fib = [](int n_) {
		return fes::pull_type<int>(
			[&](fes::push_type<int>& yield) {
				int n = n_;
				int a = 0;
				int b = 1;
				while (n-- > 0)
				{
					yield (a);
					auto next = a + b;
					a = b;
					b = next;
				}
			}
		);
	};
	for (auto& v : fib(233))
	{
		std::cout << v << std::endl;
		if (v > 10)
			break;
	}
}

TEST(AsyncFastTest, Test_recursive_n4134)
{
	std::function<fes::pull_type<int>(int,int)> range = [&range](int a_, int b_) -> fes::pull_type<int> {
		return fes::pull_type<int>(
			[&](fes::push_type<int>& yield) {
				int a = a_;
				int b = b_;
				/////////////////////
				auto n = b - a;
				if (n <= 0)
					return;
				if (n == 1)
				{
					yield (a);
					return;
				}

				auto mid = a + n / 2;

				// original proposal is:
				//     yield range(a, mid)
				//     yield range(mid, b)

				for (auto i : range(a, mid))
					yield (i);
				for (auto i : range(mid, b))
					yield (i);
				///////////////////////
			}
		);
	};

	for (auto v : range(1, 10))
		std::cout << v << std::endl;
}

TEST(AsyncFastTest, Test3)
{
	auto fib = [](int n) {
		return fes::push_type<int>(
			[&](fes::pull_type<int>& source) {
				for (auto& s : source)
				{
					std::cout << "<fib(" << n << ")> received: " << s << std::endl;
				}
			}
		);
	};
	auto fib20 = fib(20);
	fib20(1);
	fib20(3);
	fib20(7);
}

using cmd = fes::pipeline<int>;

cmd::link generator()
{
	return [](cmd::in&, cmd::out& yield)
	{
		for (auto& s : {100, 200, 300})
		{
			std::cout << "I am generator and push " << s << std::endl;
			yield(s);
		}
	};
}

cmd::link link1()
{
	return [](cmd::in& source, cmd::out& yield)
	{
		for (auto& s : source)
		{
			std::cout << "I am link1 and push " << s << std::endl;
			yield(s);
		}
	};
}

cmd::link link2()
{
	return [](cmd::in& source, cmd::out& yield)
	{
		for (auto& s : source)
		{
			std::cout << "I am link2 and push " << s << std::endl;
			yield(s);
		}
	};
}

cmd::link link3()
{
	return [](cmd::in& source, cmd::out& yield)
	{
		for (auto& s : source)
		{
			std::cout << "I am link3 and push " << s << std::endl;
			yield(s);
		}
	};
}

cmd::link receiver(fes::push_type<int>& r)
{
	return [&](cmd::in& source, cmd::out& yield)
	{
		for (auto& s : source)
		{
			// send to receiver
			r(s);
			// continue chaining
			yield(s);
		}
	};
}

TEST(AsyncFastTest, goroutines_or_something_like_that)
{
	// pipeline
	cmd(generator(), link1(), link2(), link3());

	// lambda to register
	auto l1 = [](int s) {
		std::cout << "<fib> received: " << s << std::endl;
	};
	auto r = fes::push_type<int>(
		[&](fes::pull_type<int>& source) {
			for (auto& s : source)
			{
				l1(s);
			}
		}
	);

	// channel
	fes::channel<int> go;
	go.connect(link1());
	go.connect(link2());
	go.connect(link3());
	go.connect(receiver(r));
	go(1);
	go(3);
	go(5);
	go(7);
	go(9);
}
