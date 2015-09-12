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
#include <exception>
#include <fes/h/fes.h>
#include <animator/h/interpolation.h>
#include <Poco/Thread.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>

namespace asyncply
{

template <typename R>
class task;

template <typename Function>
using task_of_functor = task<typename std::result_of<Function()>::type>;

template <typename Function>
using shared_task = std::shared_ptr<task_of_functor<Function>>;

template <typename Function>
shared_task<Function> run(Function&& f);

template <typename Function, typename FunctionPost>
shared_task<Function> run(Function&& f, FunctionPost&& fp);

template <typename R>
class future
{
public:
	future(Poco::Semaphore& sem)
		: _semaphore(sem)
		, _ready(false)
		, _has_exception(false)
	{
	}

	~future() noexcept {}

	R& get()
	{
		Poco::Mutex::ScopedLock lock(_lock);
		while (!_ready)
		{
			_semaphore.wait();
			if (_has_exception && _exception)
			{
				std::rethrow_exception(_exception);
			}
			_ready = true;
		}
		return _value;
	}

	void set_value(const R& value) noexcept { _value = value; }

	void set_value(R&& value) noexcept { _value = std::forward<R>(value); }

	void set_exception(std::exception_ptr p) noexcept
	{
	   	_exception = p;
		_has_exception = true;
	}

protected:
	Poco::Mutex _lock;
	Poco::Semaphore& _semaphore;
	std::atomic<bool> _ready;
	std::atomic<bool> _has_exception;
	std::exception_ptr _exception;
	R _value;
};

template <>
class future<void>
{
public:
	future(Poco::Semaphore& sem)
		: _semaphore(sem)
		, _ready(false)
	{
	}

	~future() noexcept {}

	void get()
	{
		Poco::Mutex::ScopedLock lock(_lock);
		while (!_ready)
		{
			_semaphore.wait();
			if (_exception)
			{
				std::rethrow_exception(_exception);
			}
			_ready = true;
		}
	}

	void set_exception(std::exception_ptr p) noexcept { _exception = p; }

protected:
	Poco::Mutex _lock;
	Poco::Semaphore& _semaphore;
	std::exception_ptr _exception;
	std::atomic<bool> _ready;
};

template <typename R>
class promise
{
public:
	promise()
		: _future(std::make_shared<future<R>>(_semaphore))
		, _semaphore(0, 1)
	{
	}

	~promise() noexcept {}

	std::shared_ptr<future<R>> get_future() const { return _future; }

	void set_value(const R& value) noexcept { _future->set_value(value); }
	R& get_value() { return _future->get(); }

	void set_value(R&& value) noexcept { _future->set_value(std::forward<R>(value)); }

	void set_exception(std::exception_ptr p) noexcept { _future->set_exception(p); }

	void signal() { _semaphore.set(); }

protected:
	std::shared_ptr<future<R>> _future;
	Poco::Semaphore _semaphore;
};

template <>
class promise<void>
{
public:
	promise()
		: _future(std::make_shared<future<void>>(_semaphore))
		, _semaphore(0, 1)
	{
	}

	~promise() noexcept {}

	std::shared_ptr<future<void>> get_future() const { return _future; }

	void set_exception(std::exception_ptr p) noexcept { _future->set_exception(p); }

	void signal() { _semaphore.set(); }

protected:
	std::shared_ptr<future<void>> _future;
	Poco::Semaphore _semaphore;
};

template <typename R>
class task : public Poco::Runnable
{
public:
	using func = std::function<R()>;
	using return_type = R;
	using post_type = std::function<return_type(const return_type&)>;

	task(const func& method)
		: _method(method)
		, _post_method(nullptr)
		, _has_post(false)
	{
	}

	task(const func& method, const post_type& post_method)
		: _method(method)
		, _post_method(post_method)
		, _has_post(true)
	{
	}

	virtual ~task() { get(); }

	task(const task&) = delete;
	task& operator=(const task&) = delete;

	R& get()
	{
		if (_post_method)
			return _result_post.get_future()->get();
		else
			return _result.get_future()->get();
	}

	void run() override
	{
		try
		{
			_result.set_value(_method());
		}
		catch (...)
		{
			try
			{
				_result.set_exception(std::current_exception());
			}
			catch (...)
			{
				;
			}
		}
		_result.signal();

		if (_has_post && _post_method)
		{
			auto post_task = asyncply::run(std::bind(_post_method, std::cref(_result.get_value())));
			try
			{
				_result_post.set_value(post_task->get());
			}
			catch (...)
			{
				try
				{
					_result_post.set_exception(std::current_exception());
				}
				catch (...)
				{
					;
				}
			}
			_result_post.signal();
		}
	}

protected:
	task() { ; }

protected:
	func _method;
	promise<R> _result;
	promise<R> _result_post;
	post_type _post_method;
	bool _has_post;
};

template <>
class task<void> : public Poco::Runnable
{
public:
	using func = std::function<void()>;
	using return_type = void;
	using post_type = std::function<return_type()>;

	task(const func& method)
		: _method(method)
		, _has_post(false)
	{
	}

	task(const func& method, const post_type& post_method)
		: _method(method)
		, _post_method(post_method)
		, _has_post(true)
	{
	}

	virtual ~task() noexcept { get(); }

	task(const task& te) = delete;
	task& operator=(const task& te) = delete;

	void get()
	{
		if (_post_method)
			_result_post.get_future()->get();
		else
			_result.get_future()->get();
	}

	void run() override
	{
		try
		{
			_method();
		}
		catch (...)
		{
			try
			{
				_result.set_exception(std::current_exception());
			}
			catch (...)
			{
				;
			}
		}
		_result.signal();

		if (_has_post && _post_method)
		{
			auto post_task = asyncply::run(std::bind(_post_method));
			try
			{
				post_task->get();
			}
			catch (...)
			{
				try
				{
					_result_post.set_exception(std::current_exception());
				}
				catch (...)
				{
					;
				}
			}
			_result_post.signal();
		}
	}

protected:
	task() { ; }

protected:
	func _method;
	promise<void> _result;
	promise<void> _result_post;
	post_type _post_method;
	bool _has_post;
};

/////////////////////////////////////// RUN //////////////////////////////////

template <typename Function>
shared_task<Function> run(Function&& f)
{
	auto job = std::make_shared<task_of_functor<Function>>(std::forward<Function>(f));
	Poco::ThreadPool::defaultPool().start(*job);
	return job;
}

template <typename Function, typename FunctionPost>
shared_task<Function> run(Function&& f, FunctionPost&& fp)
{
	auto job = std::make_shared<task_of_functor<Function>>(
		std::forward<Function>(f), std::forward<FunctionPost>(fp));
	Poco::ThreadPool::defaultPool().start(*job);
	return job;
}

////////////////////////// PARALLEL ///////////////////////////////////////////

template <typename Function>
void _parallel(std::vector<shared_task<Function>>& vf, Function&& f)
{
	vf.push_back(asyncply::run(std::forward<Function>(f)));
}

template <typename Function, typename... Functions>
void _parallel(std::vector<shared_task<Function>>& vf, Function&& f, Functions&&... fs)
{
	vf.push_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
}

template <typename Function, typename... Functions>
void parallel(std::vector<shared_task<Function>>& vf, Function&& f, Functions&&... fs)
{
	vf.push_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
}

template <typename Function>
void parallel(std::vector<shared_task<Function>> vf, Function&& f)
{
	vf.push_back(asyncply::run(std::forward<Function>(f)));
}

///////////////////////////// SEQUENCE ////////////////////////////////////////

template <typename Data, typename Function>
std::function<Data(const Data&)> _sequence(Function&& f)
{
	return [&](const Data& data)
	{
		auto job = asyncply::run([&]()
			{
				return f(data);
			});
		return job->get();
	};
}

template <typename Data, typename Function, typename... Functions>
std::function<Data(const Data&)> _sequence(Function&& f, Functions&&... fs)
{
	return [&](const Data& data)
	{
		auto job = asyncply::run(
			[&f, &data]()
			{
				return f(data);
			},
			[&](const Data& d)
			{
				return asyncply::_sequence<Data>(std::forward<Functions>(fs)...)(d);
			});
		return job->get();
	};
}

template <typename Data, typename... Functions>
Data sequence(const Data& data, Functions&&... fs)
{
	auto job = asyncply::run([&]()
		{
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
	{
		;
	}
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
		_conns.emplace_back(_commands.connect(std::bind(&commands_queue::planificator, this,
			std::ref(debug_cast<T&>(follower)), std::placeholders::_1)));
	}

	void planificator(T& self, const command& cmd)
	{
		if (_job)
		{
			_job->get();
		}
		_job = asyncply::run(std::bind(cmd, std::ref(self)), [this]()
			{
				this->busy = false;
			});
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
	std::vector<fes::shared_connection<command>> _conns;
	fes::async_delay<command> _commands;
	std::shared_ptr<task<void>> _job;
};

template <typename T>
class animations_queue
{
public:
	using command = std::function<void(T&, float)>;
	using animation = std::tuple<command, float, float, float>;

	explicit animations_queue()
		: busy(false)
	{
		;
	}
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
		_conns.emplace_back(_commands.connect(std::bind(&animations_queue::planificator, this,
			std::ref(debug_cast<T&>(follower)), std::placeholders::_1)));
	}

	void planificator(T& self, const animation& anim)
	{
		auto& cmd = std::get<0>(anim);
		float start = std::get<1>(anim);
		float end = std::get<2>(anim);
		float totaltime = std::get<3>(anim);

		if (_job)
		{
			_job->get();
		}
		_job = run(
			[start, end, totaltime, &cmd, &self]()
			{
				const int FPS = 60;
				const int FRAMETIME = 1000 / FPS;
				// float marktime = fes::high_resolution_clock();
				float marktime = 0.0f;  // TODO:
				int sleeptime = 0;
				float timeline = 0.0f;
				while (true)
				{
					auto d = timeline / totaltime;
					clamp(d, 0.0f, 1.0f);  // really need clamp ?

					auto interp = smoothstep(start, end, d);
					cmd(self, interp);

					timeline += FRAMETIME;
					if (timeline >= totaltime)
					{
						break;
					}

					marktime += FRAMETIME;
					// sleeptime = static_cast<int>(marktime - float(fes::high_resolution_clock()));
					sleeptime = static_cast<int>(marktime - 0.0f);
					if (sleeptime >= 0)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));
					}
				}
			},
			[this]()
			{
				this->busy = false;
			});
	}

	inline void call(const command& cmd, float start, float end, float totaltime,
		fes::deltatime milli = fes::deltatime(0), int priority = 0)
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
	std::vector<fes::shared_connection<animation>> _conns;
	fes::async_delay<animation> _commands;
	std::shared_ptr<task<void>> _job;
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

	virtual ~talker() {}

	talker(const talker&) = delete;
	talker& operator=(const talker&) = delete;

	void connect(FOLLOWERS& talker) { _planner_others.connect(talker); }

	inline void call_others(
		const command_others& command, fes::deltatime milli = fes::deltatime(0), int priority = 0)
	{
		_planner_others.call(command, milli, priority);
	}

	inline void call_me(
		const command_me& command, fes::deltatime milli = fes::deltatime(0), int priority = 0)
	{
		_planner_me.call(command, milli, priority);
	}

	void update()
	{
		_planner_others.update();
		_planner_me.update();
	}

	inline void sleep(int milli) { std::this_thread::sleep_for(std::chrono::milliseconds(milli)); }

protected:
	commands_queue<FOLLOWERS> _planner_others;
	commands_queue<SELF> _planner_me;
};

}  // end namespace

#endif  // _SCHEDULER_ADVANCED_SIMPLE_
