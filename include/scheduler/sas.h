// Ricardo Marmolejo Garcia
// 15-04-2015
#ifndef _SCHEDULER_ADVANCED_SIMPLE_
#define _SCHEDULER_ADVANCED_SIMPLE_

// linux
#ifndef _WIN32
#include <semaphore.h>
#else
#include <windows.h>
#endif

// std
#include <functional>
#include <future>
#include <atomic>
#include <mutex>
#include <fast-event-system/fes.h>

namespace sas {

class processor
{
public:
    processor(size_t);
    ~processor();
	
	void enqueue(const std::function<void()>& func);
	
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > _workers;
    // the task queue
    std::queue< std::function<void()> > _tasks;
    
    // synchronization
    std::mutex _queue_mutex;
    std::condition_variable _condition;
    bool _stop;
};
 
inline processor::processor(size_t threads) : _stop(false)
{
	for (size_t i = 0; i < threads; ++i)
	{
		_workers.emplace_back(
			[this]
			{
				for (;;)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->_queue_mutex);
						this->_condition.wait(lock,
							[this]{ return this->_stop || !this->_tasks.empty(); });
						if (this->_stop && this->_tasks.empty())
							return;
						task = std::move(this->_tasks.front());
						this->_tasks.pop();
					}

					task();
				}
			}
		);
	}
}

void processor::enqueue(const std::function<void()>& func)
{
	auto packaged_func = std::make_shared< std::packaged_task<void()> >(std::bind(func));
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);

        if(_stop)
            throw std::runtime_error("enqueue on stopped thread_pool");

		_tasks.emplace(
			[packaged_func]()
			{
				(*packaged_func)();
			}
		);
    }
    _condition.notify_one();
}

// the destructor joins all threads
inline processor::~processor()
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

template <typename T>
class scheduler
{
public:
	using command = std::function<void(T&)>;
	
	explicit scheduler()
		: busy(false)
	{ ; }
	explicit scheduler(const std::shared_ptr<sas::processor>& pool)
		: busy(false)
		, _processor(pool)
	{ ; }
	~scheduler() { ; }
	
	scheduler(const scheduler&) = delete;
	scheduler& operator=(const scheduler&) = delete;
	
	template <typename R>
	void add_follower(R& follower)
	{
		_conns.emplace_back(_commands.connect(std::bind(&scheduler::planificator, this, std::ref(static_cast<T&>(follower)), std::placeholders::_1)));
	}
	
	void planificator(T& self, const command& cmd)
	{
		auto packaged_cmd = std::make_shared< std::packaged_task<void()> >(std::bind(cmd, std::ref(self)));
		_processor->enqueue(
			[packaged_cmd, this]()
			{
				(*packaged_cmd)();
				this->busy = false;
			}
		);
	}

	inline void call(const command&& cmd, int milli = 0, int priority = 0)
	{
		_commands(priority, std::chrono::milliseconds(milli), std::forward<const command>(cmd));
	}
	
	void update()
	{
		if (!busy)
		{
			// dispatch return true if some is dispatched
			busy = _commands.dispatch();
		}
	}
	
	// inject depends
	void set_processor(const std::shared_ptr<sas::processor>& pool)
	{
		_processor = pool;
	}
	
public:
	std::atomic<bool> busy;
protected:
	std::vector<fes::shared_connection<command> > _conns;
	fes::queue_delayer<command> _commands;
    std::condition_variable _condition;
    std::mutex _condition_mutex;
	std::shared_ptr<sas::processor> _processor;
};

template <typename SELF, typename FOLLOWERS>
class talker
{
public:
	using command_others = typename scheduler<FOLLOWERS>::command;
	using command_me = typename scheduler<SELF>::command;
	
	explicit talker()
	{
		_planner_me.add_follower(*this);
	}
	
	~talker()
	{
		
	}
	
	talker(const talker&) = delete;
	talker& operator=(const talker&) = delete;
	
	void add_follower(FOLLOWERS& talker)
	{
		_planner_others.add_follower(talker);
	}
	
	inline void call_others(const command_others&& command, int milli = 0, int priority = 0)
	{
		_planner_others.call(std::forward<const command_others>(command), milli, priority);
	}

	inline void call_me(const command_me&& command, int milli = 0, int priority = 0)
	{
		_planner_me.call(std::forward<const command_me>(command), milli, priority);
	}
	
	void update()
	{
		_planner_others.update();
		_planner_me.update();
	}
	
	inline void sleep(int milli)
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(milli) );
	}
	
	// inject depends
	void set_processor(const std::shared_ptr<sas::processor>& pool)
	{
		_planner_others.set_processor(pool);
		_planner_me.set_processor(pool);
	}
	
protected:
	scheduler<FOLLOWERS> _planner_others;
	scheduler<SELF> _planner_me;
};

#if 0

class syncronizer
{
public:
	syncronizer(int concurrency = 1)
	{
#ifndef _WIN32
		(void) sem_init(&_sem, 0, concurrency);
#else
		_sem = CreateSemaphore(NULL, 0, concurrency, NULL);
#endif
	}
	
	~syncronizer()
	{
#ifndef _WIN32
		(void) sem_destroy(&_sem);
#else
		CloseHandle(_sem);
#endif
	}
	
	syncronizer(const syncronizer&) = delete;
	syncronizer& operator=(const syncronizer&) = delete;

	inline void wait()
	{
#ifndef _WIN32
		//_signal = _signal - 1;
		//std::unique_lock<std::mutex> context(_cond_mutex);
		//_cond.wait(context, [&](){return _signal < 0;});
#else
		DWORD dwWaitResult = WaitForSingleObject(_sem, INFINITE);
		if (dwWaitResult == WAIT_FAILED)
		{
			std::cerr << "Error en el lock()" << std::endl;
		}
#endif
	}
	
	inline void signal()
	{
#ifndef _WIN32
		//_signal = _signal + 1;
		//_cond.notify_all();
#else
		if (ReleaseSemaphore(_sem, 1, NULL) == 0)
		{
			std::cerr << "Error in unlock()" << std::endl;
		}
#endif
	}

protected:
#ifndef _WIN32
	sem_t _sem;
	//std::condition_variable _cond;
	//std::mutex _cond_mutex;
	//std::atomic<int> _signal;
	//
	//std::mutex _cond2_mutex;
#else
	HANDLE _sem;
#endif
};

#endif

} // end namespace

#endif // _SCHEDULER_ADVANCED_SIMPLE_

