#ifndef _METHOD_H_
#define _METHOD_H_

#include <list>
#include <functional>
#include "metacommon/common.h"

namespace fes {

template <typename... Args>
class method
{
public:
	using function = std::function<void(Args&&...)>;

	method(function&& m)
		: _method(std::move(m))
	{
	}
	
	method(const function& m)
		: _method(m)
	{
	}

	template <typename T>
	method(T* obj, void (T::*ptr_func)(Args&&...))
		: method(obj, ptr_func, make_int_sequence<sizeof...(Args)>{})
	{
	}

	template <typename T, int... Is>
	method(T* obj, void (T::*ptr_func)(Args&&...), int_sequence<Is...>)
		: method(std::bind(ptr_func, obj, placeholder_template<Is>{}...))
	{
	}

	method(const method& other) = delete;
	method& operator=(const method& other) = delete;
	~method() { ; }

	void operator()(Args&&... data) const { _method(std::move(data)...); }

protected:
	function _method;
};

template <typename... Args>
using methods_t = std::list<method<Args...>>;

}

#endif

