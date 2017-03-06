#include <iostream>
#include <gtest/gtest.h>
#include "../sync.h"

using testing::AtLeast;
using testing::AnyNumber;
using testing::_;

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
	void hello(const int& n1, const int& n2)
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
	MOCK_METHOD0(mock_constructor, void());
	MOCK_METHOD0(mock_destructor, void());
	MOCK_METHOD0(mock_copy, void());
	MOCK_METHOD0(mock_move, void());
	MOCK_METHOD0(mock_swap, void());
	
	foo()
		: _str("bar")
	{
		mock_constructor();
	}

	foo(const foo& other)
		: _str(other._str)
	{
		mock_copy();
		std::cout << "constructor copy foo" << std::endl;
	}

	foo(foo&& other) noexcept
		: _str(std::move(other._str))
	{
		mock_move();
		std::cout << "constructor move foo" << std::endl;
	}

	virtual ~foo()
	{
		mock_destructor();
	}
	
	void swap(foo& other) noexcept
	{
		mock_swap();
		std::cout << "swap foo" << std::endl;
		using std::swap;
		std::swap(_str, other._str);
	}

	foo& operator=(const foo& other)
	{
		mock_copy();
		std::cout << "operator copy foo" << std::endl;
		foo(other).swap(*this);
		return *this;
	}

	foo& operator=(foo&& other) noexcept
	{
		mock_move();
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
	// 
	EXPECT_CALL(f, mock_constructor()).Times(AnyNumber());
	EXPECT_CALL(f, mock_destructor()).Times(AnyNumber());
	EXPECT_CALL(f, mock_copy()).Times(0);
	EXPECT_CALL(f, mock_move()).Times(AnyNumber());
	EXPECT_CALL(f, mock_swap()).Times(AnyNumber());
}

TEST(SyncTest, test_sync_chaining)
{
	fes::sync<foo> a;
	fes::sync<foo> b;
	fes::sync<foo> c;
	a.connect(b);
	b.connect(c);
	c.connect([](auto&&)
		{
			std::cout << "received foo like pipeline" << std::endl;
		});
	// send from a
	a( foo() );
}
