// Modified from work of Aldrin D'Souza.

#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include <vector>
#include <memory>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/coroutine/coroutine.hpp>

// namespace asyncply {

template <typename T>
class basic_pipeline
{
private:
	using coro = boost::coroutines::coroutine<const T&()>;
	using coroptr = std::unique_ptr<coro>;

public:
	using in = coro;
	using out = typename coro::caller_type;
	using link = boost::function<void(in&, out&)>;

	basic_pipeline(const std::vector<link>& links)
	{
		std::vector<coroptr> coros;
		coros.emplace_back( coroptr(new coro( [](out&){} )) );
		for(auto& lnk : links)
		{
			coros.emplace_back(coroptr(new coro(boost::bind(lnk, boost::ref(*coros.back().get()), _1))));
		}
	}
};

template <typename T>
class flow_pipeline
{
private:
	using coro = boost::coroutines::coroutine<const T&()>;
	using coroptr = std::unique_ptr<coro>;

public:
	using in = coro;
	using out = typename coro::caller_type;
	using link = boost::function<void(in&, out&)>;

	flow_pipeline(const std::vector<link>& links)
	{
		std::vector<coroptr> coros;
		coros.emplace_back( coroptr(new coro( [](out&){} )) );
		for(auto& lnk : links)
		{
			coros.emplace_back( coroptr(new coro(boost::bind(lnk, boost::ref(*coros.back().get()), _1))) );
		}
	}
};

// }

#endif

