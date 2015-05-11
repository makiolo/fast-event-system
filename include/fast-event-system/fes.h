// design-patterns-cpp14 by Ricardo Marmolejo García is licensed under a Creative Commons Reconocimiento 4.0 Internacional License.
// http://creativecommons.org/licenses/by/4.0/
//



#ifndef _FAST_EVENT_SYSTEM_
#define _FAST_EVENT_SYSTEM_

#include <functional>
#include <string>
#include <memory>
#include <map>
#include <exception>
#include <thread>
#include <chrono>
#include <deque>
#include <algorithm>
#include <iostream>
#include <tuple>
#include <functional>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <queue>
#include <fast-event-system/common.h>

#ifdef _WIN32
#define noexcept _NOEXCEPT
#endif

namespace fes {

template <typename ... Args>
class internal_connection
{
public:
	internal_connection(const std::function<void(void)>& deleter)
		: _deleter(deleter)
	{
		
	}
	
	internal_connection(const internal_connection<Args...>& other) = delete;
	const internal_connection<Args...>& operator=(const internal_connection<Args...>& other) = delete;
	
	void disconnect()
	{
		_deleter();
	}

	~internal_connection()
	{
		
	}
	
protected:
	std::function<void(void)> _deleter;
};
template <typename ... Args> using shared_connection = std::shared_ptr<internal_connection<Args...> >;

template <typename ... Args>
class connection
{
public:
	connection()
	{
		
	}

	connection(const shared_connection<Args ...>& other)
		: _connection(other)
	{
		
	}
	
	const connection<Args...>& operator=(const shared_connection<Args ...>&other)
	{
		_connection = other;
		return *this;
	}

	~connection()
	{
		if (_connection)
		{
			_connection->disconnect();
		}
	}

	connection(const connection&) = delete;
	connection& operator=(const connection&) = delete;

protected:
	shared_connection<Args...> _connection;
};

template <typename ... Args>
class method
{
public:
	using function = std::function<void(const Args&...)>;

	method(const function& method)
		: _method(method)
	{
		
	}

	template <typename T>
	method(T* obj, void (T::*ptr_func)(const Args&...))
		: method(obj, ptr_func, make_int_sequence<sizeof...(Args)>{})
	{
		
	}

	template <typename T, int ... Is>
	method(T* obj, void (T::*ptr_func)(const Args&...), int_sequence<Is...>)
		: method(std::bind(ptr_func, obj, placeholder_template<Is>{}...))
	{
		
	}

	~method() { ; }
	method(const method&) = delete;
	method& operator=(const method&) = delete;

	void operator()(const Args&& ... data) const
	{
		_method(std::forward<const Args>(data)...);
	}
		
protected:
	function _method;
};

template <typename ... Args>
class callback
{
public:
	using methods = std::list<method<Args...> >;
	
	callback() { ; }
	~callback() { ; }
	callback(const callback&) = delete;
	callback& operator=(const callback&) = delete;
	
	template <typename T>
	inline shared_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _connect(obj, ptr_func, make_int_sequence<sizeof...(Args)>{});
	}
	
	inline shared_connection<Args...> connect(const typename method<Args...>::function& method)
	{
		auto it = _registered.emplace(_registered.end(), method);
		return std::make_shared<internal_connection<Args ...> >([&](){
			_registered.erase(it);
		});
	}
	
	void operator()(const Args&& ... data) const
	{
		for(auto& reg : _registered)
		{
			reg(std::forward<const Args>(data)...);
		}
	}

protected:	
	template <typename T, int ... Is>
	inline shared_connection<Args...> _connect(T* obj, void (T::*ptr_func)(const Args&...), int_sequence<Is...>)
	{
		auto it = _registered.emplace(_registered.end(), std::bind(ptr_func, obj, placeholder_template<Is>{}...));
		return std::make_shared<internal_connection<Args ...> >([&](){
			_registered.erase(it);
		});
	}
	
protected:
	methods _registered;
};

template <typename ... Args>
struct message
{
	message(int priority, std::chrono::system_clock::time_point timestamp, const Args ... data)
		: _priority(priority)
		, _timestamp(timestamp)
		, _data(std::move(data)...)
	{
		//std::cout << "constructor message" << std::endl;
	}
	
	message(const message& other)
		: _priority(other._priority)
		, _timestamp(other._timestamp)
		, _data(other._data)
	{
		//std::cout << "constructor copy" << std::endl;
	}
	
	message(message&& other) noexcept
		: _priority(std::move(other._priority))
		, _timestamp(std::move(other._timestamp))
		, _data(std::move(other._data))
	{
		//std::cout << "constructor move" << std::endl;
	}
	
	/*
	Copy-swap idiom
	http://stackoverflow.com/questions/276173/what-are-your-favorite-c-coding-style-idioms/2034447#2034447
	http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom?rq=1
	*/
	
	message& operator=(const message& other)
	{
		//std::cout << "operator= copy" << std::endl;
		message(other).swap(*this);
		return *this;
	}
	
	message& operator=(message&& other) noexcept
	{
		//std::cout << "operator= move" << std::endl;
		message(std::move(other)).swap(*this);
		return *this;
	}
	
	void swap(message& other) noexcept
	{
		//std::cout << "swap" << std::endl;
		using std::swap;
		swap(_priority, other._priority);
		swap(_timestamp, other._timestamp);
		swap(_data, other._data);
	}
	
	~message()
	{
		//std::cout << "destructor" << std::endl;
	}
	
	int _priority;
	std::chrono::system_clock::time_point _timestamp;
	std::tuple<Args...> _data;
};

// must complain strict weak order
template <typename ... Args>
struct message_comp
{
    bool operator() (const message<Args...>& one, const message<Args...>& other)
    {
	if (one._timestamp < other._timestamp)
		return false;
	else if (one._timestamp > other._timestamp)
		return true;
	
	if(one._priority < other._priority)
		return true;
	else if(one._priority > other._priority)
		return false;
	
	return false;
    }
};

template <typename ... Args>
class queue_delayer
{
public:
	using container_type = std::vector<message<Args...> >;
	
	queue_delayer() { ; }
	~queue_delayer() { ; }
	queue_delayer(const queue_delayer&) = delete;
	queue_delayer& operator=(const queue_delayer&) = delete;
	
	template <typename R, typename P>
	void operator()(int priority, std::chrono::duration<R,P> delay, const Args&& ... data)
	{
		auto delay_point = std::chrono::high_resolution_clock::now() + delay;
		_queue.emplace_back(priority, delay_point, std::forward<const Args>(data)...);
		std::sort(std::begin(_queue), std::end(_queue), message_comp<Args...>());
	}
	
	void update()
	{
		while(!_queue.empty())
		{
			_dispatch();
		}
	}

	bool dispatch()
	{
		if(!_queue.empty())
		{
			return _dispatch();
		}
		return false;
	}
		
	bool empty() const
	{
		return _queue.empty();
	}
	
	size_t size() const
	{
		return _queue.size();
	}
	
	template <typename T>
	inline shared_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _output.connect(obj, ptr_func);
	}
	
	inline shared_connection<Args...> connect(const typename method<Args...>::function& method)
	{
		return _output.connect(method);
	}
protected:
	template<int ...S>
	inline void dispatch(const std::tuple<Args...>& top, seq<S...>) const
	{
		_output(std::move(std::get<S>(top)...));
	}

	bool _dispatch()
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		auto& t = _queue.back();
		if(t1 >= t._timestamp)
		{
			dispatch(t._data, gens<sizeof...(Args)>{});
			_queue.pop_back();
			return true;
		}
		return false;
	}

protected:
	callback<Args... > _output;
	container_type _queue;
};

template <typename ... Args>
class queue_fast
{
public:
	using container_type = std::queue<std::tuple<Args...>, std::deque<std::tuple<Args...> > >;
	
	queue_fast() { ; }
	~queue_fast() { ; }
	queue_fast(const queue_fast&) = delete;
	queue_fast& operator=(const queue_fast&) = delete;
	
	void operator()(const Args&& ... data)
	{
		_queue.emplace(std::forward<const Args>(data)...);
	}
	
	void update()
	{
		while(!_queue.empty())
		{
			_dispatch();
		}
	}

	bool dispatch()
	{
		if(!_queue.empty())
		{
			_dispatch();
			return true;
		}
		return false;
	}

	bool empty() const
	{
		return _queue.empty();
	}

	size_t size() const
	{
		return _queue.size();
	}

	template <typename T>
	inline shared_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _output.connect(obj, ptr_func);
	}

	inline shared_connection<Args...> connect(const typename method<Args...>::function& method)
	{
		return _output.connect(method);
	}

protected:	
	template<int ...S>
	inline void dispatch(const std::tuple<Args...>& top, seq<S...>) const
	{
		_output(std::move(std::get<S>(top)...));
	}

	void _dispatch()
	{
		auto& t = _queue.front();
		dispatch(t, gens < sizeof...(Args) > {});
		_queue.pop();
	}

protected:
	callback<Args...> _output;
	container_type _queue;
};

template <typename Args ...>
class queue_interleace
{
public:
	explicit queue_interleace()
		: _empty_first(true)
		, _empty_second(true)
		, _fisrt_is_more_early_no_empty(true)
	{
		
	}
	
	~queue_interleace()
	{
		
	}
	
	void update_state()
	{
		_empty_first = _first.empty();
		_empty_second = _second.empty();
	}
	
	void push_first(const Args&& ... data)
	{
		if(_empty_second)
		{
			_fisrt_is_more_early_no_empty = true;
		}
		_first(std::forward<const Args>(data)...);
		update_state();
	}
	
	void push_second(const Args&& ... data)
	{
		if(_empty_first)
		{
			_fisrt_is_more_early_no_empty = false;
		}
		_second(std::forward<const Args>(data)...);
		update_state();
	}
	
	bool dispatch()
	{
		bool ok;
		
		if(_empty_first)
		{
			if(_empty_second)
			{
				// both empty, nothing to do
				ok = false;
			}
			else
			{
				// firts empty, dispatch only second
				ok = _second.dispatch();
			}
		}
		else
		{
			if(_empty_second)
			{
				// second empty, dispatch only first
				ok = _first.dispatch();
			}
			else
			{
				// alternation based in "fisrt_is_more_early_no_empty"
				if(_fisrt_is_more_early_no_empty)
				{
					ok = _first.dispatch();
				}
				else
				{
					ok = _second.dispatch();
				}
				
				// flip
				_fisrt_is_more_early_no_empty = !_fisrt_is_more_early_no_empty;
			}
		}
		update_state();
		return ok;
	}
	
	void update()
	{
		while(!_first.empty() && !_second.empty())
		{
			_dispatch();
		}
	}
	
protected:
	queue_fast<Args...> _first;
	queue_fast<Args...> _second;
	bool _empty_first;
	bool _empty_second;
	// first queue that leave your empty state
	bool _fisrt_is_more_early_no_empty;
};

} // end namespace

#endif

// copy from https://github.com/progschj/ThreadPool/blob/master/ThreadPool.h
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace fes {

class thread_pool {
public:
    thread_pool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~thread_pool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
 
// the constructor just launches some amount of workers
inline thread_pool::thread_pool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto thread_pool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped thread_pool");

        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline thread_pool::~thread_pool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

} // end namespace

#endif

