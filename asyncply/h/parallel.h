#ifndef _PARALLEL_H_
#define _PARALLEL_H_

#include <vector>
#include <fes/h/common.h>
#include <asyncply/h/run.h>

namespace asyncply {

template <typename Function>
void _parallel(std::vector<shared_task<Function>>& vf, Function&& f)
{
	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
}

template <typename Function, typename... Functions>
void _parallel(std::vector<shared_task<Function>>& vf, Function&& f, Functions&&... fs)
{
	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
}

template <  typename Function,
			typename... Functions,
			class = typename std::enable_if<
				(!std::is_arithmetic<typename std::result_of<Function()>::type>::value) &&
				(!std::is_same<typename std::result_of<Function()>::type, bool>::value) &&
				(!std::is_void<typename std::result_of<Function()>::type>::value)
			>::type
	>
std::vector<typename std::result_of<Function()>::type> parallel(Function&& f, Functions&&... fs)
{
	using ret_t = typename std::result_of<Function()>::type;
	std::vector<shared_task<Function>> vf;
	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
	std::vector<ret_t> results;
	for(auto& v : vf)
		results.emplace_back(v->get());
	return std::move(results);
}

template <  typename Function,
			typename... Functions,
			class = typename std::enable_if<
				(std::is_arithmetic<typename std::result_of<Function()>::type>::value) &&
				(!std::is_same<typename std::result_of<Function()>::type, bool>::value)
			>::type
	>
typename std::result_of<Function()>::type parallel(Function&& f, Functions&&... fs)
{
	using ret_t = typename std::result_of<Function()>::type;
	std::vector<shared_task<Function>> vf;
	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
	std::vector<ret_t> results;
	for(auto& v : vf)
		results.emplace_back(v->get());
	return std::accumulate(results.begin(), results.end(), ret_t(0), std::plus<ret_t>());
}

template <  typename Function,
			typename... Functions,
			class = typename std::enable_if<
				(std::is_same<typename std::result_of<Function()>::type, bool>::value)
			>::type
	>
bool parallel(Function&& f, Functions&&... fs)
{
	std::vector<shared_task<Function>> vf;
	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
	std::vector<bool> results;
	for(auto& v : vf)
		results.emplace_back(v->get());
	return std::accumulate(results.begin(), results.end(), true, std::logical_and<bool>());
}

template <  typename Function,
			typename... Functions,
			class = typename std::enable_if<
				(std::is_void<typename std::result_of<Function()>::type>::value)
			>::type
	>
void parallel(Function&& f, Functions&&... fs)
{
	std::vector<shared_task<Function> > vf;
	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
	for(auto& v : vf)
		v->get();
}

}

#endif

