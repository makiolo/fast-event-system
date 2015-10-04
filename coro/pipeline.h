// Modified from work of Aldrin D'Souza.

#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include <vector>
#include <memory>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/coroutine/coroutine.hpp>

namespace asyncply {

template <typename T>
using coro = boost::coroutines::coroutine<T()>;

template <typename T>
using yield_type = typename coro<T>::caller_type;

template <typename T>
using link = boost::function<void(asyncply::coro<T>&, asyncply::yield_type<T>&)>;

template <typename T, typename Function>
std::shared_ptr< coro<T> > corun(Function&& f)
{
	return std::make_shared<coro<T> >(std::forward<Function>(f));
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

template <typename T>
class pipeline
{
private:
	using coro = asyncply::coro<T>;
	using coroptr = std::shared_ptr<coro>;

public:
	using in = asyncply::coro<T>;
	using out = asyncply::yield_type<T>;
	using link = asyncply::link<T>;

	template <typename Function>
	pipeline(Function&& f)
	{
		std::vector<coroptr> coros;
		coros.emplace_back(
				asyncply::corun<T>(
					[](asyncply::yield_type<T>&) { ; }
				)
		);
		coros.emplace_back(
				asyncply::corun<T>(
					boost::bind(f, boost::ref(*coros.back().get()), _1)
				)
		);
	}

	template <typename Function, typename ... Functions>
	pipeline(Function&& f, Functions&& ... fs)
	{
		std::vector<coroptr> coros;
		coros.emplace_back(
				asyncply::corun<T>(
					[](asyncply::yield_type<T>&) { ; }
				)
		);
		coros.emplace_back(
				asyncply::corun<T>(
					boost::bind(f, boost::ref(*coros.back().get()), _1)
				)
		);
		// recursion
		_add(coros, std::forward<Functions>(fs)...);
	}

protected:
	template <typename Function>
	void _add(std::vector<coroptr>& coros, Function&& f)
	{
		coros.emplace_back(
				asyncply::corun<T>(
					boost::bind(f, boost::ref(*coros.back().get()), _1)
				)
		);
	}

	template <typename Function, typename ... Functions>
	void _add(std::vector<coroptr>& coros, Function&& f, Functions&& ... fs)
	{
		coros.emplace_back(
				asyncply::corun<T>(
					boost::bind(f, boost::ref(*coros.back().get()), _1)
				)
		);
		// recursion
		_add(coros, std::forward<Functions>(fs)...);
	}
};

}

#endif

