#ifndef _COMMANDS_QUEUE_H_
#define _COMMANDS_QUEUE_H_

#include <vector>
#include <memory>
#include <atomic>
#include <functional>
#include <asyncply/h/run.h>
#include <fes/h/async_delay.h>

namespace asyncply {

template <typename T>
class commands_queue
{
public:
	using command = std::function<void(T&)>;

	explicit commands_queue()
		: busy(false)
	{
		;
	}
	~commands_queue() { ; }

	commands_queue(const commands_queue&) = delete;
	commands_queue& operator=(const commands_queue&) = delete;

	template <typename R>
	void connect(R& follower)
	{
#ifdef _DEBUG
#define debug_cast dynamic_cast
#else
#define debug_cast static_cast
#endif
		_conns.emplace_back(_commands.connect(std::bind(&commands_queue::planificator, this,
			std::ref(debug_cast<T&>(follower)), std::placeholders::_1)));
	}

	void planificator(T& self, const command& cmd)
	{
		if (_job)
		{
			_job->get();
		}
		_job = asyncply::run(std::bind(cmd, std::ref(self)), [this]()
			{
				this->busy = false;
			});
	}

	inline void call(const command& cmd, fes::deltatime milli = fes::deltatime(0), int priority = 0)
	{
		_commands(priority, milli, cmd);
	}

	void update()
	{
		if (!busy)
		{
			// dispatch return true if some is dispatched
			busy = _commands.dispatch_one();
		}
	}

public:
	std::atomic<bool> busy;

protected:
	std::vector<fes::shared_connection<command>> _conns;
	fes::async_delay<command> _commands;
	std::shared_ptr<task<void>> _job;
};

}

#endif

