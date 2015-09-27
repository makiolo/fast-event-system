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

// namespace asyncply {

using StatusCode = enum
{
	RUNNING = 0,
	COMPLETED,
	FAILED,
	ABORTED,
	PANIC_ERROR
};

using behaviour = flow_pipeline<StatusCode>;

// }

#endif

