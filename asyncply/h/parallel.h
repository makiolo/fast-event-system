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

}

#endif

