#ifndef _SYNC_H_
#define _SYNC_H_

#include <vector>
#include "method.h"
#include "connection.h"
#include "clock.h"

namespace fes {

template <typename... Args>
class async_delay;
template <typename... Args>
class async_fast;

template <typename... Args>
class sync
{
public:
	using methods = methods_t<Args...>;

	explicit sync()
		: _registered()
		, _conns()
	{
		;
	}
	
	~sync() { ; }
	sync(const sync& other) = delete;
	sync& operator=(const sync& other) = delete;

	template <typename T, typename  ... PARMS>
	inline weak_connection<Args...> connect(T* obj, void (T::*ptr_func)(const PARMS&...))
	{
		return _connect(obj, ptr_func, make_int_sequence<sizeof...(PARMS)>{});
	}
	
	template <typename METHOD>
	inline weak_connection<Args...> connect(METHOD&& fun)
	{
		typename methods::iterator it = _registered.emplace(_registered.end(), std::forward<METHOD>(fun));
		shared_connection<Args...> conn
			= std::make_shared<internal_connection<Args...>>(_registered, [it](methods& registered)
				{
					registered.erase(it);
				});
		_conns.push_back(conn);
		return weak_connection<Args...>(conn);
	}

	template <typename ... PARMS>
	inline weak_connection<Args...> connect(sync<Args...>& callback)
	{
		return connect([&callback](PARMS&&... data)
			{
				callback(std::forward<PARMS>(data)...);
			});
	}

	template <typename ... PARMS>
	inline weak_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return connect([&queue](PARMS&&... data)
			{
				queue(std::forward<PARMS>(data)...);
			});
	}

	template <typename ... PARMS>
	inline weak_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return connect([&queue, priority, delay](PARMS&&... data)
			{
				queue(priority, delay, std::forward<PARMS>(data)...);
			});
	}
	
	template <typename ... PARMS>
	void operator()(PARMS&&... data) const
	{
		auto it = _registered.begin();
		auto ite = _registered.end();
		auto itee = --_registered.end();
		for(; it != ite; ++it)
		{
			// is last_iteration ?
			auto& reg = *it;
			if(it == itee)
			{
				// forward
				reg(std::forward<PARMS>(data)...);
			}
			else
			{
				// copy
				reg.call_copy(data...);
			}
		}
	}

protected:
	template <typename T, int... Is>
	weak_connection<Args...> _connect(T* obj, void (T::*ptr_func)(Args&&...), int_sequence<Is...>)
	{
		typename methods::iterator it = _registered.emplace(
			_registered.end(), std::bind(ptr_func, obj, placeholder_template<Is>{}...));
		shared_connection<Args...> conn
			= std::make_shared<internal_connection<Args...> >(_registered, [it](methods& registered)
				{
					registered.erase(it);
				});
		_conns.push_back(conn);
		return weak_connection<Args...>(conn);
	}

protected:
	methods _registered;
	std::vector<shared_connection<Args...>> _conns;
};

}

#endif
