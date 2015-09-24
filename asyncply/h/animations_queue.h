#ifndef _ANIMATIONS_QUEUE_H_
#define _ANIMATIONS_QUEUE_H_

namespace asyncply {

template <typename T>
class animations_queue
{
public:
	using command = std::function<void(T&, float)>;
	using animation = std::tuple<command, float, float, float>;

	explicit animations_queue()
		: busy(false)
	{
		;
	}
	~animations_queue() { ; }

	animations_queue(const animations_queue&) = delete;
	animations_queue& operator=(const animations_queue&) = delete;

	template <typename R>
	void connect(R& follower)
	{
#ifdef _DEBUG
#define debug_cast dynamic_cast
#else
#define debug_cast static_cast
#endif
		_conns.emplace_back(_commands.connect(std::bind(&animations_queue::planificator, this,
			std::ref(debug_cast<T&>(follower)), std::placeholders::_1)));
	}

	void planificator(T& self, const animation& anim)
	{
		auto& cmd = std::get<0>(anim);
		float start = std::get<1>(anim);
		float end = std::get<2>(anim);
		float totaltime = std::get<3>(anim);

		if (_job)
		{
			_job->get();
		}
		_job = run(
			[start, end, totaltime, &cmd, &self]()
			{
				const int FPS = 60;
				const int FRAMETIME = 1000 / FPS;
				// float marktime = fes::high_resolution_clock();
				float marktime = 0.0f;  // TODO:
				int sleeptime = 0;
				float timeline = 0.0f;
				while (true)
				{
					auto d = timeline / totaltime;
					clamp(d, 0.0f, 1.0f);  // really need clamp ?

					auto interp = smoothstep(start, end, d);
					cmd(self, interp);

					timeline += FRAMETIME;
					if (timeline >= totaltime)
					{
						break;
					}

					marktime += FRAMETIME;
					// sleeptime = static_cast<int>(marktime - float(fes::high_resolution_clock()));
					sleeptime = static_cast<int>(marktime - 0.0f);
					if (sleeptime >= 0)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));
					}
				}
			},
			[this]()
			{
				this->busy = false;
			});
	}

	inline void call(const command& cmd, float start, float end, float totaltime, fes::deltatime milli = fes::deltatime(0), int priority = 0)
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

public:
	std::atomic<bool> busy;

protected:
	std::vector<fes::shared_connection<animation>> _conns;
	fes::async_delay<animation> _commands;
	std::shared_ptr<task<void>> _job;
};

}

#endif

