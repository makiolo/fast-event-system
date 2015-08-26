#ifndef _SYNCRONIZER_H_
#define _SYNCRONIZER_H_

#include "Semaphore.h"

namespace asyncply {

class fast_event_system_API synchronizer : public semaphore
{
public:
	synchronizer(int max = 1) : semaphore(max, true)
	{

	}

	~synchronizer()
	{

	}

	inline void wait(int count = 1) const
	{
		for (int i(0); i < count; ++i)
			lock();
	}

	inline void signal(int count = 1) const
	{
		for (int i(0); i < count; ++i)
			unlock();
	}
};

}

#endif // _SYNCRONIZER_H_
