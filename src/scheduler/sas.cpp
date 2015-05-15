#include <scheduler/sas.h>

namespace sas {

processor::processor(size_t threads) : _stop(false)
{
	for (size_t i = 0; i < threads; ++i)
	{
		_workers.emplace_back(
			[this]
		{
			for (;;)
			{
				std::shared_ptr< std::packaged_task<void()> > task;

				{
					std::unique_lock<std::mutex> lock(this->_queue_mutex);
					this->_condition.wait(lock, [this]{ return this->_stop || !this->_tasks.empty(); });
					if (this->_stop && this->_tasks.empty())
					{
						return;
					}
					task = std::move(this->_tasks.front());
					this->_tasks.pop();
				}

				(*task)();
			}
		}
		);
	}
}
void processor::enqueue(const std::shared_ptr< std::packaged_task<void()> >& func)
{
	{
		std::unique_lock<std::mutex> lock(_queue_mutex);

		if (!_stop)
		{
			_tasks.emplace(func);
		}
	}
	_condition.notify_one();
}

// the destructor joins all threads
processor::~processor()
{
	{
		std::unique_lock<std::mutex> lock(_queue_mutex);
		_stop = true;
	}
	_condition.notify_all();

	for (auto &worker : _workers)
	{
		worker.join();
	}
}

}
