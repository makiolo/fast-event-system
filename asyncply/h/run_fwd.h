#ifndef _RUN_FWD_H_
#define _RUN_FWD_H_

#include <memory>
#include <type_traits>
#include <asyncply/h/task_fwd.h>

namespace asyncply {

template <typename Function>
using task_of_functor = asyncply::task<typename std::result_of<Function()>::type>;

template <typename Function>
using shared_task = std::shared_ptr<task_of_functor<Function>>;

template <typename Function>
shared_task<Function> run(Function&& f);

template <typename Function, typename FunctionPost>
shared_task<Function> run(Function&& f, FunctionPost&& fp);

}

#endif

