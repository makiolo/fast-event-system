#ifndef _SEQUENCE_H_
#define _SEQUENCE_H_

#include <functional>
#include <asyncply/h/run.h>

namespace asyncply {

template <typename Data, typename Function>
std::function<Data(const Data&)> _sequence(Function&& f)
{
	return [&f](const Data& data)
	{
		auto job = asyncply::run(
			[&f, &data]()
			{
				return f(data);
			});
		return job->get();
	};
}

template <typename Data, typename Function, typename... Functions>
std::function<Data(const Data&)> _sequence(Function&& f, Functions&&... fs)
{
	return [&f, &fs...](const Data& data)
	{
		auto job = asyncply::run(
			[&f, &data]()
			{
				return f(data);
			},
			[&fs...](const Data& d)
			{
				return asyncply::_sequence<Data>(std::forward<Functions>(fs)...)(d);
			});
		return job->get();
	};
}

template <typename Data, typename... Functions>
Data sequence(const Data& data, Functions&&... fs)
{
	auto job = asyncply::run(
			[&data, &fs...]()
			{
				return asyncply::_sequence<Data>(std::forward<Functions>(fs)...)(data);
			});
	return job->get();
}

}

#endif

