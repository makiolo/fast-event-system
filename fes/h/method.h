// design-patterns-cpp14 by Ricardo Marmolejo Garc�a is licensed under a Creative Commons
// Reconocimiento 4.0 Internacional License.
// http://creativecommons.org/licenses/by/4.0/
//
#ifndef _METHOD_H_
#define _METHOD_H_

#include <list>
#include <functional>
#include <fes/h/common.h>

namespace fes {

template <typename... Args>
class method
{
public:
	using function = std::function<void(const Args&...)>;

	method(const function& m)
		: _method(m)
	{
	}

	template <typename T>
	method(T* obj, void (T::*ptr_func)(const Args&...))
		: method(obj, ptr_func, make_int_sequence<sizeof...(Args)>{})
	{
	}

	template <typename T, int... Is>
	method(T* obj, void (T::*ptr_func)(const Args&...), int_sequence<Is...>)
		: method(std::bind(ptr_func, obj, placeholder_template<Is>{}...))
	{
	}

	method(const method& other) = delete;
	method& operator=(const method& other) = delete;
	~method() { ; }

	void operator()(const Args&... data) const { _method(data...); }

protected:
	function _method;
};

template <typename... Args>
using methods_t = std::list<method<Args...>>;

}

#endif

