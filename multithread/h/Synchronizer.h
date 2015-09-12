#ifndef _SYNCRONIZER_H_
#define _SYNCRONIZER_H_

#include "Semaphore.h"

namespace asyncply
{

class multithread_API synchronizer : public semaphore
{
public:
	synchronizer(uint32_t max = 1)
		: semaphore(max, true)
	{
	}

	virtual ~synchronizer() {}

	inline void wait(uint32_t count = 1) const
	{
		for (size_t i(0); i < count; ++i)
			lock();
	}

	inline void signal(uint32_t count = 1) const
	{
		for (size_t i(0); i < count; ++i)
			unlock();
	}
};
}

#endif  // _SYNCRONIZER_H_
