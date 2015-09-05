#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

namespace asyncply {

#ifdef _WIN32
#ifdef _DEBUG
void PrintLastError();
#endif
#endif

template <typename T>
class scoped_lock
{
public:
	explicit scoped_lock(T& semaforo) : _semaphore(semaforo)
	{
		_semaphore.lock();
	}

	~scoped_lock()
	{
		_semaphore.unlock();
	}
	scoped_lock& operator=(const scoped_lock&) = delete;

protected:
	T& _semaphore;
};

class multithread_API semaphore
{
public:
	typedef scoped_lock<semaphore> scoped_lock_t;

	semaphore(int concurrency = 1, bool isForSync = false);
	~semaphore();

	///
	/// Reduce el valor del semaforo. Bloquea la región critica. Esta operación tiene múltiples nombres.
	///  * wait (s)
	///	 * {
	///		  if s > 0
	///				s--
	///		  else // s == 0
	///				bloqueo
	///		}
	///
	///		esperar / wait / lock / down / sleep / P
	///
	inline void lock() const
	{
#if defined(LINUX)

		(void) sem_wait(&_sem);

#elif defined(__APPLE__)

		(void) sem_wait(_sem);

#else
#ifdef _DEBUG
		DWORD dwWaitResult = WaitForSingleObject(_semaphore, INFINITE);
		if (dwWaitResult == WAIT_FAILED)
		{
			PrintLastError();
			std::cout << "Error en el lock()" << std::endl;
		}
#else
		WaitForSingleObject(_semaphore, INFINITE);
#endif
#endif
	}

	///
	/// Aumenta el semaforo. Libera la región critica.
	///    signal(s)
	///    {
	///        if s == 0
	///            s++
	///        else // s > 0
	///            if s < MAX
	///                s++
	///    }
	///
	///	avisar / signal / unlock / up / wakeup / release / V
	///
	inline void unlock() const
	{
#if defined(LINUX)

		(void) sem_post(&_sem);

#elif defined(__APPLE__)

		(void) sem_post(_sem);

#else
#ifdef _DEBUG
		if (ReleaseSemaphore(_semaphore, 1, NULL) == 0)
		{
			PrintLastError();
			std::cout << "Error en el unlock()" << std::endl;
		}
#else
		ReleaseSemaphore(_semaphore, 1, NULL);
#endif
#endif
	}

	int get_value() const
	{
#if defined(LINUX)
		int value;
		(void) sem_getvalue(&_sem, &value);
		return value;
#elif defined(__APPLE__)
		// apple no lo implementa (sem_getvalue)
		return -1;
#else
		// En windows es complicado saber el valor de un semaforo
		// Ver NtQuerySemaphore
		return -1;
#endif
	}

protected:

#if defined(LINUX)
	mutable sem_t _sem;
#elif defined(__APPLE__)
	sem_t* _sem;
	std::string _name_sem;
public:
	static int _count_sem;
protected:
#else
	HANDLE _semaphore;
#endif
	int _concurrency;
};

}

#endif // _SEMAPHORE_H_
