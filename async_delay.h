#ifndef _ASYNC_DELAY_H_
#define _ASYNC_DELAY_H_

#include <vector>
#include <algorithm>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "message.h"
#include "connection.h"
#include "sem.h"
#include "sync.h"

namespace fes {

template <typename... Args>
class async_fast;

template <typename... Args>
class async_delay
{
public:
	using container_type = std::vector<message<Args...>>;

	explicit async_delay()
        	: _output()
        	, _queue()
	{
		;
	}

	template <typename ... PARMS>
	void operator()(int priority, deltatime delay, PARMS&&... data)
	{
		marktime delay_point = high_resolution_clock() + delay;
		_queue.emplace_back(priority, delay_point, std::forward<PARMS>(data)...);
		std::sort(std::begin(_queue), std::end(_queue), message_comp<Args...>());
		_sem.notify();
	}

	/*
	template <typename ... PARMS>
	void operator()(deltatime delay, PARMS&&... data)
	{
		operator()(0, delay, data...);
	}

	template <typename ... PARMS>
	void operator()(int priority, PARMS&&... data)
	{
		operator()(priority, fes::deltatime(0), data...);
	}

	template <typename ... PARMS>
	inline void operator()(PARMS&&... data)
	{
		operator()(0, fes::deltatime(0), data...);
	}
	*/

	void update()
	{
		if(!empty())
			get();
	}

	void fortime(deltatime time = fes::deltatime(16))
	{
		auto mark = fes::high_resolution_clock() + time;
		while (fes::high_resolution_clock() <= mark)
		{
			update();
		}
	}

	inline auto get()
	{
		return _get();
	}

	inline bool empty() const
	{
		return (_sem.size() <= 0);
	}

	inline size_t size() const
	{
		return _sem.size();
	}

	template <typename T, typename ... ARGS>
	inline weak_connection<Args...> connect(T* obj, void (T::*ptr_func)(const ARGS&...))
	{
		return _output.connect(obj, ptr_func);
	}

	template <typename METHOD>
	inline weak_connection<Args...> connect(METHOD&& method)
	{
		return _output.connect(std::forward<METHOD>(method));
	}

	template <typename ... ARGS>
	inline weak_connection<Args...> connect(sync<Args...>& callback)
	{
		return _output.connect([&callback](ARGS&&... data)
			{
				callback(std::forward<ARGS>(data)...);
			});
	}

	template <typename ... ARGS>
	inline weak_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return _output.connect([&queue](ARGS&&... data)
			{
				queue(std::forward<ARGS>(data)...);
			});
	}

	template <typename ... ARGS>
	inline weak_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return _output.connect([&queue, priority, delay](ARGS&&... data)
			{
				queue(priority, delay, std::forward<ARGS>(data)...);
			});
	}

protected:	
	template <typename Tuple, int... S>
	inline void get(Tuple&& top, seq<S...>) const
	{
		_output(std::get<S>(std::forward<Tuple>(top))...);
	}

	auto _get()
	{
		_sem.wait();
		/*
		// bugged ?
#if 0
		while(high_resolution_clock() < _queue.back()._timestamp)
		{
#ifndef _WIN32
			// each 100 ms
			usleep(100);
#endif
		}
#endif
		*/
		auto& t = _queue.back();
		get(std::forward<std::tuple<Args...> >(t._data), gens<sizeof...(Args)>{});
		_queue.pop_back();
		return t._data;
	}

protected:
	sync<Args...> _output;
	container_type _queue;
	fes::semaphore _sem;
};

}

#endif
