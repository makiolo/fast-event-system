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
	using return_type = void;
	using function = std::function<return_type(const Args&...)>;

	explicit method()
		: _method([](const Args& ...){})
	{
	}
	
	template <typename FUNCTION>
	explicit method(FUNCTION&& m)
		: _method(std::forward<FUNCTION>(m))
	{
	}

	template <typename T, typename ... PARMS>
	explicit method(T* obj, return_type (T::*ptr_func)(const PARMS&...))
		: method(obj, ptr_func, make_int_sequence<sizeof...(PARMS)>{})
	{
	}

	template <typename T, typename ... PARMS, int... Is>
	explicit method(T* obj, return_type (T::*ptr_func)(const PARMS&...), int_sequence<Is...>)
		: method(std::bind(ptr_func, obj, placeholder_template<Is>()...))
	{
	}

	template <typename ... PARMS>
	return_type operator()(PARMS&&... data) const
	{
		_method(std::forward<PARMS>(data)...);
	}
	
	template <typename ... PARMS>
	return_type call_copy(PARMS... data) const
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
