// design-patterns-cpp14 by Ricardo Marmolejo Garc�a is licensed under a Creative Commons
// Reconocimiento 4.0 Internacional License.
// http://creativecommons.org/licenses/by/4.0/
//
#ifndef _ASYNC_FAST_H_
#define _ASYNC_FAST_H_

#include <tuple>
#include <atomic>
#include <concurrentqueue/concurrentqueue.h>
#include <connection.h>
#include <sync.h>
#include <method.h>

namespace fes {

template <typename... Args>
class async_delay;

/*!
 * # Canal seguro de comunicacion entre threads
 * Esta clase permite crear un canal de datos asincrono.
 * Mediante el *operator()* puede publicar datos.
 * Los clientes se conectan usando el metodo *connect()*
 */
template <typename... Args>
class async_fast
{
public:
	using container_type = moodycamel::ConcurrentQueue<std::tuple<Args...>>;

	async_fast()
        : _output()
        , _queue()
		, _size_exact(0)
	{ ; }

	async_fast(size_t initial_allocation)
        : _output()
		, _queue(initial_allocation)
		, _size_exact(0)
	{ ; }

	~async_fast()
	{
		;
	}

	async_fast(const async_fast&) = delete;
	async_fast& operator=(const async_fast&) = delete;

	void operator()(const Args&... data)
	{
		++_size_exact;
		_queue.enqueue(std::make_tuple(data...));
	}

	void update(deltatime tmax = fes::deltatime(16))
	{
		marktime timeout = high_resolution_clock() + tmax;
		while (!empty() && (high_resolution_clock() <= timeout))
		{
			_dispatch_one();
		}
	}

	void update_while(deltatime time)
	{
		auto mark = fes::high_resolution_clock() + time;
		while (fes::high_resolution_clock() <= mark)
		{
			_dispatch_one();
		}
	}

	bool dispatch_one()
	{
		if (!empty())
		{
			_dispatch_one();
			return true;
		}
		return false;
	}

	inline bool empty() const { return (_size_exact <= 0); }

	inline size_t size() const { return _size_exact; }

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

	void _dispatch_one()
	{
		std::tuple<Args...> t;
		if (_queue.try_dequeue(t))
		{
			--_size_exact;
			dispatch_one(t, gens<sizeof...(Args)>{});
		}
	}

protected:
	sync<Args...> _output;
	container_type _queue;
	std::atomic<int> _size_exact;
};

}  // end namespace

#endif

