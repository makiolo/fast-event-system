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
using generator = std::shared_ptr< pull_type<T> >;
//using generator = pull_type<T>;

template <typename T>
using iterator = std::shared_ptr< push_type<T> >;
//using iterator = push_type<T>;

template <typename T>
using link = boost::function<void(fes::pull_type<T>&, fes::push_type<T>&)>;

template <typename T, typename Function>
generator<T> make_generator(Function&& f)
{
	return std::make_shared< pull_type<T> >(std::forward<Function>(f));
	//return pull_type<T>(std::forward<Function>(f));
}

template <typename T, typename Function>
iterator<T> make_iterator(Function&& f)
{
	return std::make_shared< push_type<T> >(std::forward<Function>(f));
	//return push_type<T>(std::forward<Function>(f));
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
		std::vector<generator<T> > coros;
		coros.emplace_back(fes::make_generator<T>( [](auto&) { ; } ));
		coros.emplace_back(fes::make_generator<T>(boost::bind(f, boost::ref(*coros.back().get()), _1)));
	}

	template <typename Function, typename ... Functions>
	pipeline(Function&& f, Functions&& ... fs)
	{
		std::vector<generator<T> > coros;
		coros.emplace_back(fes::make_generator<T>([](auto&) { ; }));
		coros.emplace_back(fes::make_generator<T>(boost::bind(f, boost::ref(*coros.back().get()), _1)));
		_add(coros, std::forward<Functions>(fs)...);
	}

protected:
	template <typename Function>
	void _add(std::vector<generator<T> >& coros, Function&& f)
	{
		coros.emplace_back(fes::make_generator<T>(boost::bind(f, boost::ref(*coros.back().get()), _1)));
	}

	template <typename Function, typename ... Functions>
	void _add(std::vector<generator<T> >& coros, Function&& f, Functions&& ... fs)
	{
		coros.emplace_back(fes::make_generator<T>(boost::bind(f, boost::ref(*coros.back().get()), _1)));
		_add(coros, std::forward<Functions>(fs)...);
	}
};

template <typename T>
class pipeline_iter
{
public:
	using in = fes::pull_type<T>;
	using out = fes::push_type<T>;
	using link = fes::link<T>;

	template <typename Function>
	pipeline_iter(Function&& f)
	{
		std::deque<iterator<T> > coros;
		coros.emplace_front(fes::make_iterator<T>( [](auto&) { ; } ));
		coros.emplace_front(fes::make_iterator<T>(boost::bind(f, _1, boost::ref(*coros.front().get()))));
		
		*coros.front()(60);
		*coros.front()(666);
		*coros.front()(7);
	}

	template <typename Function, typename ... Functions>
	pipeline_iter(Function&& f, Functions&& ... fs)
	{
		std::deque<iterator<T> > coros;
		coros.emplace_front(fes::make_iterator<T>([](auto&) { ; }));
		coros.emplace_front(fes::make_iterator<T>(boost::bind(f, _1, boost::ref(*coros.front().get()))));
		_add(coros, std::forward<Functions>(fs)...);
		
		*coros.front()(60);
		*coros.front()(666);
		*coros.front()(7);
	}
	
protected:
	template <typename Function>
	void _add(std::deque<iterator<T> >& coros, Function&& f)
	{
		coros.emplace_front(fes::make_iterator<T>(boost::bind(f, _1, boost::ref(*coros.front().get()))));
	}

	template <typename Function, typename ... Functions>
	void _add(std::deque<iterator<T> >& coros, Function&& f, Functions&& ... fs)
	{
		coros.emplace_front(fes::make_iterator<T>(boost::bind(f, _1, boost::ref(*coros.front().get()))));
		_add(coros, std::forward<Functions>(fs)...);
	}
};
	
using cmd = fes::pipeline<std::string>;

cmd::link cat()
{
	return [](cmd::in& source, cmd::out& yield)
	{
		std::string line;
		for (const auto s : source)
		{
			std::ifstream input(s);
			while (std::getline(input, line))
			{
				yield(line);
			}
		}
	};
}

void find_tree(const boost::filesystem::path& p, std::vector<std::string>& files)
{
	namespace fs = boost::filesystem;
	if(fs::is_directory(p))
	{
		for (auto f = fs::directory_iterator(p); f != fs::directory_iterator(); ++f)
		{
			if(fs::is_directory(f->path()))
			{
				find_tree(f->path(), files);
			}
			else
			{
				files.emplace_back(f->path().string());
			}
		}
	}
	else
	{
		files.emplace_back(p.string());
	}
}

cmd::link find(const std::string& dir)
{
	return [dir](cmd::in&, cmd::out& yield)
	{
		boost::filesystem::path p(dir);
		if (boost::filesystem::exists(p))
		{
			std::vector<std::string> files;
			find_tree(p, files);
			for(auto& f : files)
			{
				yield(f);
			}
		}
	};
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
	generator<std::tuple<Args...> > _g;
};

}  // end namespace

#endif
