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

	template <typename FUNCTION>
	method(FUNCTION&& m)
		: _method(std::forward<FUNCTION>(m))
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

	template <typename ... PARMS>
	void operator()(PARMS&&... data) const
	{
		_method(std::forward<PARMS>(data)...);
	}

protected:
	function _method;
};

template <typename... Args>
using methods_t = std::list<method<Args...>>;

}

#endif
