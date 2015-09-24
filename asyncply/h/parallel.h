#ifndef _PARALLEL_H_
#define _PARALLEL_H_

namespace asyncply {

template <typename Function>
void _parallel(std::vector<shared_task<Function>>& vf, Function&& f)
{
	vf.push_back(asyncply::run(std::forward<Function>(f)));
}

template <typename Function, typename... Functions>
void _parallel(std::vector<shared_task<Function>>& vf, Function&& f, Functions&&... fs)
{
	vf.push_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
}

template <typename Function, typename... Functions>
void parallel(std::vector<shared_task<Function>>& vf, Function&& f, Functions&&... fs)
{
	vf.push_back(asyncply::run(std::forward<Function>(f)));
	asyncply::_parallel(vf, std::forward<Functions>(fs)...);
}

template <typename Function>
void parallel(std::vector<shared_task<Function>>& vf, Function&& f)
{
	vf.push_back(asyncply::run(std::forward<Function>(f)));
}

}

#endif

