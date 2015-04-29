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
		: _busy(false) { ; }
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
		std::thread th([&]()
		{
			cmd(self);
			_busy = false;
		});
		th.detach();
	}

	inline void call(const command&& cmd, int milli = 0, int priority = 0)
	{
		_commands(priority, std::chrono::milliseconds(milli), std::forward<const command>(cmd));
		//_commands(std::forward<const command>(cmd));
	}
	
	void update()
	{
		if (!_busy)
		{
			// dispatch return true if some is dispatched
			_busy = _commands.dispatch();
		}
	}
protected:
	std::vector<fes::shared_connection<command> > _conns;
	fes::queue_delayer<command> _commands;
	//fes::queue_fast<command> _commands;
	std::atomic<bool> _busy;
};

template <typename SELF, typename FOLLOWERS>
class talker
{
public:
	using command_others = typename scheduler<FOLLOWERS>::command;
	using command_me = typename scheduler<SELF>::command;
	
	talker()
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
	
protected:
	scheduler<FOLLOWERS> _planner_others;
	scheduler<SELF> _planner_me;
};

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

	inline void lock()
	{
#ifndef _WIN32
		(void) sem_wait(&_sem);
#else
		DWORD dwWaitResult = WaitForSingleObject(_sem, INFINITE);
		if (dwWaitResult == WAIT_FAILED)
		{
			std::cerr << "Error en el lock()" << std::endl;
		}
#endif
	}
	
	inline void unlock()
	{
#ifndef _WIN32
		(void) sem_post(&_sem);
#else
		if (ReleaseSemaphore(_sem, 1, NULL) == 0)
		{
			std::cerr << "Error in unlock()" << std::endl;
		}
#endif
	}

	inline void wait(int count = 1)
	{
		for(int i = 0; i<count; ++i)
		{
			lock();
		}
	}
	
	inline void signal(int count = 1)
	{
		for(int i = 0; i<count; ++i)
		{
			unlock();
		}
	}
protected:
#ifndef _WIN32
	sem_t _sem;
#else
	HANDLE _sem;
#endif
};

} // end namespace

#endif // _SCHEDULER_ADVANCED_SIMPLE_
