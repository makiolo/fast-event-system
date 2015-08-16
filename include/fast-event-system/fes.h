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
#include <mutex>
#include <condition_variable>
#include <future>
#include <fast-event-system/common.h>
#include <concurrentqueue/concurrentqueue.h>
#include <ctime>
#include <fast-event-system/api.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#define noexcept _NOEXCEPT
#endif

namespace fes {

// BEGIN workaround

#ifdef _WIN32
// std::chrono in windows is bugged:
// https://connect.microsoft.com/VisualStudio/feedback/details/719443/c-chrono-headers-high-resolution-clock-does-not-have-high-resolution

using marktime = double;
using deltatime = double;

bool init_clock();
fast_event_system_API marktime high_resolution_clock();

#else // gcc, clang ...

using marktime = std::chrono::system_clock::time_point;
using deltatime = std::chrono::milliseconds;

marktime high_resolution_clock();

#endif
// END workaround

template <typename ... Args>
class internal_connection
{
public:
	internal_connection(const std::function<void(void)>& deleter)
		: _deleter(deleter)
		, _connected(true)
	{
		
	}
	
	internal_connection(const internal_connection<Args...>& other) = delete;
	const internal_connection<Args...>& operator=(const internal_connection<Args...>& other) = delete;
	
	void disconnect()
	{
		if (_connected)
		{
			_deleter();
			_connected = false;
		}
	}

	void fake_disconnect()
	{
		_connected = false;
	}

	~internal_connection()
	{
		
	}
	
protected:
	std::function<void(void)> _deleter;
	bool _connected;
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
	
	method(const method& other) = delete;
	method& operator=(const method& other) = delete;
	~method() { ; }

	void operator()(const Args& ... data) const
	{
		_method(data...);
	}
		
protected:
	function _method;
};

template <typename ... Args> class async_delay;
template <typename ... Args> class async_fast;

template <typename ... Args>
class sync
{
public:
	using methods = std::list<method<Args...> >;
	
	sync() = default;
	~sync()
	{
		for (auto& c : _conns)
		{
			c->fake_disconnect();
		}
	}
	
	sync(const sync& other) = delete;
	sync& operator=(const sync& other) = delete;
	
	template <typename T>
	inline shared_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _connect(obj, ptr_func, make_int_sequence<sizeof...(Args)>{});
	}
	
	inline shared_connection<Args...> connect(const typename method<Args...>::function& method)
	{
		auto it = _registered.emplace(_registered.end(), method);
		shared_connection<Args...> conn = std::make_shared<internal_connection<Args ...> >([&]() {
			_registered.erase(it);
		});
		_conns.emplace_back(conn);
		return conn;
	}

	inline shared_connection<Args...> connect(sync<Args...>& callback)
	{
		return connect([&callback](const Args& ... data) {
			callback(data...);
		});
	}
	
	inline shared_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return connect([&queue](const Args& ... data) {
			queue(data...);
		});
	}
	
	inline shared_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return connect([&queue, priority, delay](const Args& ... data) {
			queue(priority, delay, data...);
		});
	}
	
	void operator()(const Args& ... data) const
	{
		for(auto& reg : _registered)
		{
			reg(data...);
		}
	}

protected:	
	template <typename T, int ... Is>
	shared_connection<Args...> _connect(T* obj, void (T::*ptr_func)(const Args&...), int_sequence<Is...>)
	{
		auto it = _registered.emplace(_registered.end(), std::bind(ptr_func, obj, placeholder_template<Is>{}...));
		shared_connection<Args...> conn = std::make_shared<internal_connection<Args ...> >([&](){
			_registered.erase(it);
		});
		_conns.emplace_back(conn);
		return conn;
	}
	
protected:
	methods _registered;
	std::vector<shared_connection<Args...> > _conns;
};

template <typename ... Args>
struct message
{
	message(int priority, marktime timestamp, const Args ... data)
		: _priority(priority)
		, _timestamp(timestamp)
		, _data(std::move(data)...)
	{
		
	}
	
	message(const message& other)
		: _priority(other._priority)
		, _timestamp(other._timestamp)
		, _data(other._data)
	{

	}
	
	message(message&& other) noexcept
		: _priority(other._priority)
		, _timestamp(other._timestamp)
		, _data(std::move(other._data))
	{

	}
	
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
	
	int _priority;
	marktime _timestamp;
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
class async_delay
{
public:
	using container_type = std::vector<message<Args...> >;
	
	async_delay() = default;
	~async_delay() = default;
	async_delay(const async_delay&) = delete;
	async_delay& operator=(const async_delay&) = delete;
	
	void operator()(int priority, deltatime delay, const Args& ... data)
	{
		marktime delay_point = high_resolution_clock() + delay;
		_queue.emplace_back(priority, delay_point, data...);
		std::sort(std::begin(_queue), std::end(_queue), message_comp<Args...>());
	}

	inline void operator()(const Args& ... data)
	{
		operator()(0, fes::deltatime(0), data...);
	}
	
	void update(deltatime tmax = fes::deltatime(1))
	{
		marktime timeout = high_resolution_clock() + tmax;
		bool has_next = true;
		while (!empty() && has_next && (high_resolution_clock() <= timeout))
		{
			has_next = _dispatch_one();
		}
	}

	bool dispatch_one()
	{
		if (!empty())
		{
			return _dispatch_one();
		}
		return false;
	}
		
	inline bool empty() const
	{
		return _queue.empty();
	}
	
	inline size_t size() const
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

	inline shared_connection<Args...> connect(sync<Args...>& callback)
	{
		return _output.connect([&callback](const Args& ... data) {
			callback(data...);
		});
	}
	
	inline shared_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return _output.connect([&queue](const Args& ... data) {
			queue(data...);
		});
	}

	inline shared_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return _output.connect([&queue, priority, delay](const Args& ... data) {
			queue(priority, delay, data...);
		});
	}

protected:
	template<int ...S>
	inline void dispatch_one(std::tuple<Args...> top, seq<S...>) const
	{
		_output(std::get<S>(top)...);
	}

	bool _dispatch_one()
	{
		auto& t = _queue.back();
		if (high_resolution_clock() >= t._timestamp)
		{
			dispatch_one(t._data, gens<sizeof...(Args)>{});
			_queue.pop_back();
			return true;
		}
		return false;
	}

protected:
	sync<Args... > _output;
	container_type _queue;
};

template <typename ... Args>
class async_fast
{
public:
	using container_type = moodycamel::ConcurrentQueue<std::tuple<Args...> >;
	
	async_fast() = default;
	~async_fast() = default;
	async_fast(const async_fast&) = delete;
	async_fast& operator=(const async_fast&) = delete;
	
	void operator()(const Args& ... data)
	{
		_queue.enqueue(std::make_tuple(data...));
	}
	
	void update(deltatime tmax = fes::deltatime(1))
	{
		marktime timeout = high_resolution_clock() + tmax;
		bool has_next = true;
		while (!empty() && has_next && (high_resolution_clock() <= timeout))
		{
			has_next = _dispatch_one();
		}
	}
	
	bool dispatch_one()
	{
		if (!empty())
		{
			return _dispatch_one();
		}
		return false;
	}

	inline bool empty() const
	{
		return (_queue.size_approx() <= 0);
	}

	inline size_t size() const
	{
		return _queue.size_approx();
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
	
	inline shared_connection<Args...> connect(sync<Args...>& callback)
	{
		return _output.connect([&callback](const Args& ... data) {
			callback(data...);
		});
	}
	
	inline shared_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return _output.connect([&queue](const Args& ... data) {
			queue(data...);
		});
	}

	inline shared_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return _output.connect([&queue, priority, delay](const Args& ... data) {
			queue(priority, delay, data...);
		});
	}

protected:	
	template<int ...S>
	inline void dispatch_one(std::tuple<Args...> top, seq<S...>) const
	{
		_output(std::get<S>(top)...);
	}

	bool _dispatch_one()
	{
		std::tuple<Args...> t;
		if (_queue.try_dequeue(t))
		{
			dispatch_one(t, gens < sizeof...(Args) > {});
			return true;
		}
		return false;
	}

protected:
	sync<Args...> _output;
	container_type _queue;
};

} // end namespace

#endif
