#ifndef _ASYNC_DELAY_H_
#define _ASYNC_DELAY_H_

#include <vector>
#include <algorithm>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <coroutine/coroutine.h>
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
	void operator()(int priority, marktime delay_point, PARMS&&... data)
	{
		_queue.emplace_back(priority, delay_point, std::forward<Args>(std::forward<PARMS>(data))...);
		if(_queue.size() > 1)
		{
			std::sort(_queue.begin(), _queue.end(), message_comp<Args...>());
		}
		_sem.notify();
	}

	template <typename ... PARMS>
	void operator()(int priority, deltatime delay, PARMS&&... data)
	{
		marktime delay_point = high_resolution_clock() + delay;
		operator()(priority, delay_point, std::forward<PARMS>(data)...);
	}

	// sleep in case of blocking
	void wait(fes::deltatime timeout = fes::deltatime(0))
	{
		if(timeout > fes::deltatime(0))
		{
			auto mark = fes::high_resolution_clock() + timeout;
			while(fes::high_resolution_clock() <= mark)
			{
				if(!empty())
				{
					get();
					break;
				}
			}
		}
		else
		{
			get();
		}
	}
	
	// yield in case of blocking
	void wait(cu::yield_type& yield, fes::deltatime timeout = fes::deltatime(0))
	{
		if(timeout > fes::deltatime(0))
		{
			auto mark = fes::high_resolution_clock() + timeout;
			while(fes::high_resolution_clock() <= mark)
			{
				if(!empty())
				{
					get(yield);
					break;
				}
			}
		}
		else
		{
			get(yield);
		}
	}

	// non-blocking
	void update()
	{
		if(!empty())
			get();
	}

	// sleep in case of blocking
	inline auto get() -> std::tuple<Args...>
	{
		return _get();
	}
	
	// yield in case of blocking
	inline auto get(cu::yield_type& yield) -> std::tuple<Args...>
	{
		return _get(yield);
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
	inline weak_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
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
		return _output.connect([&callback](Args... data)
			{
				callback(std::move(data)...);
			});
	}

	inline weak_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return _output.connect([&queue](Args... data)
			{
				queue(std::move(data)...);
			});
	}

	inline weak_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return _output.connect([&queue, priority, delay](Args... data)
			{
				queue(priority, delay, std::move(data)...);
			});
	}

protected:	
	template <typename Tuple, int... S>
	inline void get(Tuple&& top, seq<S...>) const
	{
		_output(std::get<S>(std::forward<Tuple>(top))...);
	}

	inline auto _get() -> std::tuple<Args...>
	{
		_sem.wait();
		while(high_resolution_clock() < _queue.back()._timestamp)
		{
#ifndef _WIN32
			// for avoid sleeps, use coroutines
			usleep(100);
#endif
		}
		auto t = std::move(_queue.back());
		get(std::forward<std::tuple<Args...> >(t._data), gens<sizeof...(Args)>{});
		_queue.pop_back();
		return std::move(t._data);
	}
	
	inline auto _get(cu::yield_type& yield) -> std::tuple<Args...>
	{
		_sem.wait();
		while(high_resolution_clock() < _queue.back()._timestamp)
		{
			yield( cu::control_type{} );
		}
		auto t = std::move(_queue.back());
		get(std::forward<std::tuple<Args...> >(t._data), gens<sizeof...(Args)>{});
		_queue.pop_back();
		return std::move(t._data);
	}

protected:
	sync<Args...> _output;
	container_type _queue;
	fes::semaphore _sem;
};

}

#endif
