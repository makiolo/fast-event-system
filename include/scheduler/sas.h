// Ricardo Marmolejo Garcia
// 15-04-2015
#ifndef _SCHEDULER_ADVANCED_SIMPLE_
#define _SCHEDULER_ADVANCED_SIMPLE_

#ifdef _WIN32
#include <windows.h>
#endif
#include <functional>
#include <future>
#include <atomic>
#include <mutex>
#include <csignal>
#include <fast-event-system/fes.h>
#include <animator/interpolation.h>
#include <Poco/Thread.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>

namespace asyncply {

template <typename R>
class task;

template< typename Function> using task_t = task<typename std::result_of<Function()>::type>;
template< typename Function> using shared_task = std::shared_ptr<task_t<Function> >;
template< typename Function> shared_task<Function> run(Function&& f);

template <typename R>
class future
{
public:
	future(Poco::Semaphore& mutex)
		: _mutex(mutex)
		, _ready(false)
	{
		
	}

	~future()
	{
		
	}
	
	R& get()
	{
		Poco::Mutex::ScopedLock lock(_zone);
		if (!_ready)
		{
			_mutex.wait();
			_ready = true;
			if (_exception)
			{
				std::rethrow_exception(_exception);
			}
		}
		return _value;
	}
	
	void set_value(const R& value)
	{
		_value = value;
	}

	void set_value(R&& value)
	{
		_value = std::forward<R>(value);
	}

	void set_exception(std::exception_ptr p)
	{
		_exception = p;
	}

protected:
	Poco::Semaphore& _mutex;
	Poco::Mutex _zone;
	R _value;
	std::exception_ptr _exception;
	std::atomic<bool> _ready;
};

template <>
class future<void>
{
public:
	future(Poco::Semaphore& mutex)
		: _mutex(mutex)
		, _ready(false)
	{
		
	}

	~future()
	{

	}
	
	void get()
	{
		Poco::Mutex::ScopedLock lock(_zone);
		if (!_ready)
		{
			_mutex.wait();
			_ready = true;
			if (_exception)
			{
				std::rethrow_exception(_exception);
			}
		}
	}
	
	void set_exception(std::exception_ptr p)
	{
		_exception = p;
	}
	
protected:
	Poco::Semaphore& _mutex;
	Poco::Mutex _zone;
	std::exception_ptr _exception;
	std::atomic<bool> _ready;
};

template <typename R>
class promise
{
public:
	promise()
		: _future( std::make_shared<future<R> >(_mutex) )
		, _mutex(0, 1)
	{
		
	}
	
	~promise()
	{
		
	}

	std::shared_ptr< future<R> > get_future() const
	{
		return _future;
	}

	void set_value(const R& value)
	{
		_future->set_value(value);
	}

	void set_value(R&& value)
	{
		_future->set_value(std::forward<R>(value));
	}

	void set_exception(std::exception_ptr p)
	{
		_future->set_exception(p);
	}

	void signal()
	{
		_mutex.set();
	}

protected:
	std::shared_ptr< future<R> > _future;
	Poco::Semaphore _mutex;
};

template <>
class promise<void>
{
public:
	promise()
		: _future( std::make_shared<future<void> >( _mutex ) )
		, _mutex(0, 1)
	{
		
	}
	
	~promise()
	{
		
	}
	
	std::shared_ptr< future<void> > get_future() const
	{
		return _future;
	}
	
	void set_exception(std::exception_ptr p)
	{
		_future->set_exception(p);
	}
	
	void signal()
	{
		_mutex.set();
	}
	
protected:
	std::shared_ptr< future<void> > _future;
	Poco::Semaphore _mutex;
};

template <typename R>
class task : public Poco::Runnable
{
public:
	using func = std::function<R()>;
	using return_type = typename std::remove_reference<R>::type;
	using then_type = std::function<void(const return_type&)>;

	task(const func& method)
		: _method(method)
	{
		
	}
	
	~task()
	{
		
	}

	task(const task& te) = delete;
	task& operator = (const task& te) = delete;

	R& wait()
	{
		return get_future()->get();
	}
	
	std::shared_ptr< future<R> > get_future() const
	{
		return _result.get_future();
	}

	void run() override
	{
		{
			try {
				auto v = _method();
				_result.set_value(v);
				if (_post_method)
				{
					_post_method(v);
				}
			}
			catch (...) {
				try {
					_result.set_exception(std::current_exception());
				}
				catch (...) { ; }
			}
		}
		_result.signal();
	}
	
	void then(const then_type& post_method)
	{
		_post_method = post_method;
	}
	
protected:
	task() { ; }
	
protected:
	func _method;
	promise<R> _result;
	then_type _post_method;
};

template <>
class task<void> : public Poco::Runnable
{
public:
	using func = std::function<void()>;
	using return_type = void;
	using then_type = std::function<void()>;
	
	task(const func& method)
		: _method(method)
	{
		
	}
	
	~task()
	{
		
	}

	task(const task& te) = delete;
	task& operator = (const task& te) = delete;

	void wait()
	{
		get_future()->get();
	}
	
	std::shared_ptr< future<void> > get_future() const
	{
		return _result.get_future();
	}
	
	void run() override
	{
		{
			try {
				_method();
				if (_post_method)
				{
					_post_method();
				}
			}
			catch (...) {
				try {
					_result.set_exception(std::current_exception());
				}
				catch (...) { ; }
			}
		}
		_result.signal();
	}

	void then(const then_type& post_method)
	{
		_post_method = post_method;
	}
	
protected:
	task() { ; }
	
protected:
	func _method;
	promise<void> _result;
	then_type _post_method;
};

template <typename Function>
shared_task<Function> run(Function&& f)
{
	auto job = std::make_shared<task< typename std::result_of<Function()>::type > >( std::bind( std::forward<Function>(f) ) );
	Poco::ThreadPool::defaultPool().start( *job );
	return job;
}

template <typename Function, typename ... Args>
shared_task<Function> run(Function&& f, Args&& ... args)
{
	auto job = std::make_shared<task< typename std::result_of<Function()>::type > >( std::bind( std::forward<Function>(f), std::forward<Args>(args)... ) );
	Poco::ThreadPool::defaultPool().start( *job );
	return job;
}

template <typename Function>
void _parallel(std::vector<shared_task<Function> >& vf, Function&& f)
{
	vf.emplace_back( asyncply::run(std::forward<Function>(f)) );
}

template <typename Function, typename ... Functions>
void _parallel(std::vector<shared_task<Function> >& vf, Function&& f, Functions&& ... fs)
{
	vf.emplace_back( asyncply::run(std::forward<Function>(f)) );
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
}

template <typename Function, typename ... Functions>
std::vector<shared_task<Function> > parallel(Function&& f, Functions&& ... fs)
{
	std::vector<shared_task<Function> > vf;
	vf.emplace_back( asyncply::run(std::forward<Function>(f)) );
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
	return vf;
}

template <typename Function>
std::vector<shared_task<Function> > parallel(Function&& f)
{
	std::vector<shared_task<Function> > vf;
	vf.emplace_back( asyncply::run(std::forward<Function>(f)) );
	return vf;
}

template <typename FunctionPrev, typename Function>
std::function<void(const typename task_t<FunctionPrev>::return_type& data)> 
_sequence(Function&& f)
{
	// last execution
	return [=](const typename task_t<FunctionPrev>::return_type& data) {
		shared_task<Function> job = asyncply::run(
			std::bind( std::forward<Function>(f) )
		);
		job->wait();
	};
}

template <typename FunctionPrev, typename Function, typename ... Functions>
std::function<void(const typename task_t<FunctionPrev>::return_type& data)> 
_sequence(Function&& f, Functions&& ... fs)
{
	return [=](const typename task_t<FunctionPrev>::return_type& data, Functions&& ... ffs) {
		shared_task<Function> job = asyncply::run(
			std::bind( std::forward<Function>(f) )
		);
		job->then(asyncply::_sequence( std::forward<Function>(f), std::forward<Functions>(ffs)... ));
		job->wait();
	};
}

template <typename ... Functions>
void sequence(Functions&& ... fs)
{
	auto job = asyncply::run(
		[](Functions&& ... ffs) {
			struct dummy_functor
			{
				int operator()()
				{
					return 0;
				}
			};
			auto seq = asyncply::_sequence(dummy_functor(), std::forward<Functions>(ffs)...);
			seq(0, std::forward<Functions>(ffs)... );
		}, 
		std::forward<Functions>(fs)...
	);
	job->wait();
}

/*
using Leaf = std::function<void()>;
using CompositeLeaf = std::function<Leaf(const Leaf&)>;

Leaf operator >>= (const CompositeLeaf& a, const Leaf& b)
{
	return a(b);
}

CompositeLeaf repeat(int n)
{
	return [=](const Leaf& f)
	{
		return [&]()
		{
			for(int cont = 0; cont < n; ++cont)
			{
				f();
			}
		};
	};
}
*/

/*
		using call_type = boost::coroutines::asymmetric_coroutine<void>::pull_type;
		using yield_type = boost::coroutines::asymmetric_coroutine<void>::push_type;

		run( sync_function, parm1, parm2
		).step(
			[](yield_type& yield, return_type& ret) 
			{
				comprar_huevos();
				yield();
				cocinar_huevos();
				yield();
				ret(100);
			}
		).then(
			[&](int finish_result) {
				
			}
		);
*/

template <typename T>
class commands_queue
{
public:
	using command = std::function<void(T&)>;
	
	explicit commands_queue()
		: busy(false)
	{ ; }
	~commands_queue() { ; }
	
	commands_queue(const commands_queue&) = delete;
	commands_queue& operator=(const commands_queue&) = delete;
	
	template <typename R>
	void connect(R& follower)
	{
#ifdef _DEBUG
#define debug_cast dynamic_cast
#else
#define debug_cast static_cast
#endif
		_conns.emplace_back(_commands.connect(std::bind(&commands_queue::planificator, this, std::ref(debug_cast<T&>(follower)), std::placeholders::_1)));
	}
	
	void planificator(T& self, const command& cmd)
	{
		if(_job)
		{
			_job->get_future()->get();
		}
		_job = asyncply::run( std::bind(cmd, std::ref(self)) );
		_job->then(
			[this]()
			{
				this->busy = false;
			}
		);
	}

	inline void call(const command& cmd, fes::deltatime milli = fes::deltatime(0), int priority = 0)
	{
		_commands(priority, milli, cmd);
	}
	
	void update()
	{
		if (!busy)
		{
			// dispatch return true if some is dispatched
			busy = _commands.dispatch_one();
		}
	}
	
public:
	std::atomic<bool> busy;
protected:
	std::vector<fes::shared_connection<command> > _conns;
	fes::async_delay<command> _commands;
	std::shared_ptr< task<void> > _job;
};

template <typename T>
class animations_queue
{
public:
	using command = std::function<void(T&, float)>;
	using animation = std::tuple<command, float, float, float>;
	
	explicit animations_queue()
		: busy(false)
	{ ; }
	~animations_queue() { ; }
	
	animations_queue(const animations_queue&) = delete;
	animations_queue& operator=(const animations_queue&) = delete;
	
	template <typename R>
	void connect(R& follower)
	{
#ifdef _DEBUG
#define debug_cast dynamic_cast
#else
#define debug_cast static_cast
#endif
		_conns.emplace_back(_commands.connect(std::bind(&animations_queue::planificator, this, std::ref(debug_cast<T&>(follower)), std::placeholders::_1)));
	}
	
	void planificator(T& self, const animation& anim)
	{
		auto& cmd = std::get<0>(anim);
		float start = std::get<1>(anim);
		float end = std::get<2>(anim);
		float totaltime = std::get<3>(anim);

		if(_job)
		{
			_job->get_future()->get();
		}
		_job = run(
			[start, end, totaltime, &cmd, &self]()
			{
				const int FPS = 60;
				const int FRAMETIME = 1000 / FPS;
				//float marktime = fes::high_resolution_clock();
				float marktime = 0.0f; // TODO:
				int sleeptime = 0;
				float timeline = 0.0f;
				while (true)
				{
					auto d = timeline / totaltime;
					clamp( d, 0.0f, 1.0f ); // really need clamp ?
					
					auto interp = smoothstep(start, end, d);
					cmd(self, interp);
					
					timeline += FRAMETIME;
					if (timeline >= totaltime)
					{
						break;
					}
					
					marktime += FRAMETIME;
					//sleeptime = static_cast<int>(marktime - float(fes::high_resolution_clock()));
					sleeptime = static_cast<int>(marktime - 0.0f);
					if (sleeptime >= 0) {
						std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));
					}
				}
			}
		);
		_job->then(
			[this]()
			{
				this->busy = false;
			}
		);
	}

	inline void call(const command& cmd, float start, float end, float totaltime, fes::deltatime milli = fes::deltatime(0), int priority = 0)
	{
		_commands(priority, milli, std::make_tuple(cmd, start, end, totaltime));
	}
	
	void update()
	{
		if (!busy)
		{
			// dispatch return true if some is dispatched
			busy = _commands.dispatch_one();
		}
	}
	
public:
	std::atomic<bool> busy;
protected:
	std::vector<fes::shared_connection<animation> > _conns;
	fes::async_delay<animation> _commands;
	std::shared_ptr< task< void > > _job;
};

template <typename SELF, typename FOLLOWERS = SELF>
class talker
{
public:
	using command_others = typename commands_queue<FOLLOWERS>::command;
	using command_me = typename commands_queue<SELF>::command;
	
	explicit talker()
	{
#ifdef _DEBUG
#define debug_cast dynamic_cast
#else
#define debug_cast static_cast
#endif
		_planner_me.template connect<SELF>(debug_cast<SELF&>(*this));
	}
	
	virtual ~talker()
	{
		
	}
	
	talker(const talker&) = delete;
	talker& operator=(const talker&) = delete;
	
	void connect(FOLLOWERS& talker)
	{
		_planner_others.connect(talker);
	}
	
	inline void call_others(const command_others& command, fes::deltatime milli = fes::deltatime(0), int priority = 0)
	{
		_planner_others.call(command, milli, priority);
	}

	inline void call_me(const command_me& command, fes::deltatime milli = fes::deltatime(0), int priority = 0)
	{
		_planner_me.call(command, milli, priority);
	}
	
	void update()
	{
		_planner_others.update();
		_planner_me.update();
	}
	
	inline void sleep(int milli)
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(milli) );
	}
	
protected:
	commands_queue<FOLLOWERS> _planner_others;
	commands_queue<SELF> _planner_me;
};

} // end namespace

#endif // _SCHEDULER_ADVANCED_SIMPLE_
