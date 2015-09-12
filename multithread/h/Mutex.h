#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "Semaphore.h"

namespace asyncply
{

class multithread_API mutex
/*!
 * This class represent a mutex.
 * OS Supported: *Windows*, *Linux*, *Mac*
 */
{
public:
	using scoped_lock_t = scoped_lock<mutex>;

	mutex();
	~mutex();

    mutex(const mutex& other) = delete;
    mutex& operator=(const mutex& other) = delete;

	inline void lock()
	//! lock this mutex
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
	//! unlock mutex
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
	//! lock if is not busy
	{
#if defined(LINUX)
		return pthread_mutex_trylock(&_mutex) == 0;
#elif defined(__APPLE__)
		assert(0);
#else
		return TryEnterCriticalSection(&_section_critical) != 0;
#endif
	}

protected:
#if defined(LINUX)
	pthread_mutex_t _mutex;
	pthread_mutexattr_t _attr;
#elif defined(__APPLE__)
	Semaphore _sem;
#else
	CRITICAL_SECTION _section_critical;
#endif
};
}

#endif  // _MUTEX_H_
