// design-patterns-cpp14 by Ricardo Marmolejo Garc�a is licensed under a Creative Commons
// Reconocimiento 4.0 Internacional License.
// http://creativecommons.org/licenses/by/4.0/
//
#ifndef _INTERNAL_CONNECTION_H_
#define _INTERNAL_CONNECTION_H_

#include <atomic>
#include <functional>
#include <fes/h/method.h>

namespace fes {

template <typename... Args>
class internal_connection
{
public:
	using deleter_t = std::function<void(methods_t<Args...>&)>;

	internal_connection(methods_t<Args...>& registered, const deleter_t& deleter)
		: _deleter(deleter)
		, _connected(true)
		, _registered(registered)
	{
		;
	}

	internal_connection(const internal_connection& other) = delete;
	internal_connection& operator=(const internal_connection& other) = delete;

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

}

#endif

