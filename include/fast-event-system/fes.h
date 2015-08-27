// fes by Ricardo Marmolejo García is licensed under a Creative Commons Reconocimiento 4.0 Internacional License.// http://creativecommons.org/licenses/by/4.0/
// me/makiolo/dev/sandbox_private/vimfiles/doc/clang.txt' 
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
#include <ctime>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <concurrentqueue/concurrentqueue.h>
#pragma GCC diagnostic pop

#include <fast-event-system/common.h>
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

template <typename ... Args> using methods_t = std::list<method<Args...> >;

template <typename ... Args>
class internal_connection
{
public:
	using deleter_t = std::function<void(methods_t<Args...>&)>;

	internal_connection(methods_t<Args...>& registered, const deleter_t& deleter)
		: _deleter(deleter)
		, _connected(true)
		, _registered(registered)
	{ ; }
	
	internal_connection(const internal_connection<Args...>& other) = delete;
	const internal_connection<Args...>& operator=(const internal_connection<Args...>& other) = delete;
	
	void disconnect()
	{
		if (_connected)
		{
			_deleter(_registered);
			_connected = false;
		}
	}
	
protected:
	deleter_t _deleter;
	std::atomic<bool> _connected;
	methods_t<Args...>& _registered;
};
template <typename ... Args> using shared_connection = std::shared_ptr<internal_connection<Args...> >;
template <typename ... Args> using weak_connection = std::weak_ptr<internal_connection<Args...> >;

template <typename ... Args>
class connection
{
public:
	connection()
	{
		
	}

	connection(const weak_connection<Args ...>& other)
		: _connection(other)
	{
		
	}

	connection<Args...>& operator=(const weak_connection<Args ...>& other)
	{
		_connection = other;
		return *this;
	}

	~connection()
	{
		if(auto connection = _connection.lock())
		{
			connection->disconnect();
		}
	}

	connection(const connection&) = delete;
	connection(connection<Args ...>&&) = delete;
	connection& operator=(const connection&) = delete;
	connection& operator=(connection&&) = delete;

protected:
	weak_connection<Args...> _connection;
};

template <typename ... Args> class async_delay;
template <typename ... Args> class async_fast;

template <typename ... Args>
class sync
{
public:
	using methods = methods_t<Args...>;
	
	sync() = default;
	~sync() { ; }
	
	sync(const sync& other) = delete;
	sync& operator=(const sync& other) = delete;
	
	template <typename T>
	inline weak_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _connect(obj, ptr_func, make_int_sequence<sizeof...(Args)>{});
	}
	
	inline weak_connection<Args...> connect(const typename method<Args...>::function& fun)
	{
		typename methods::iterator it = _registered.emplace(_registered.end(), fun);
		shared_connection<Args...> conn = std::make_shared<internal_connection<Args ...> >(_registered, [it](methods& registered) {
			registered.erase(it);
		});
		_conns.push_back(conn);
		return weak_connection<Args...>(conn);
	}

	inline weak_connection<Args...> connect(sync<Args...>& callback)
	{
		return connect([&callback](const Args& ... data) {
			callback(data...);
		});
	}
	
	inline weak_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return connect([&queue](const Args& ... data) {
			queue(data...);
		});
	}
	
	inline weak_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return connect([&queue, priority, delay](const Args& ... data) {
			queue(priority, delay, data...);
		});
	}
	
	void operator()(const Args& ... data) const
	{
		for (auto& reg : _registered)
		{
			reg(data...);
		}
	}

protected:	
	template <typename T, int ... Is>
	weak_connection<Args...> _connect(T* obj, void (T::*ptr_func)(const Args&...), int_sequence<Is...>)
	{
		typename methods::iterator it = _registered.emplace(_registered.end(), std::bind(ptr_func, obj, placeholder_template<Is>{}...));
		shared_connection<Args...> conn = std::make_shared<internal_connection<Args ...> >(_registered, [it](methods& registered) {
			registered.erase(it);
		});
		_conns.push_back(conn);
		return weak_connection<Args...>(conn);
	}
	
protected:
	methods _registered;
	std::vector<shared_connection<Args...> > _conns;
};

template <typename ... Args>
struct message
{
	message(int priority, marktime timestamp, const Args& ... data)
		: _priority(priority)
		, _timestamp(timestamp)
		, _data(data...)
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
	
	void operator()(deltatime delay, const Args& ... data)
	{
		operator()(0, delay, data...);
	} 

	void operator()(int priority, const Args& ... data)
	{
		operator()(priority, fes::deltatime(0), data...);
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

	void update_while(deltatime time)
	{
		auto mark = fes::high_resolution_clock() + time;
		while(fes::high_resolution_clock() < mark)
		{
			update();
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
		return _output.connect([&callback](const Args& ... data) {
			callback(data...);
		});
	}
	
	inline weak_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return _output.connect([&queue](const Args& ... data) {
			queue(data...);
		});
	}

	inline weak_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return _output.connect([&queue, priority, delay](const Args& ... data) {
			queue(priority, delay, data...);
		});
	}

protected:
	template<int ...S>
	inline void dispatch_one(const std::tuple<Args...>& top, seq<S...>) const
	{
		_output( std::get<S>(top)... );
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
		return _output.connect([&callback](const Args& ... data) {
			callback(data...);
		});
	}
	
	inline weak_connection<Args...> connect(async_fast<Args...>& queue)
	{
		return _output.connect([&queue](const Args& ... data) {
			queue(data...);
		});
	}

	inline weak_connection<Args...> connect(int priority, deltatime delay, async_delay<Args...>& queue)
	{
		return _output.connect([&queue, priority, delay](const Args& ... data) {
			queue(priority, delay, data...);
		});
	}
	
protected:	
	template<int ...S>
	inline void dispatch_one(const std::tuple<Args...>& top, seq<S...>) const
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
