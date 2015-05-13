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

	template <typename R, typename P>
	void operator()(int priority, std::chrono::duration<R,P> delay, const std::string&& data)
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
			p(0, std::chrono::milliseconds(0), "data");
			p.update();
			assert(c1.get_data() == "data");
			assert(c2.get_data() == "data");
		}
	}
	
	fes::async_fast<int> root;
	fes::sync<int> node_a;
	fes::sync<int> node_b;
	
	root.connect(node_a);
	root.connect(node_b);

	node_a.connect([](int data) {
		std::cout << "A: data = " << data << std::endl;
		assert(data == 111);
	});
	node_b.connect([](int data) {
		std::cout << "B: data = " << data << std::endl;
		assert(data == 111);
	});

	root(111);
	root.update();
	
	return 0;
}

