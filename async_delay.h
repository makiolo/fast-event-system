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

	~async_delay()
	{
		;
	}

	async_delay(const async_delay&) = delete;
	async_delay& operator=(const async_delay&) = delete;

	template <typename DATA>
	void operator()(int priority, deltatime delay, DATA&&... data)
	{
		marktime delay_point = high_resolution_clock() + delay;
		_queue.emplace_back(priority, delay_point, std::forward<DATA>(data)...);
		std::sort(std::begin(_queue), std::end(_queue), message_comp<DATA...>());
		_sem.notify();
	}

	template <typename DATA>
	void operator()(deltatime delay, DATA&&... data)
	{
		operator()(0, delay, std::forward<DATA>(data)...);
	}

	template <typename DATA>
	void operator()(int priority, DATA&&... data)
	{
		operator()(priority, fes::deltatime(0), std::forward<DATA>(data)...);
	}

	template <typename DATA>
	inline void operator()(DATA&&... data)
	{
		operator()(0, fes::deltatime(0), std::forward<DATA>(data)...);
	}

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

	inline std::tuple<Args...> get()
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

	template <typename T>
	inline weak_connection<Args...> connect(T* obj, void (T::*ptr_func)(Args&&...))
	{
		return _output.connect(obj, ptr_func);
	}

	template <typename METHOD>
	inline weak_connection<Args...> connect(METHOD&& method)
	{
		return _output.connect(std::forward<METHOD>(method));
	}

	inline weak_connection<Args...> connect(sync<Args...>& callback)
	{
		return _output.connect([&callback](Args&&... data)
			{
				callback(std::forward<Args>(data)...);
			});
	}

	inline weak_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return _output.connect([&queue](Args&&... data)
			{
				queue(std::forward<Args>(data)...);
			});
	}

	inline weak_connection<Args...> connect(
		int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return _output.connect([&queue, priority, delay](Args&&... data)
			{
				queue(priority, delay, std::forward<Args>(data)...);
			});
	}

protected:	
	template <typename Tuple, int... S>
	inline void get(Tuple&& top, seq<S...>) const
	{
		_output(std::get<S>(std::forward<Tuple>(top))...);
	}

	std::tuple<Args...> _get()
	{
		auto& t = _queue.back();
		_sem.wait();
		while(high_resolution_clock() < t._timestamp)
		{
#ifndef _WIN32
			// each 100 ms
			usleep(100);
#endif
		}
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
