#ifndef _WORKER_H_
#define _WORKER_H_

#include "Thread.h"

namespace asyncply {

class fast_event_system_API worker
{
public:
	worker(pool_thread* owner)
	{
		_owner = owner;
		_interrupted = false;
	}

	~worker()
	{

	}

	void execute();

	void set_interrupted(bool interrupted)
	{
		_interrupted = interrupted;
	}

	pool_thread* get_owner() const
	{
		return _owner;
	}

protected:
	pool_thread* _owner;
	bool _interrupted;
};

}
