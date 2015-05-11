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

template <typename T>
class scheduler
{
public:
	using command = std::function<void(T&)>;
	
	explicit scheduler()
		: _busy(false)
	{ ; }
	explicit scheduler(const std::shared_ptr<fes::thread_pool>& pool)
		: _busy(false)
		, _pool(pool)
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
		// return future
		_pool->enqueue([&](T& self)
		{
			cmd(self);
			_busy = false;
		}, std::ref(self));
	}

	inline void call(const command&& cmd, int milli = 0, int priority = 0)
	{
		_commands(priority, std::chrono::milliseconds(milli), std::forward<const command>(cmd));
	}
	
	void update()
	{
		if (!_busy)
		{
			// dispatch return true if some is dispatched
			_busy = _commands.dispatch();
		}
	}
	
	// inject depends
	void set_thread_pool(const std::shared_ptr<fes::thread_pool>& pool)
	{
		_pool = pool;
	}
	
protected:
	std::vector<fes::shared_connection<command> > _conns;
	fes::queue_delayer<command> _commands;
	std::atomic<bool> _busy;
	std::shared_ptr<fes::thread_pool> _pool;
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
	void set_thread_pool(const std::shared_ptr<fes::thread_pool>& pool)
	{
		_planner_others.set_thread_pool(pool);
		_planner_me.set_thread_pool(pool);
	}
	
protected:
	scheduler<FOLLOWERS> _planner_others;
	scheduler<SELF> _planner_me;
};

class syncronizer
{
public:
	syncronizer(int concurrency = 1)
		// : _signal(0)
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

} // end namespace

#endif // _SCHEDULER_ADVANCED_SIMPLE_

