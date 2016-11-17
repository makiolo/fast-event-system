#ifndef _FES_SEMAPHORE_H_
#define _FES_SEMAPHORE_H_

#include <mutex>
#include <condition_variable>
// #include <fast-event-system/concurrentqueue/blockingconcurrentqueue.h>

namespace fes {

#if 1

class semaphore
{
public:
	explicit semaphore(unsigned long len = 0) : _len(len) { ; }

	void notify()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		++_len;
		_cond.notify_one();
	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
			_cond.wait(lock, [this](){return (this->size() > 0);});
		--_len;
	}

	inline unsigned long size() const
	{
		return _len;
	}

protected:
	std::mutex _mutex;
	std::condition_variable _cond;
	unsigned long _len;
};

#else

using semaphore = moodycamel::details::mpmc_sema::Semaphore;

#endif

}

#endif
