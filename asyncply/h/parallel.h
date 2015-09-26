#ifndef _PARALLEL_H_
#define _PARALLEL_H_

#include <vector>
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

template <typename Function, typename... Functions>
void parallel(std::vector<shared_task<Function>>& vf, Function&& f, Functions&&... fs)
{
	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
}

template <typename Function>
void parallel(std::vector<shared_task<Function>>& vf, Function&& f)
{
	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
}

template <  typename Function,
			typename... Functions>
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
			typename = std::enable_if_t<std::is_same<typename std::result_of<Function()>::type, bool>::value>
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
			typename = std::enable_if_t<std::is_void<typename std::result_of<Function()>::type>::value>
	>
void parallel(Function&& f, Functions&&... fs)
{
	std::vector<shared_task<Function>> vf;
	vf.emplace_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
}

}

#endif

