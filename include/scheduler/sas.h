// Ricardo Marmolejo Garcia
// 15-04-2015
#ifndef _SCHEDULER_ADVANCED_SIMPLE_
#define _SCHEDULER_ADVANCED_SIMPLE_

// linux
#include <semaphore.h>
// std
#include <functional>
#include <future>
#include <atomic>
#include <mutex>
#include <fast-event-system/fes.h>

namespace sas {

template <typename T> using CommandTalker = std::function<void(T&)>;
template <typename T> using CompositeCommandTalker = std::function<CommandTalker<T>(const CommandTalker<T>&)>;

template <typename T>
class scheduler
{
public:
	using command = CommandTalker<T>;
	
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

	inline void call(const command& command, int milli = 0, int priority = 0)
	{
		//_commands(priority, std::chrono::milliseconds(milli), command);
		_commands(command);
	}
	
	void update()
	{
		if (!_busy)
		{
			_busy = _commands.dispatch();
		}
	}
protected:
	std::vector<fes::shared_connection<command> > _conns;
	//fes::queue_delayer<command> _commands;
	fes::queue_fast<command> _commands;
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
	
	inline void call_others(const command_others& command, int milli = 0, int priority = 0)
	{
		_planner_others.call(command, milli, priority);
	}

	inline void call_me(const command_me& command, int milli = 0, int priority = 0)
	{
		_planner_me.call(command, milli, priority);
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
		(void) sem_init(&_sem, 0, concurrency);
	}
	
	~syncronizer()
	{
		(void) sem_destroy(&_sem);
	}
	
	syncronizer(const syncronizer&) = delete;
	syncronizer& operator=(const syncronizer&) = delete;

	inline void lock()
	{
		(void) sem_wait(&_sem);
	}
	
	inline void unlock()
	{
		(void) sem_post(&_sem);
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
	sem_t _sem;
	//std::mutex _m;
	//std::condition_variable _signal;
	//std::mutex _signal_mutex;
};

} // end namespace

#endif // _SCHEDULER_ADVANCED_SIMPLE_
