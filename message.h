#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <tuple>
#include "clock.h"

namespace fes {

template <typename... Args>
struct message
{
	template <typename ... ARGS>
	message(int priority, marktime timestamp, ARGS&&... data)
		: _priority(priority)
		, _timestamp(timestamp)
		, _data(std::forward<Args>(data)...)
	{ ; }

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
		// first order by timestamp, after by priority
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
