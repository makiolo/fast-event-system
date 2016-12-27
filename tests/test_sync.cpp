#include <iostream>
#include <gtest/gtest.h>
#include "../sync.h"

class SyncTest : testing::Test { };

TEST(SyncTest, Test1)
{
	// fes::safe::sync
	// fes::safe::async
	// fes::unsafe::sync
	// fes::unsafe::async

	fes::sync<int, std::string, double> sync;

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
	}
	// kill only if autodisconnection failed
	sync(6, "kill", 12.0);
}

void hello(int n1, int n2)
{
    std::cout << "hi: " << n1 << ", " << n2 << std::endl;
}

TEST(SyncTest, Test2)
{
    fes::method<int, int> m(hello);
    m(1, 2);
}

struct foo
{
	void hello(const int& n1, const int& n2)
	{
	    std::cout << "hi, from foo: " << n1 << ", " << n2 << std::endl;
	}
};

TEST(SyncTest, Test3)
{
	foo f;
	fes::method<int, int> m(&f, &foo::hello);
	m(1, 2);
}
