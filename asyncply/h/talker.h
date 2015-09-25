#ifndef _TALKER_H_
#define _TALKER_H_

#include <thread>
#include <asyncply/h/commands_queue.h>

namespace asyncply {

template <typename SELF, typename FOLLOWERS = SELF>
class talker
{
public:
	using command_others = typename commands_queue<FOLLOWERS>::command;
	using command_me = typename commands_queue<SELF>::command;

	explicit talker()
	{
#ifdef _DEBUG
#define debug_cast dynamic_cast
#else
#define debug_cast static_cast
#endif
		_planner_me.template connect<SELF>(debug_cast<SELF&>(*this));
	}

	virtual ~talker() {}

	talker(const talker&) = delete;
	talker& operator=(const talker&) = delete;

	void connect(FOLLOWERS& t) { _planner_others.connect(t); }

	inline void call_others(
		const command_others& command, fes::deltatime milli = fes::deltatime(0), int priority = 0)
	{
		_planner_others.call(command, milli, priority);
	}

	inline void call_me(
		const command_me& command, fes::deltatime milli = fes::deltatime(0), int priority = 0)
	{
		_planner_me.call(command, milli, priority);
	}

	void update()
	{
		_planner_others.update();
		_planner_me.update();
	}

	inline void sleep(int milli) { std::this_thread::sleep_for(std::chrono::milliseconds(milli)); }

protected:
	commands_queue<FOLLOWERS> _planner_others;
	commands_queue<SELF> _planner_me;
};

}

#endif

