#ifndef _ASYNC_FAST_H_
#define _ASYNC_FAST_H_

#include <tuple>
#include <atomic>
#include "concurrentqueue/blockingconcurrentqueue.h"
#include "sem.h"
#include "connection.h"
#include "sync.h"
#include "method.h"

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
	using container_type = moodycamel::BlockingConcurrentQueue<std::tuple<Args...>>;

	async_fast()
		: _output()
		, _queue()
	{ ; }

	async_fast(size_t initial_allocation)
		: _output()
		, _queue(initial_allocation)
	{ ; }

	~async_fast()
	{
		;
	}

	async_fast(const async_fast&) = delete;
	async_fast& operator=(const async_fast&) = delete;

	template <typename ... ARGS>
	void operator()(ARGS&&... data)
	{
		_queue.enqueue(std::make_tuple(std::forward<ARGS>(data)...));
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

	inline auto get() -> std::tuple<Args...>
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
	inline weak_connection<Args...> connect(sync<ARGS...>& callback)
	{
		return _output.connect([&callback](ARGS&&... data)
			{
				callback(std::forward<ARGS>(data)...);
			});
	}

	template <typename ... ARGS>
	inline weak_connection<Args...> connect(async_fast<ARGS...>& queue)
	{
		return _output.connect([&queue](ARGS&&... data)
			{
				queue(std::forward<ARGS>(data)...);
			});
	}

	template <typename ... ARGS>
	inline weak_connection<Args...> connect(int priority, deltatime delay, async_delay<ARGS...>& queue)
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
		std::tuple<Args...> t;
		_queue.wait_dequeue(t);
		get(std::forward<std::tuple<Args...> >(t), gens<sizeof...(Args)>{});
		return t;
	}

protected:
	sync<Args...> _output;
	container_type _queue;
	fes::semaphore _sem;
};

}  // end namespace

#endif
