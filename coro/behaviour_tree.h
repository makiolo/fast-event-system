#ifndef _BEHAVIOUR_TREE_H_
#define _BEHAVIOUR_TREE_H_

#include <set>
#include <regex>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <exception>
#include <vector>

#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <coro/pipeline.h>

using StatusCode = enum
{
	RUNNING = 0,
	COMPLETED,
	FAILED,
	ABORTED,
	PANIC_ERROR
};

using behaviour = flow_pipeline<StatusCode>;

behaviour::link _for(int n)
{
	return [n](behaviour::in&, behaviour::out& yield)
	{
		// ignore source
		for(int i=0; i<n; ++i)
		{
			yield(RUNNING);
		}
		yield(COMPLETED);
	};
}

behaviour::link _jump()
{
	return [](behaviour::in&, behaviour::out& yield)
	{
		std::cout << "JUMPING!!" << std::endl;
		yield(COMPLETED);
	};
}

#endif

