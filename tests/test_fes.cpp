// Ricardo Marmolejo Garcia
// 15-04-2015
// testing fes
#include <iostream>
#include <tuple>
#include <functional>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <assert.h>
#include <fast-event-system/fes.h>

template <typename TYPE = fes::sync<std::string> >
class Producer
{
public:
	Producer() {}
	~Producer() {}

	void operator()(const std::string&& data)
	{
		_channel(std::forward<const std::string>(data));
	}

	void operator()(int priority, fes::deltatime delay, const std::string&& data)
	{
		_channel(priority, delay, std::forward<const std::string>(data));
	}

	void update()
	{
		_channel.update();
	}
	
	TYPE& get_channel() {return _channel;}
protected:
	TYPE _channel;
};

template <typename TYPE = fes::sync<std::string> >
class Consumer
{
public:
	Consumer()
		: _data("waiting")
	{
		
	}

	~Consumer()
	{
		
	}

	void connect(Producer<TYPE>& producer)
	{
		_conn = producer.get_channel().connect(this, &Consumer::on_handler);
	}
	
	const std::string& get_data() const {return _data;}

protected:
	void on_handler(const std::string& data)
	{
		_data = data;
	}
	
	std::string _data;
	fes::shared_connection<std::string> _conn;
};

int main()
{
#if 1
	{
		Producer<fes::sync<std::string> > p;
		Consumer<fes::sync<std::string> > c1;
		Consumer<fes::sync<std::string> > c2;
		{
			c1.connect(p);
			c2.connect(p);
			p("data");
			assert(c1.get_data() == "data");
			assert(c2.get_data() == "data");
		}
	}
	{
		Producer<fes::async_fast<std::string> > p;
		Consumer<fes::async_fast<std::string> > c1;
		Consumer<fes::async_fast<std::string> > c2;
		{
			c1.connect(p);
			c2.connect(p);
			p("data");
			p.update();
			assert(c1.get_data() == "data");
			assert(c2.get_data() == "data");
		}
	}
	{
		Producer<fes::async_delay<std::string> > p;
		Consumer<fes::async_delay<std::string> > c1;
		Consumer<fes::async_delay<std::string> > c2;
		{
			c1.connect(p);
			c2.connect(p);
			p(0, 0, "data");
			p.update();
			assert(c1.get_data() == "data");
			assert(c2.get_data() == "data");
		}
	}
#endif
	
	fes::async_delay<int> root;
	fes::async_delay<int> node_a;
	fes::async_delay<int> node_b;
	
	auto c1 = root.connect(0, fes::deltatime(1500), node_a);
	auto c2 = root.connect(0, fes::deltatime(3000), node_b);
	
	static bool called1 = false;
	auto c3 = node_a.connect([](const int& data) {
		std::cout << "A: data = " << data << std::endl;
		called1 = true;
		assert(data == 111);
	});

	static bool called2 = false;
	auto c4 = node_b.connect([](const int& data) {
		std::cout << "B: data = " << data << std::endl;
		called2 = true;
		assert(data == 111);
	});
	
	root(0, fes::deltatime(10), 111);
	
	auto t1 = fes::high_resolution_clock() + fes::deltatime(8000);
	while (true)
	{
		root.update();
		node_a.update();
		node_b.update();

		if (fes::high_resolution_clock() > t1)
		{
			break;
		}
	}

	assert(called1 == true);
	assert(called2 == true);
	
	return 0;
}

