#ifndef _PROMISE_H_
#define _PROMISE_H_

#include <memory>
#include <exception>
#include <Poco/Semaphore.h>
#include <asyncply/h/future.h>

namespace asyncply {

template <typename R>
class promise
{
public:
	promise()
		: _future(std::make_shared<asyncply::future<R>>(_semaphore))
		, _semaphore(0, 1)
	{
	}

	~promise() {}

	std::shared_ptr<asyncply::future<R>> get_future() const { return _future; }

	void set_value(const R& value) { _future->set_value(value); }
	R& get_value() { return _future->get(); }

	void set_value(R&& value) { _future->set_value(std::forward<R>(value)); }

	void set_exception(std::exception_ptr p) { _future->set_exception(p); }

	void signal() { _semaphore.set(); }

protected:
	std::shared_ptr<asyncply::future<R>> _future;
	Poco::Semaphore _semaphore;
};

template <>
class promise<void>
{
public:
	promise()
		: _future(std::make_shared<asyncply::future<void>>(_semaphore))
		, _semaphore(0, 1)
	{
	}

	~promise() {}

	std::shared_ptr<asyncply::future<void>> get_future() const { return _future; }

	void set_exception(std::exception_ptr p) { _future->set_exception(p); }

	void signal() { _semaphore.set(); }

protected:
	std::shared_ptr<asyncply::future<void>> _future;
	Poco::Semaphore _semaphore;
};

}

#endif

