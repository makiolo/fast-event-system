#include <iostream>
#include <tuple>
#include <functional>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <assert.h>
#include <cstdlib>
#include <gtest/gtest.h>
#include "../sync.h"
#include "../async_delay.h"
#include "../async_fast.h"

class FesTest : testing::Test { };

template <typename TYPE = fes::sync<std::string>>
class Producer
{
public:
	Producer() {}
	~Producer() {}

	template <typename Data>
	void operator()(Data&& data) { _channel(std::forward<Data>(data)); }

	template <typename Data>
	void operator()(int priority, fes::deltatime delay, Data&& data)
	{
		_channel(priority, delay, std::forward<Data>(data));
	}

	void update() { _channel.update(); }

	TYPE& get_channel() { return _channel; }

protected:
	TYPE _channel;
};

template <typename TYPE = fes::sync<std::string>>
class Consumer
{
public:
	Consumer() : _data("waiting")
	{
	}

	~Consumer() {}

	void connect(Producer<TYPE>& producer)
	{
		_conn = producer.get_channel().connect(this, &Consumer::on_handler);
	}

	const std::string& get_data() const { return _data; }

protected:
	template <typename Data>
	void on_handler(Data&& data) { _data = std::forward<Data>(data); }

protected:
	std::string _data;
	fes::connection<std::string> _conn;
};

TEST(FesTest, Test1)
{
	Producer<fes::sync<std::string>> p;
	Consumer<fes::sync<std::string>> c1;
	Consumer<fes::sync<std::string>> c2;
	{
		c1.connect(p);
		c2.connect(p);
		p("data");
		ASSERT_STREQ(c1.get_data().c_str(), "data");
		ASSERT_STREQ(c2.get_data().c_str(), "data");
	}
}

TEST(FesTest, Test2)
{
	Producer<fes::async_fast<std::string>> p;
	Consumer<fes::async_fast<std::string>> c1;
	Consumer<fes::async_fast<std::string>> c2;
	{
		c1.connect(p);
		c2.connect(p);
		p("data");
		p.update();
		ASSERT_STREQ(c1.get_data().c_str(), "data");
		ASSERT_STREQ(c2.get_data().c_str(), "data");
	}
}

TEST(FesTest, Test3)
{
	Producer<fes::async_delay<std::string>> p;
	Consumer<fes::async_delay<std::string>> c1;
	Consumer<fes::async_delay<std::string>> c2;
	{
		c1.connect(p);
		c2.connect(p);
		p(0, fes::deltatime(0), "data");
		p.update();
		ASSERT_STREQ(c1.get_data().c_str(), "data");
		ASSERT_STREQ(c2.get_data().c_str(), "data");
	}
}

TEST(FesTest, Test4)
{
	fes::async_delay<int> root;
	fes::async_delay<int> node_a;
	fes::async_delay<int> node_b;

	auto c1 = root.connect(0, fes::deltatime(100), node_a);
	auto c2 = root.connect(0, fes::deltatime(200), node_b);

	static bool called1 = false;
	auto c3 = node_a.connect([&](auto&& data)
		{
			std::cout << "A: data = " << data << std::endl;
			called1 = true;
			ASSERT_EQ(data, 111);
		});

	static bool called2 = false;
	auto c4 = node_b.connect([&](auto&& data)
		{
			std::cout << "B: data = " << data << std::endl;
			called2 = true;
			ASSERT_EQ(data, 111);
		});

	root(0, fes::deltatime(10), 111);

	auto t1 = fes::high_resolution_clock() + fes::deltatime(600);
	while (fes::high_resolution_clock() < t1)
	{
		root.update();
		node_a.update();
		node_b.update();
	}
	ASSERT_TRUE(called1);
	ASSERT_TRUE(called2);
}
