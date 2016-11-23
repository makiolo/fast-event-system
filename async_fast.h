#ifndef _ASYNC_FAST_H_
#define _ASYNC_FAST_H_

#include <tuple>
#include <atomic>
#include <deque>
#include <fast-event-system/concurrentqueue/blockingconcurrentqueue.h>
#include <fast-event-system/sem.h>
#include <fast-event-system/connection.h>
#include <fast-event-system/sync.h>
#include <fast-event-system/method.h>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>

namespace fes {

template <typename T>
using asymm_coroutine = boost::coroutines2::asymmetric_coroutine<T>;

template <typename T>
using pull_type = typename asymm_coroutine<T>::pull_type;

template <typename T>
using push_type = typename asymm_coroutine<T>::push_type;

template <typename T>
using pull_type_ptr = std::shared_ptr< pull_type<T> >;

template <typename T>
using push_type_ptr = std::shared_ptr< push_type<T> >;

template <typename T>
using link = boost::function<void(fes::pull_type<T>&, fes::push_type<T>&)>;

template <typename T, typename Function>
pull_type_ptr<T> make_generator(Function&& f)
{
	return std::make_shared< pull_type<T> >(std::forward<Function>(f));
}

template <typename T, typename Function>
push_type_ptr<T> make_iterator(Function&& f)
{
	return std::make_shared< push_type<T> >(std::forward<Function>(f));
}

template <typename T>
link<T> end_link()
{
	return [](fes::pull_type<T>& source, fes::push_type<T>&)
	{
		for (auto& s : source) { ; }
	};
}

template <typename T>
class pipeline
{
public:
	using in = fes::pull_type<T>;
	using out = fes::push_type<T>;
	using link = fes::link<T>;

	template <typename Function>
	pipeline(Function&& f)
	{
		std::vector<pull_type_ptr<T> > coros;
		coros.emplace_back(fes::make_generator<T>( [](auto& yield) { ; } ));
		coros.emplace_back(fes::make_generator<T>(boost::bind(f, boost::ref(*coros.back().get()), _1)));
		coros.emplace_back(fes::make_generator<T>(boost::bind(end_link<T>(), boost::ref(*coros.back().get()), _1)));
	}

	template <typename Function, typename ... Functions>
	pipeline(Function&& f, Functions&& ... fs)
	{
		std::vector<pull_type_ptr<T> > coros;
		coros.emplace_back(fes::make_generator<T>([](auto& yield) { ; }));
		coros.emplace_back(fes::make_generator<T>(boost::bind(f, boost::ref(*coros.back().get()), _1)));
		_add(coros, std::forward<Functions>(fs)...);
		coros.emplace_back(fes::make_generator<T>(boost::bind(end_link<T>(), boost::ref(*coros.back().get()), _1)));
	}

protected:
	template <typename Function>
	void _add(std::vector<pull_type_ptr<T> >& coros, Function&& f)
	{
		coros.emplace_back(fes::make_generator<T>(boost::bind(f, boost::ref(*coros.back().get()), _1)));
	}

	template <typename Function, typename ... Functions>
	void _add(std::vector<pull_type_ptr<T> >& coros, Function&& f, Functions&& ... fs)
	{
		coros.emplace_back(fes::make_generator<T>(boost::bind(f, boost::ref(*coros.back().get()), _1)));
		_add(coros, std::forward<Functions>(fs)...);
	}
};

template <typename T>
class channel
{
public:
	using in = fes::pull_type<T>;
	using out = fes::push_type<T>;
	using link = fes::link<T>;

	channel()
	{
		_set_tail();
	}

	template <typename Function>
	channel(Function&& f)
	{
		_set_tail();
		_add(std::forward<Function>(f));
	}

	template <typename Function, typename ... Functions>
	channel(Function&& f, Functions&& ... fs)
	{
		_set_tail();
		_add(std::forward<Function>(f), std::forward<Functions>(fs)...);
	}

	template <typename Function>
	void connect(Function&& f)
	{
		_add(std::forward<Function>(f));
	}

	void operator()(const T& data)
	{
		(*_coros.front())(data);
	}

protected:
	void _set_tail()
	{
		_coros.emplace_front(fes::make_iterator<T>([](auto& source) { for(auto& v: source) { ; }; }));
	}

	template <typename Function>
	void _add(Function&& f)
	{
		_coros.emplace_front(fes::make_iterator<T>(boost::bind(f, _1, boost::ref(*_coros.front().get()))));
	}

	template <typename Function, typename ... Functions>
	void _add(Function&& f, Functions&& ... fs)
	{
		_add(std::forward<Functions>(fs)...);
		_coros.emplace_front(fes::make_iterator<T>(boost::bind(f, _1, boost::ref(*_coros.front().get()))));
	}
protected:
	std::deque<push_type_ptr<T> > _coros;
};

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

	explicit async_fast(size_t initial_allocation = 128)
		: _output()
		, _queue(initial_allocation)
		, _closed(false)
		, _g(make_generator<std::tuple<Args...> >([this](auto& yield) {
				/*
				// coroutines boost execute in construction
				std::tuple<Args...> t;
				yield(t);

				while(true)
				{
					yield(this->_get());
				}
				*/
			}))
	{
		;
	}

	~async_fast()
	{
		;
	}

	async_fast(const async_fast&) = delete;
	async_fast& operator=(const async_fast&) = delete;

	void close()
	{
		// how close a channel
	}

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

	auto begin()
	{
		return begin(_g);
	}

	auto end()
	{
		return end(_g);
	}

	inline std::tuple<Args...> get()
	{
		/*
		if(*_g)
		{
		*/
			// return (*_g)();
			return _get();
		/*
		}
		else
		{
			// exception
			return std::tuple<Args...>();
		}
		*/
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

	template <int... S>
	inline void __get(const std::tuple<Args...>& top, seq<S...>) const
	{
		_output(std::get<S>(top)...);
	}

	std::tuple<Args...> _get()
	{
		_sem.wait();
		std::tuple<Args...> t;
		_queue.wait_dequeue(t);
		__get(t, gens<sizeof...(Args)>{});
		return t;
	}

protected:
	sync<Args...> _output;
	container_type _queue;
	fes::semaphore _sem;
	bool _closed;
	pull_type_ptr<std::tuple<Args...> > _g;
};

}  // end namespace

#endif

