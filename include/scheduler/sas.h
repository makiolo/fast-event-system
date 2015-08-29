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

template <typename R> class task;
template< typename Function> using task_of_functor = task<typename std::result_of< Function() >::type>;
template< typename Function> using shared_task = std::shared_ptr< task_of_functor<Function> >;

template <typename R>
class future
{
public:
	future(Poco::Semaphore& mutex)
		: _semaphore(mutex)
		, _ready(0)
	{
		
	}

	~future() noexcept
	{

	}
	
	const R& get() const
	{
		Poco::Mutex::ScopedLock lock(_zone);
		while (_ready <= 0)
		{
			_semaphore.wait();
			if (_exception)
			{
				std::rethrow_exception(_exception);
			}
			++_ready;
		}
		return _value;
	}

	const R& get_post() const
	{
		Poco::Mutex::ScopedLock lock(_zone);
		while (_ready <= 1)
		{
			_semaphore.wait();
			if (_exception)
			{
				std::rethrow_exception(_exception);
			}
			++_ready;
		}
		return _value;
	}
	
	void set_value(const R& value) noexcept
	{
		_value = value;
	}

	void set_value(R&& value) noexcept
	{
		_value = std::forward<R>(value);
	}

	void set_exception(std::exception_ptr p) noexcept
	{
		_exception = p;
	}

protected:
	mutable Poco::Mutex _zone;
	Poco::Semaphore& _semaphore;
	R _value;
	std::exception_ptr _exception;
	mutable std::atomic<int> _ready;
};

template <>
class future<void>
{
public:
	future(Poco::Semaphore& mutex)
		: _semaphore(mutex)
		, _ready(0)
	{
		
	}

	~future() noexcept
	{

	}
	
	void get() const
	{
		Poco::Mutex::ScopedLock lock(_zone);
		while (_ready <= 0)
		{
			_semaphore.wait();
			if (_exception)
			{
				std::rethrow_exception(_exception);
			}
			++_ready;
		}
	}

	void get_post() const
	{
		Poco::Mutex::ScopedLock lock(_zone);
		while (_ready <= 1)
		{
			_semaphore.wait();
			if (_exception)
			{
				std::rethrow_exception(_exception);
			}
			++_ready;
		}
	}
	
	void set_exception(std::exception_ptr p) noexcept
	{
		_exception = p;
	}
	
protected:
	mutable Poco::Mutex _zone;
	Poco::Semaphore& _semaphore;
	std::exception_ptr _exception;
	mutable std::atomic<int> _ready;
};

template <typename R>
class promise
{
public:
	promise()
		: _future( std::make_shared<future<R> >(_semaphore) )
		, _semaphore(0, 2)
	{
		
	}
	
	~promise() noexcept
	{
		
	}

	std::shared_ptr< future<R> > get_future() const
	{
		return _future;
	}

	void set_value(const R& value) noexcept
	{
		_future->set_value(value);
	}

	void set_value(R&& value) noexcept
	{
		_future->set_value(std::forward<R>(value));
	}

	void set_exception(std::exception_ptr p) noexcept
	{
		_future->set_exception(p);
	}

	void signal() noexcept
	{
		_semaphore.set();
	}

protected:
	std::shared_ptr< future<R> > _future;
	Poco::Semaphore _semaphore;
};

template <>
class promise<void>
{
public:
	promise()
		: _future( std::make_shared<future<void> >( _semaphore ) )
		, _semaphore(0, 2)
	{
		
	}
	
	~promise() noexcept
	{

	}
	
	std::shared_ptr< future<void> > get_future() const
	{
		return _future;
	}
	
	void set_exception(std::exception_ptr p) noexcept
	{
		_future->set_exception(p);
	}
	
	void signal() noexcept
	{
		_semaphore.set();
	}
	
protected:
	std::shared_ptr< future<void> > _future;
	Poco::Semaphore _semaphore;
};

template <typename R>
class task : public Poco::Runnable
{
public:
	using func = std::function<R()>;
	using return_type = R;
	using post_type = std::function<return_type(const return_type&)>;

	task(const func& method) : _method( method )
	{
		
	}
	
	virtual ~task()
	{
		get_post();
	}

	task(const task&) = delete;
	task& operator = (const task&) = delete;

	const R& get() const
	{
		return get_future()->get();
	}

	const R& get_post() const
	{
		return get_future()->get_post();
	}
	
	std::shared_ptr< future<R> > get_future() const
	{
		return _result.get_future();
	}

	void run() override
	{
		R r;
		{
			try {
				r = _method();
				_result.set_value(r);
				// riesgo de setear el post demasiado tarde
			}
			catch (...) {
				try {
					_result.set_exception(std::current_exception());
				}
				catch (...) { ; }
			}
		}
		_result.signal();

		if (_post_method)
		{
			try {
				r = _post_method(r);
				_result.set_value(r);
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
	
	void post(const post_type& post_method) noexcept
	{
		_post_method = post_method;
	}
	
protected:
	task() { ; }
	
protected:
	func _method;
	promise<R> _result;
	post_type _post_method;
};

template <>
class task<void> : public Poco::Runnable
{
public:
	using func = std::function<void()>;
	using return_type = void;
	using post_type = std::function<return_type()>;
	
	task(const func& method) : _method( method )
	{
		
	}
	
	virtual ~task() noexcept
	{
		get_post();
	}

	task(const task& te) = delete;
	task& operator = (const task& te) = delete;

	void get() const
	{
		get_future()->get();
	}

	void get_post() const
	{
		get_future()->get_post();
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
			}
			catch (...) {
				try {
					_result.set_exception(std::current_exception());
				}
				catch (...) { ; }
			}
		}
		_result.signal();

		if (_post_method)
		{
			try {
				_post_method();
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

	void post(const post_type& post_method) noexcept
	{
		_post_method = post_method;
	}
	
protected:
	task() { ; }
	
protected:
	func _method;
	promise<void> _result;
	post_type _post_method;
};

/////////////////////////////////////// RUN //////////////////////////////////

template <typename Function>
shared_task<Function> run(Function&& f)
{
	auto job = std::make_shared< task_of_functor<Function> >( std::forward<Function>(f) );
	Poco::ThreadPool::defaultPool().start( *job );
	return job;
}

////////////////////////// PARALLEL ///////////////////////////////////////////

template <typename Function>
void _parallel(std::vector<shared_task<Function> >& vf, Function&& f)
{
	vf.push_back( asyncply::run(std::forward<Function>(f)) );
}

template <typename Function, typename ... Functions>
void _parallel(std::vector<shared_task<Function> >& vf, Function&& f, Functions&& ... fs)
{
	vf.push_back( asyncply::run(std::forward<Function>(f)) );
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
}

template <typename Function, typename ... Functions>
std::vector<shared_task<Function> > parallel(Function&& f, Functions&& ... fs)
{
	std::vector<shared_task<Function> > vf;
	vf.push_back( asyncply::run(std::forward<Function>(f)) );
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
	return std::move(vf);
}

template <typename Function>
std::vector<shared_task<Function> > parallel(Function&& f)
{
	std::vector<shared_task<Function> > vf;
	vf.push_back( asyncply::run(std::forward<Function>(f)) );
	return std::move(vf);
}

/////////aaaaaa////////// SEQUENCE ////////////////////////////////////////

template <typename Data, typename Function>
std::function<Data(const Data&)> _sequence(Function&& f)
{
	return [&f](const Data& data) {
		auto job = asyncply::run( [&f, &data]() {
					return f(data);
				} );
		return job->get();
	};
}

template <typename Data, typename Function, typename ... Functions>
std::function<Data(const Data&)> _sequence(Function&& f, Functions&& ... fs)
{
	return [&f, &fs...](const Data& data) {
		auto job = asyncply::run([&f, &data]() {
					return f(data);
				});
		job->post(
				[&fs...](const Data& d){
					return asyncply::_sequence<Data>(std::forward<Functions>(fs)...)(d);
				});
		return job->get_post();
	};
}

template <typename Data, typename ... Functions>
const Data& sequence(const Data& data, Functions&& ... fs)
{
	auto job = asyncply::run([&data, &fs...] () {
				return asyncply::_sequence<Data>(std::forward<Functions>(fs)...)(data);
			});
	return job->get();
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
		).post(
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
		_job->post(
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
		_job->post(
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
