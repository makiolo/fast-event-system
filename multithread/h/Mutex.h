#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "Semaphore.h"

namespace asyncply {

class multithread_API mutex
{
public:
	using scoped_lock_t = scoped_lock<mutex>;

	mutex();
	~mutex();

	inline void lock()
	{
#if defined(LINUX)
		pthread_mutex_lock(&_mutex);
#elif defined(__APPLE__)
		_sem.lock();
#else
		EnterCriticalSection(&_section_critical);
#endif
	}

	inline void unlock()
	{
#if defined(LINUX)
		pthread_mutex_unlock(&_mutex);
#elif defined(__APPLE__)
		_sem.unlock();
#else
		LeaveCriticalSection(&_section_critical);
#endif
	}

	inline bool trylock()
	{
#if defined(LINUX)
		return pthread_mutex_trylock(&_mutex) == 0;
#elif defined(__APPLE__)
		// sin implementar
		assert(0);
#else
		return TryEnterCriticalSection(&_section_critical) != 0;
#endif
	}

protected:

#if defined(LINUX)
	pthread_mutex_t _mutex;
	pthread_mutexattr_t  _attr;
#elif defined(__APPLE__)
	Semaphore _sem;
#else
	CRITICAL_SECTION _section_critical;
#endif
};

}

#endif // _MUTEX_H_

