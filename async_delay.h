#ifndef _ASYNC_DELAY_H_
#define _ASYNC_DELAY_H_

#include <vector>
#include <algorithm>
#include <message.h>
#include <connection.h>
#include <sync.h>

namespace fes {

template <typename... Args>
class async_fast;

template <typename... Args>
class async_delay
{
public:
	using container_type = std::vector<message<Args...>>;

	async_delay()
        : _output()
        , _queue()
    {

    }

	~async_delay()
	{
		;
	}

	async_delay(const async_delay&) = delete;
	async_delay& operator=(const async_delay&) = delete;

	void operator()(int priority, deltatime delay, const Args&... data)
	{
		marktime delay_point = high_resolution_clock() + delay;
		_queue.emplace_back(priority, delay_point, data...);
		std::sort(std::begin(_queue), std::end(_queue), message_comp<Args...>());
	}

	void operator()(deltatime delay, const Args&... data)
	{
		operator()(0, delay, data...);
	}

	void operator()(int priority, const Args&... data)
	{
		operator()(priority, fes::deltatime(0), data...);
	}

	inline void operator()(const Args&... data) { operator()(0, fes::deltatime(0), data...); }

	void update(deltatime tmax = fes::deltatime(16))
	{
		marktime timeout = high_resolution_clock() + tmax;
		bool has_next = true;
		while (!empty() && has_next && (high_resolution_clock() <= timeout))
		{
			has_next = _dispatch_one();
		}
	}

	void update_while(deltatime time)
	{
		auto mark = fes::high_resolution_clock() + time;
		while (fes::high_resolution_clock() <= mark)
		{
			dispatch_one();
		}
	}

	bool dispatch_one()
	{
		if (!empty())
		{
			return _dispatch_one();
		}
		return false;
	}

	inline bool empty() const { return _queue.empty(); }

	inline size_t size() const { return _queue.size(); }

	template <typename T>
	inline weak_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _output.connect(obj, ptr_func);
	}

	inline weak_connection<Args...> connect(const typename method<Args...>::function& method)
	{
		return _output.connect(method);
	}

	inline weak_connection<Args...> connect(sync<Args...>& callback)
	{
		return _output.connect([&callback](const Args&... data)
			{
				callback(data...);
			});
	}

	inline weak_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return _output.connect([&queue](const Args&... data)
			{
				queue(data...);
			});
	}

	inline weak_connection<Args...> connect(
		int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return _output.connect([&queue, priority, delay](const Args&... data)
			{
				queue(priority, delay, data...);
			});
	}

protected:
	template <int... S>
	inline void dispatch_one(const std::tuple<Args...>& top, seq<S...>) const
	{
		_output(std::get<S>(top)...);
	}

	bool _dispatch_one()
	{
		auto& t = _queue.back();
		if (high_resolution_clock() >= t._timestamp)
		{
			dispatch_one(t._data, gens<sizeof...(Args)>{});
			_queue.pop_back();
			return true;
		}
		return false;
	}

protected:
	sync<Args...> _output;
	container_type _queue;
};

}

#endif

