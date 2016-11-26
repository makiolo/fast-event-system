#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <tuple>
#include "clock.h"

namespace fes {

template <typename... Args>
struct message
{
	message(int priority, marktime timestamp, const Args&... data)
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

	message(message&& other) noexcept : _priority(other._priority),
										_timestamp(other._timestamp),
										_data(std::move(other._data))
	{
	}

	~message() {}

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

	int _priority;
	marktime _timestamp;
	std::tuple<Args...> _data;
};

// must complain strict weak order
template <typename... Args>
struct message_comp
{
	bool operator()(const message<Args...>& one, const message<Args...>& other)
	{
		if (one._timestamp < other._timestamp)
			return false;
		else if (one._timestamp > other._timestamp)
			return true;

		if (one._priority < other._priority)
			return true;
		else if (one._priority > other._priority)
			return false;

		return false;
	}
};

}

#endif

