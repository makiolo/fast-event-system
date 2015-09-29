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

namespace asyncply {

template <typename YIELD_TYPE>
using coro = boost::coroutines::coroutine<YIELD_TYPE()>;

template <typename YIELD_TYPE>
using yield_type = typename coro<YIELD_TYPE>::caller_type;

template <typename YIELD_TYPE, typename Function>
std::shared_ptr< coro<YIELD_TYPE> > corun(Function&& f)
{
	return std::make_shared<coro<YIELD_TYPE> >(std::forward<Function>(f));
}

// template <typename Function>
// void _pipeline(std::vector<shared_task<Function>>& vf, Function&& f)
// {
// 	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
// }
//
// template <typename Function, typename... Functions>
// void _pipeline(std::vector<shared_task<Function>>& vf, Function&& f, Functions&&... fs)
// {
// 	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
// 	asyncply::_pipeline(vf, std::forward<Functions>(fs)...);
// }

// template <  typename Function,
// 			typename... Functions,
// 			class = typename std::enable_if<
// 				(!std::is_arithmetic<typename std::result_of<Function()>::type>::value) &&
// 				(!std::is_same<typename std::result_of<Function()>::type, bool>::value) &&
// 				(!std::is_void<typename std::result_of<Function()>::type>::value)
// 			>::type
// 	>
// std::vector<typename std::result_of<Function()>::type> pipeline(Function&& f, Functions&&... fs)
// {
// 	using ret_t = typename std::result_of<Function()>::type;
// 	std::vector<shared_task<Function>> vf;
// 	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
// 	asyncply::_pipeline(vf, std::forward<Functions>(fs)...);
// 	std::vector<ret_t> results;
// 	for(auto& v : vf)
// 		results.emplace_back(v->get());
// 	return std::move(results);
// }
//
// template <  typename Function,
// 			typename... Functions,
// 			class = typename std::enable_if<
// 				(std::is_arithmetic<typename std::result_of<Function()>::type>::value) &&
// 				(!std::is_same<typename std::result_of<Function()>::type, bool>::value)
// 			>::type
// 	>
// typename std::result_of<Function()>::type pipeline(Function&& f, Functions&&... fs)
// {
// 	using ret_t = typename std::result_of<Function()>::type;
// 	std::vector<shared_task<Function>> vf;
// 	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
// 	asyncply::_pipeline(vf, std::forward<Functions>(fs)...);
// 	std::vector<ret_t> results;
// 	for(auto& v : vf)
// 		results.emplace_back(v->get());
// 	return std::accumulate(results.begin(), results.end(), ret_t(0), std::plus<ret_t>());
// }
//
// template <  typename Function,
// 			typename... Functions,
// 			class = typename std::enable_if<
// 				(std::is_same<typename std::result_of<Function()>::type, bool>::value)
// 			>::type
// 	>
// bool pipeline(Function&& f, Functions&&... fs)
// {
// 	std::vector<shared_task<Function>> vf;
// 	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
// 	asyncply::_pipeline(vf, std::forward<Functions>(fs)...);
// 	std::vector<bool> results;
// 	for(auto& v : vf)
// 		results.emplace_back(v->get());
// 	return std::accumulate(results.begin(), results.end(), true, std::logical_and<bool>());
// }
//
// template <  typename Function,
// 			typename... Functions,
// 			class = typename std::enable_if<
// 				(std::is_void<typename std::result_of<Function()>::type>::value)
// 			>::type
// 	>
// void pipeline(Function&& f, Functions&&... fs)
// {
// 	std::vector<shared_task<Function> > vf;
// 	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
// 	asyncply::_pipeline(vf, std::forward<Functions>(fs)...);
// 	for(auto& v : vf)
// 		v->get();
// }

}

#endif

