#include <iostream>
#include <gmock/gmock.h>
#include "../async_fast.h"

using testing::AtLeast;
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
	MOCK_METHOD1(mock_copy, void(const foo& other));
	MOCK_METHOD1(mock_move, void(foo&& other));
	MOCK_METHOD1(mock_swap, void(foo& other));
	
	foo()
		: _str("bar")
	{
		mock_constructor();
	}

	foo(const foo& other)
		: _str(other._str)
	{
		mock_copy(other);
		std::cout << "constructor copy foo" << std::endl;
	}

	foo(foo&& other) noexcept
		: _str(std::move(other._str))
	{
		mock_move(other);
		std::cout << "constructor move foo" << std::endl;
	}

	virtual ~foo()
	{
		mock_destructor();
	}
	
	void swap(foo& other) noexcept
	{
		mock_swap(other);
		std::cout << "swap foo" << std::endl;
		using std::swap;
		std::swap(_str, other._str);
	}

	foo& operator=(const foo& other)
	{
		mock_copy(other);
		std::cout << "operator copy foo" << std::endl;
		foo(other).swap(*this);
		return *this;
	}

	foo& operator=(foo&& other) noexcept
	{
		mock_move(other);
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
	EXPECT_CALL(f, constructor()).Times(AtLeast(1));
	EXPECT_CALL(f, destructor()).Times(AtLeast(1));
	EXPECT_CALL(f, copy(_)).Times(0);
	EXPECT_CALL(f, move(_)).Times(AtLeast(1));
}
