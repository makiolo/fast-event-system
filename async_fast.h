#ifndef _ASYNC_FAST_H_
#define _ASYNC_FAST_H_

#include <tuple>
#include <atomic>
#include <concurrentqueue/concurrentqueue.h>
#include <connection.h>
#include <sync.h>
#include <method.h>
#include <condition_variable>
#include <mutex>

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
		_queue.enqueue(std::make_tuple(data...));
		++_size_exact;
		_cond_var.notify_one();
	}

	void update()
	{
		if(!empty())
		{
			get();
		}
	}

	std::tuple<Args...> get()
	{
		return _get();
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
	inline void get(const std::tuple<Args...>& top, seq<S...>) const
	{
		_output(std::get<S>(top)...);
	}

	std::tuple<Args...> _get()
	{
		std::unique_lock<std::mutex> lock(_m);
		while (empty()) {
			_cond_var.wait(lock);
		}

		std::tuple<Args...> t;
		bool ok;
		do
		{
			ok = _queue.try_dequeue(t);
			if(ok)
			{
				get(t, gens<sizeof...(Args)>{});
				--_size_exact;
			}
		}
		while(!ok);
		return t;
	}

protected:
	sync<Args...> _output;
	container_type _queue;
	std::atomic<int> _size_exact;
	std::mutex _m;
	std::condition_variable _cond_var;
};

}  // end namespace

#endif

