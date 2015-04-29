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
	method(const std::function<void(const Args&...)>& method)
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

	void operator()(const Args&& ... data)
	{
		_method(std::forward<const Args>(data)...);
	}
		
protected:
	std::function<void(const Args&...)> _method;
};

template <typename ... Args>
class callback
{
public:
	using methods = std::vector<method<Args...> >;
	
	callback() { ; }
	~callback() { ; }
	callback(const callback&) = delete;
	callback& operator=(const callback&) = delete;
	
	template <typename T>
	inline shared_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _connect(obj, ptr_func, make_int_sequence < sizeof...(Args) > {});
	}
	
	inline shared_connection<Args...> connect(const std::function<void(const Args&...)>& method)
	{
		_registered.emplace_back(method);
		return std::make_shared<internal_connection<Args ...> >([&](){
			//_registered.erase(it);
		});
	}
	
	void operator()(const Args&& ... data)
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
		_registered.emplace_back(std::bind(ptr_func, obj, placeholder_template<Is>{}...));
		return std::make_shared<internal_connection<Args ...> >([&](){
			//_registered.erase(it);
		});
	}
	
protected:
	methods _registered;
};

template <typename ... Args>
struct message
{
	message(int priority, std::chrono::system_clock::time_point timestamp, const Args&& ... data)
		: _priority(priority)
		, _timestamp(timestamp)
		, _data(std::forward<const Args>(data)...)
	{
		
	}
	
	message(const message& other)
		: _priority(other._priority)
		, _timestamp(other._timestamp)
		, _data(other._data)
	{
		
	}

	message(message&& other) noexcept
		: _priority(std::move(other._priority))
		, _timestamp(std::move(other._timestamp))
		, _data(std::move(other._data))
	{
		
	}

	/*
	message& operator=(const message& other)
	{
		message(other).swap(*this);
		return *this;
	}

	Copy-swap idiom
	http://stackoverflow.com/questions/276173/what-are-your-favorite-c-coding-style-idioms/2034447#2034447
	http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom?rq=1
	*/

#if 1
	message& operator=(message other)
	{
		swap(other);
		return *this;
	}
#else
	message& operator=(const message& other)
	{
		message(other).swap(*this);
		return *this;
	}

	message& operator=(message&& other) noexcept
	{
		message(std::move(other)).swap(*this);
		return *this;
	}
#endif

	void swap(message& other) noexcept
	{
		using std::swap;
		swap(_priority, other._priority);
		swap(_timestamp, other._timestamp);
		swap(_data, other._data);
	}

	~message()
	{
		
	}

	bool operator<(const message& other) const
	{
		std::cout << "sort message, priority " << _priority << std::endl;
		
		if (_timestamp < other._timestamp)
			return false;
		else if (_timestamp > other._timestamp)
			return true;
		
		return (_priority < other._priority);
	}
	
	int _priority;
	std::chrono::system_clock::time_point _timestamp;
	std::tuple<Args...> _data;
};

template <typename ... Args>
class queue_delayer
{
public:
	using container_type = std::priority_queue<message<Args...>, std::vector<message<Args...> > >;
	
	queue_delayer() { ; }
	~queue_delayer() { ; }
	queue_delayer(const queue_delayer&) = delete;
	queue_delayer& operator=(const queue_delayer&) = delete;

	template <typename R, typename P>
	void operator()(int priority, std::chrono::duration<R,P> delay, const Args&& ... data)
	{
		auto delay_point = std::chrono::high_resolution_clock::now() + delay;
		_queue.emplace(priority, delay_point, std::forward<const Args>(data)...);
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
	
	inline shared_connection<Args...> connect(const std::function<void(const Args&...)>& method)
	{
		return _output.connect(method);
	}
protected:
	template<int ...S>
	inline void dispatch(const std::tuple<Args...>& top, seq<S...>)
	{
		_output(std::move(std::get<S>(top)...));
	}

	bool _dispatch()
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		auto& t = _queue.top();
		if(t1 >= t._timestamp)
		{
			dispatch(t._data, gens<sizeof...(Args)>{});
			_queue.pop();
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

	inline shared_connection<Args...> connect(const std::function<void(const Args&...)>& method)
	{
		return _output.connect(method);
	}

protected:	
	template<int ...S>
	inline void dispatch(const std::tuple<Args...>& top, seq<S...>)
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

} // end namespace

#endif
