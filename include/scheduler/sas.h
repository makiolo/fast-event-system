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
	processor(const processor&) = delete;
	processor& operator=(const processor&) = delete;
	
	void enqueue(const std::shared_ptr< std::packaged_task<void()> >& func);
	
private:
	// need to keep track of threads so we can join them
	std::vector< std::thread > _workers;
	// the task queue
	std::queue< std::shared_ptr< std::packaged_task<void()> > > _tasks;
	
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
		
		if(!_stop)
		{
			_tasks.emplace(func);
		}
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
class commands_queue
{
public:
	using command = std::function<void(T&)>;
	
	explicit commands_queue()
		: busy(false)
	{ ; }
	explicit commands_queue(const std::shared_ptr<sas::processor>& pool)
		: busy(false)
		, _processor(pool)
	{ ; }
	~commands_queue() { ; }
	
	commands_queue(const commands_queue&) = delete;
	commands_queue& operator=(const commands_queue&) = delete;
	
	template <typename R>
	void add_follower(R& follower)
	{
		_conns.emplace_back(_commands.connect(std::bind(&commands_queue::planificator, this, std::ref(static_cast<T&>(follower)), std::placeholders::_1)));
	}
	
	void planificator(T& self, const command& cmd)
	{
		auto packaged_cmd = std::make_shared< std::packaged_task<void()> >(std::bind(cmd, std::ref(self)));
		auto packaged_pack_cmd = std::make_shared< std::packaged_task<void()> >(
			[packaged_cmd, this]()
			{
				(*packaged_cmd)();
				this->busy = false;
			}
		);
		_processor->enqueue(packaged_pack_cmd);
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
	fes::async_delay<command> _commands;
	std::shared_ptr<sas::processor> _processor;
};

template <typename SELF, typename FOLLOWERS = SELF>
class talker
{
public:
	using command_others = typename commands_queue<FOLLOWERS>::command;
	using command_me = typename commands_queue<SELF>::command;
	
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
	commands_queue<FOLLOWERS> _planner_others;
	commands_queue<SELF> _planner_me;
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

