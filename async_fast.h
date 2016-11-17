#ifndef _ASYNC_FAST_H_
#define _ASYNC_FAST_H_

#include <tuple>
#include <atomic>
#include <fast-event-system/concurrentqueue/blockingconcurrentqueue.h>
#include <fast-event-system/sem.h>
#include <fast-event-system/connection.h>
#include <fast-event-system/sync.h>
#include <fast-event-system/method.h>
#include <boost/coroutine2/coroutine.hpp>

namespace fes {

template <typename T>
using asymm_coro = boost::coroutines2::asymmetric_coroutine<T>;

template <typename T>
using pull_type = typename asymm_coro<T>::pull_type;

template <typename T>
using yield_type = typename asymm_coro<T>::push_type;

template <typename T>
using coroutine = std::shared_ptr< pull_type<T> >;

template <typename T, typename Function>
coroutine<T> make_coroutine(Function&& f)
{
	return std::make_shared< pull_type<T> >(std::forward<Function>(f));
}

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
	using container_type = moodycamel::BlockingConcurrentQueue<std::tuple<Args...>>;

	explicit async_fast()
		: _output()
		, _queue()
		, _closed(false)
		, _coro(make_coroutine<std::tuple<Args...> >([](auto& yield) {
				// yield(get());
			}))
	{ ; }

	explicit async_fast(size_t initial_allocation)
		: _output()
		, _queue(initial_allocation)
		, _closed(false)
		, _coro(make_coroutine<std::tuple<Args...> >([](auto& yield) {
				// yield(get());
			}))
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
		_sem.notify();
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

	inline weak_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
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
		_sem.wait();
		std::tuple<Args...> t;
		_queue.wait_dequeue(t);
		get(t, gens<sizeof...(Args)>{});
		return t;
	}

protected:
	sync<Args...> _output;
	container_type _queue;
	fes::semaphore _sem;
	bool _closed;
	coroutine<std::tuple<Args...> > _coro;
};

}  // end namespace

#endif
