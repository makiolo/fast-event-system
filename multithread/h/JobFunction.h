#ifndef _JOB_FUNCTION_H_
#define _JOB_FUNCTION_H_

#include "Job.h"

namespace asyncply
{

class job_function : public job
{
public:
	job_function(const std::function<job::StateJob()>& function)
        : _function( function )
        , _callback( true )
	{

	}

	job_function()
        : _function( nullptr )
        , _callback(false)
    { ; }

	~job_function(void) {}

	void set_function(const std::function<job::StateJob()>& function)
	{
		_function = function;
		_callback = true;
	}

	virtual void Start() { assert(_callback); }

	virtual job::StateJob Update() { return _function(); }

	virtual void Finish() {}

protected:
	std::function<job::StateJob()> _function;
	bool _callback;
};
}

#endif  // _JOB_FUNCTION_H_
