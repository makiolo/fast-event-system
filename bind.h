#ifndef _FES_BIND_H_
#define _FES_BIND_H_

#include <vector>
#include "method.h"
#include "connection.h"
#include "clock.h"

namespace fes {

template <typename... Args>
class sync;
template <typename... Args>
class async_delay;
template <typename... Args>
class async_fast;

template <typename... Args>
class bind
{
public:
	using method = method<Args...>;

	explicit bind() : _connected(false)
	{
		;
	}

	template <typename T>
	inline void connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		if(_connected)
		{
			throw std::runtime_error("bind already connected!");
		}
		_reg = method(std::bind(ptr_func, obj, placeholder_template<Is>()...));
		_connected = true;
	}
	
	template <typename METHOD>
	inline void connect(METHOD&& fun)
	{
		if(_connected)
		{
			throw std::runtime_error("bind already connected!");
		}
		_reg = method(std::forward<METHOD>(fun));
		_connected = true;
	}

	inline void connect(sync<Args...>& callback)
	{
		connect([&callback](const Args&... data)
			{
				callback(data...);
			});
	}

	inline void connect(async_fast<Args...>& queue)
	{
		connect([&queue](Args... data)
			{
				queue(std::move(data)...);
			});
	}

	inline void connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		connect([&queue, priority, delay](Args... data)
			{
				queue(priority, delay, std::move(data)...);
			});
	}
	
	template <typename ... PARMS>
	void operator()(PARMS&&... data) const
	{
		(_reg)(std::forward<PARMS>(data)...);
	}

protected:
	method _reg;
	bool _connected;
};

}

#endif
