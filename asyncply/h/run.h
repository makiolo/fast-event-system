#ifndef _RUN_H_
#define _RUN_H_

/*
	using call_type = boost::coroutines::asymmetric_coroutine<void>::pull_type;
	using yield_type = boost::coroutines::asymmetric_coroutine<void>::push_type;
*/

namespace asyncply {

template <typename Function>
shared_task<Function> run(Function&& f)
{
	static Allocator<task_of_functor<Function>> alloc;

	auto job = std::allocate_shared< task_of_functor<Function> >(alloc, std::forward<Function>(f));
	Poco::ThreadPool::defaultPool().start(*job);
	return job;
}

template <typename Function, typename FunctionPost>
shared_task<Function> run(Function&& f, FunctionPost&& fp)
{
	static Allocator<task_of_functor<Function>> alloc;

	auto job = std::allocate_shared<task_of_functor<Function> >(alloc, std::forward<Function>(f), std::forward<FunctionPost>(fp));
	Poco::ThreadPool::defaultPool().start(*job);
	return job;
}

}

#endif

