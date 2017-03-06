#include <iostream>
#include <gmock/gmock.h>
#include "../async_delay.h"

using testing::AtLeast;
using testing::_;

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
		sync(0, fes::deltatime(0), 0, "hello world", 11.0);
		sync.update();
	}
	// kill only if autodisconnection failed
	// TODO: exception here
	sync(0, fes::deltatime(0), 6, "kill", 12.0);
	sync.update();

	ASSERT_TRUE(is_dispatched);
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
		mock_move(std::move(other));
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
		mock_move(std::move(other));
		std::cout << "operator move foo" << std::endl;
		foo(std::move(other)).swap(*this);
		return *this;
	}
public:
	std::string _str;
};

TEST(AsyncDelayTest, Test2)
{
	fes::async_delay<foo> sync;
	sync.connect([](auto&& f)
		{
			std::cout << "<async_delay> received rvalue" << std::endl;
		});
	sync(0, fes::deltatime(0), foo());
	sync.update();
}

TEST(AsyncDelayTest, Test3)
{
	fes::async_delay<foo> sync;
	sync.connect([](auto&& f)
		{
			std::cout << "<async_delay> received lvalue" << std::endl;
		});
	foo f;
	sync(0, fes::deltatime(0), f);
	sync.update();
	// 
	EXPECT_CALL(f, constructor()).Times(AtLeast(1));
	EXPECT_CALL(f, destructor()).Times(AtLeast(1));
	EXPECT_CALL(f, copy(_)).Times(0);
	EXPECT_CALL(f, move(_)).Times(AtLeast(1));
}
