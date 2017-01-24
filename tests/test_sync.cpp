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

struct foofoo
{
	template <typename PARM>
	void hello(PARM&& n1, PARM&& n2)
	{
	    std::cout << "hi, from foofoo: " << n1 << ", " << n2 << std::endl;
	}
};

TEST(SyncTest, Test3)
{
	foofoo f;
	fes::method<int, int> m(&f, &foofoo::hello);
	m(1, 2);
}

struct foo
{	
	foo()
		: _str("bar")
	{
	}
	
	foo(const foo& other)
		: _str(other._str)
	{
		std::cout << "constructor copy foo" << std::endl;
	}

	foo(foo&& other) noexcept
		: _str(std::move(other._str))
	{
		std::cout << "constructor move foo" << std::endl;
	}

	~foo() {}
	
	void swap(foo& other) noexcept
	{
		std::cout << "swap foo" << std::endl;
		using std::swap;
		std::swap(_str, other._str);
	}

	foo& operator=(const foo& other)
	{
		std::cout << "operator copy foo" << std::endl;
		foo(other).swap(*this);
		return *this;
	}

	foo& operator=(foo&& other) noexcept
	{
		std::cout << "operator move foo" << std::endl;
		foo(std::move(other)).swap(*this);
		return *this;
	}
public:
	std::string _str;
};

TEST(SyncTest, Test4)
{
	fes::sync<foo> sync;
	sync.connect([](auto&& f)
		{
			std::cout << "received rvalue" << std::endl;
		});
	sync(foo());
}

TEST(SyncTest, Test5)
{
	fes::sync<foo> sync;
	sync.connect([](auto&& f)
		{
			std::cout << "received lvalue" << std::endl;
		});
	foo f;
	sync(f);
}
