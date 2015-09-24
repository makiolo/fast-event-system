#ifndef _FUTURE_H_
#define _FUTURE_H_

namespace asyncply {

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

	~future() {}

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

	void set_value(const R& value) { _value = value; }

	void set_value(R&& value) { _value = std::forward<R>(value); }

	void set_exception(std::exception_ptr p)
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
	// atomic ?
	R _value;
};

template <>
class future<void>
{
public:
	future(Poco::Semaphore& sem)
        	: _lock()
		, _semaphore(sem)
        	, _exception()
		, _ready(false)
	{
	}

	~future() {}

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

	void set_exception(std::exception_ptr p) { _exception = p; }

protected:
	Poco::Mutex _lock;
	Poco::Semaphore& _semaphore;
	std::exception_ptr _exception;
	std::atomic<bool> _ready;
};

}

#endif

