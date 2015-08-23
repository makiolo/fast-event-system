#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

namespace asyncply {

#ifdef _WIN32
void PrintLastError();
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

class fast_event_system_API semaphore
{
public:
	using scoped_lock_t = scoped_lock<semaphore>;

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
		DWORD dwWaitResult = WaitForSingleObject(_semaphore, INFINITE);
		if (dwWaitResult == WAIT_FAILED)
		{
			PrintLastError();
			std::cout << "Error en el lock()" << std::endl;
		}
#endif
	}

	///
	/// @brief	Aumenta el semaforo. Libera la región critica.
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
		if (ReleaseSemaphore(_semaphore, 1, NULL) == 0)
		{
			PrintLastError();
			std::cout << "Error en el unlock()" << std::endl;
		}
#endif
	}

protected:

#if defined(LINUX)
	mutable sem_t _sem;
#elif defined(__APPLE__)
	sem_t* _sem;
	std::string _name_sem;
protected:
#else
	HANDLE _semaphore;
#endif
	int _concurrency;
};

}

#endif /* _SEMAPHORE_H_ */
