// Ricardo Marmolejo Garcia
// 15-04-2015
#ifndef _SCHEDULER_ADVANCED_SIMPLE_
#define _SCHEDULER_ADVANCED_SIMPLE_

// linux
#ifndef _WIN32
#include <semaphore.h>
#else
#include <windows.h>
#endif

// std
#include <functional>
#include <future>
#include <atomic>
#include <mutex>
#include <fast-event-system/fes.h>
#include <animator/interpolation.h>
#include <Poco/Thread.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/Condition.h>
#include <Poco/Mutex.h>

namespace sas {

template <typename R>
class task;

template< typename Function>
std::shared_ptr< task<typename std::result_of<Function()>::type> > await(Function&& f);

template <typename R>
class future
{
public:
	future(Poco::Condition& compliment)
		: _compliment( compliment )
		, _ready(false)
	{
		
	}

	~future()
	{
		//auto r = get();
	}
	
	R& get()
	{
		if (!_ready)
		{
			_ready = true;
			_compliment.wait(_mutex);
			if (_exception)
			{
				std::rethrow_exception(_exception);
			}
		}
		return _value;
	}

	R& get(long milliseconds)
	{
		if (!_ready)
		{
			_ready = true;
			_compliment.wait(_mutex, milliseconds);
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
	Poco::Condition& _compliment;
	Poco::Mutex _mutex;
	R _value;
	std::exception_ptr _exception;
	std::atomic<bool> _ready;
};

template <>
class future<void>
{
public:
	future(Poco::Condition& compliment)
		: _compliment( compliment )
		, _ready(false)
	{
		
	}

	~future()
	{
		//get();
	}
	
	void get()
	{
		if (!_ready)
		{
			_ready = true;
			_compliment.wait(_mutex);
			if (_exception)
			{
				std::rethrow_exception(_exception);
			}
		}
	}

	void get(long milliseconds)
	{
		if (!_ready)
		{
			_ready = true;
			_compliment.wait(_mutex, milliseconds);
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
	Poco::Condition& _compliment;
	Poco::Mutex _mutex;
	std::exception_ptr _exception;
	std::atomic<bool> _ready;
};

template <typename R>
class promise
{
public:
	promise()
		: _future( std::make_shared<future<R> >(_compliment) )
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
		_compliment.signal();
	}

protected:
	std::shared_ptr< future<R> > _future;
	Poco::Condition _compliment;
};

template <>
class promise<void>
{
public:
	promise()
		: _future( std::make_shared<future<void> >( _compliment ) )
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
		_compliment.signal();
	}

protected:
	std::shared_ptr< future<void> > _future;
	Poco::Condition _compliment;
};

template <typename R>
class task : public Poco::Runnable
{
public:
	using func = std::function<R()>;
	using then_type = std::function<void(const R&)>;

	task(const func& method)
		: _method(method)
	{
		
	}
	
	~task()
	{
		get_future()->get();
	}

	task(const task& te) = delete;
	task& operator = (const task& te) = delete;
	
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
	using then_type = std::function<void()>;
	
	task(const func& method)
		: _method(method)
	{
		
	}
	
	~task()
	{
		get_future()->get();
	}

	task(const task& te) = delete;
	task& operator = (const task& te) = delete;
	
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

template< typename Function>
std::shared_ptr< task<typename std::result_of<Function()>::type> > await(Function&& f)
{
	auto job = std::make_shared<task< typename std::result_of<Function()>::type > >(
		[&]() -> typename std::result_of<Function()>::type
		{
			return f();
		}
	);
	Poco::ThreadPool::defaultPool().start( *job );
	return job;
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

		await( sync_function, parm1, parm2
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

class fast_event_system_API processor
{
public:
	processor(size_t threads)
	{
		Poco::ThreadPool::defaultPool().stopAll();
		Poco::ThreadPool::defaultPool().addCapacity(threads);
	}

	// the destructor joins all threads
	~processor()
	{
		Poco::ThreadPool::defaultPool().joinAll();
	}

	processor(const processor&) = delete;
	processor& operator=(const processor&) = delete;
	
	// add work
	template< typename Function>
	std::shared_ptr< task<int> > enqueue(Function&& func)
	{

		auto job = await(
			[&]() -> int
			{
				func();
				return 1;
			}
		);
		_tasks.push_back(job);
		return job;
	}
	
private:
	// synchronization
	std::vector< std::shared_ptr< task<int> > > _tasks;
};

template <typename T>
class commands_queue
{
public:
	using command = std::function<void(T&)>;
	
	explicit commands_queue()
		: busy(false)
	{ ; }
	explicit commands_queue(const std::shared_ptr<sas::processor>& pool)
		: busy(false)
		, _processor(pool)
	{ ; }
	~commands_queue() { ; }
	
	commands_queue(const commands_queue&) = delete;
	commands_queue& operator=(const commands_queue&) = delete;
	
	template <typename R>
	void add_follower(R& follower)
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
		_job = await(
			[&]()
			{
				cmd(self);
			}
		);
		_job->then(
			[this]()
			{
				this->busy = false;
				_job = nullptr;
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
	
	// inject depends
	void set_processor(const std::shared_ptr<sas::processor>& pool)
	{
		_processor = pool;
	}
	
public:
	std::atomic<bool> busy;
protected:
	std::vector<fes::shared_connection<command> > _conns;
	fes::async_delay<command> _commands;
	std::shared_ptr<sas::processor> _processor;
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
	explicit animations_queue(const std::shared_ptr<sas::processor>& pool)
		: busy(false)
		, _processor(pool)
	{ ; }
	~animations_queue() { ; }
	
	animations_queue(const animations_queue&) = delete;
	animations_queue& operator=(const animations_queue&) = delete;
	
	template <typename R>
	void add_follower(R& follower)
	{
		_conns.emplace_back(_commands.connect(std::bind(&animations_queue::planificator, this, std::ref(dynamic_cast<T&>(follower)), std::placeholders::_1)));
	}
	
	void planificator(T& self, const animation& anim)
	{
		auto& cmd = std::get<0>(anim);
		float start = std::get<1>(anim);
		float end = std::get<2>(anim);
		float totaltime = std::get<3>(anim);

		_job = await(
			[this, start, end, totaltime, cmd, &self]()
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

				this->busy = false;
				return 1;
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
	
	// inject depends
	void set_processor(const std::shared_ptr<sas::processor>& pool)
	{
		_processor = pool;
	}
	
public:
	std::atomic<bool> busy;
protected:
	std::vector<fes::shared_connection<animation> > _conns;
	fes::async_delay<animation> _commands;
	std::shared_ptr<sas::processor> _processor;
	std::shared_ptr< task<int> > _job;
};

template <typename SELF, typename FOLLOWERS = SELF>
class talker
{
public:
	using command_others = typename commands_queue<FOLLOWERS>::command;
	using command_me = typename commands_queue<SELF>::command;
	
	explicit talker()
	{
		//_planner_me.add_follower<SELF>(*this);
	}
	
	virtual ~talker()
	{
		
	}
	
	talker(const talker&) = delete;
	talker& operator=(const talker&) = delete;
	
	void add_follower(FOLLOWERS& talker)
	{
		_planner_others.add_follower<FOLLOWERS>(talker);
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
	
	// inject depends
	void set_processor(const std::shared_ptr<sas::processor>& pool)
	{
		_planner_others.set_processor(pool);
		_planner_me.set_processor(pool);
	}
	
protected:
	commands_queue<FOLLOWERS> _planner_others;
	commands_queue<SELF> _planner_me;
};

} // end namespace

#endif // _SCHEDULER_ADVANCED_SIMPLE_
