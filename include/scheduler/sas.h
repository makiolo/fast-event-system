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
#include <animator/interpolation.h>

namespace sas {

class fast_event_system_API processor
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
		auto packaged_pack_cmd = std::make_shared< std::packaged_task<void()> >(
			[this, cmd, &self]()
			{
				cmd(self);
				this->busy = false;
			}
		);
		_processor->enqueue(packaged_pack_cmd);
	}

	inline void call(const command&& cmd, fes::deltatime milli = 0, int priority = 0)
	{
		_commands(priority, milli, std::forward<const command>(cmd));
	}
	
	void update()
	{
		if (!busy)
		{
			// dispatch return true if some is dispatched
			busy = _commands.dispatch_one();
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

template <typename T>
class animations_queue
{
public:
	using command = std::function<void(T&, float)>;
	using animation = std::tuple<command, float, float, float>;
	
	explicit animations_queue()
		: busy(false)
	{ ; }
	explicit animations_queue(const std::shared_ptr<sas::processor>& pool)
		: busy(false)
		, _processor(pool)
	{ ; }
	~animations_queue() { ; }
	
	animations_queue(const animations_queue&) = delete;
	animations_queue& operator=(const animations_queue&) = delete;
	
	template <typename R>
	void add_follower(R& follower)
	{
		_conns.emplace_back(_commands.connect(std::bind(&animations_queue::planificator, this, std::ref(static_cast<T&>(follower)), std::placeholders::_1)));
	}
	
	void planificator(T& self, const animation& anim)
	{
		auto& cmd = std::get<0>(anim);
		float start = std::get<1>(anim);
		float end = std::get<2>(anim);
		float totaltime = std::get<3>(anim);

		auto packaged_pack_cmd = std::make_shared< std::packaged_task<void()> >(
			[this, start, end, totaltime, cmd, &self]()
			{
				const int FPS = 60;
				const int FRAMETIME = 1000 / FPS;
				double marktime = fes::high_resolution_clock();
				int sleeptime = 0;
				float timeline = 0.0f;
				while (true)
				{
					auto d = timeline / totaltime;
					clamp( d, 0.0f, 1.0f ); // really need clamp ?
					
					auto interp = smoothstep(start, end, d);
					cmd(self, interp);
					
					timeline += FRAMETIME;
					if (timeline >= totaltime)
					{
						break;
					}

					marktime += FRAMETIME;
					sleeptime = int(marktime - fes::high_resolution_clock());
					if (sleeptime >= 0) {
						std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));
					}
				}

				this->busy = false;
			}
		);
		_processor->enqueue(packaged_pack_cmd);
	}

	inline void call(const command& cmd, float start, float end, float totaltime, fes::deltatime milli = 0, int priority = 0)
	{
		_commands(priority, milli, std::make_tuple(cmd, start, end, totaltime));
	}
	
	void update()
	{
		if (!busy)
		{
			// dispatch return true if some is dispatched
			busy = _commands.dispatch_one();
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
	std::vector<fes::shared_connection<animation> > _conns;
	fes::async_delay<animation> _commands;
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
	
	inline void call_others(const command_others&& command, fes::deltatime milli = 0, int priority = 0)
	{
		_planner_others.call(std::forward<const command_others>(command), milli, priority);
	}

	inline void call_me(const command_me&& command, fes::deltatime milli = 0, int priority = 0)
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

} // end namespace

#endif // _SCHEDULER_ADVANCED_SIMPLE_
