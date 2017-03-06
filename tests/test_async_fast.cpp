#include <iostream>
#include <gmock/gmock.h>
#include "../async_fast.h"

using testing::AtLeast;
using testing::AnyNumber;
using testing::_;

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

TEST(AsyncFastTest, Test2)
{
	fes::async_fast<foo> sync;
	sync.connect([](auto&& f)
		{
			std::cout << "<async_fast> received rvalue" << std::endl;
		});
	sync(foo());
	sync.update();
}

TEST(AsyncFastTest, Test3)
{
	fes::async_fast<foo> sync;
	sync.connect([](auto&& f)
		{
			std::cout << "<async_fast> received lvalue" << std::endl;
		});
	foo f;
	ASSERT_STREQ(f._str.c_str(), "bar");
	sync(f);
	ASSERT_STREQ(f._str.c_str(), "");
	sync.update();
	ASSERT_STREQ(f._str.c_str(), "");
	// 
	EXPECT_CALL(f, mock_constructor()).Times(AnyNumber());
	EXPECT_CALL(f, mock_destructor()).Times(AnyNumber());
	EXPECT_CALL(f, mock_copy()).Times(0);
	EXPECT_CALL(f, mock_move()).Times(AnyNumber());
	EXPECT_CALL(f, mock_swap()).Times(AnyNumber());
}
