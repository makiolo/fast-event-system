#ifndef _FES_SEMAPHORE_H_
#define _FES_SEMAPHORE_H_

#include <mutex>
#include <condition_variable>

namespace fes {

class semaphore
{
public:
    semaphore()
        : _len(0)
    {}

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

}

#endif

