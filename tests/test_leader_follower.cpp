// Ricardo Marmolejo Garcia
// 15-04-2015
// experimental reinterpretation pattern command
#include <iostream>
#include <tuple>
#include <functional>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <fast-event-system/fes.h>
#include <future>
#include <chrono>
#include <assert.h>
#include <atomic>
#include <exception>
#include <mutex>

namespace lead {

template <typename T> using CommandTalker = std::function<void(T&)>;
template <typename T> using CompositeCommandTalker = std::function<CommandTalker<T>(const CommandTalker<T>&)>;

template <typename T>
class scheduler
{
public:
	using command = CommandTalker<T>;

	explicit scheduler()
		: _busy(false) { ; }
	~scheduler() { ;  }

	scheduler(const scheduler&) = delete;
	scheduler& operator=(const scheduler&) = delete;
	
	void add_follower(T& follower)
	{
		_conns.emplace_back(_commands.connect(std::bind(&scheduler::planificator, this, std::ref(follower), std::placeholders::_1)));
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
		_commands(priority, std::chrono::milliseconds(milli), command);
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
	fes::queue_delayer<command> _commands;
	std::atomic<bool> _busy;
};

class syncronizer
{
public:
	syncronizer() { ; }
	~syncronizer() { ; }
	syncronizer(const syncronizer&) = delete;
	syncronizer& operator=(const syncronizer&) = delete;

	void signal()
	{
		_signal.notify_one();
	}
	
	void wait()
	{
		std::unique_lock<std::mutex> context(_signal_mutex);
		_signal.wait(context);
	}
protected:	
	std::condition_variable _signal;
	std::mutex _signal_mutex;
};

template <typename SELF, typename FOLLOWERS>
class talker
{
public:
	using command_others = typename scheduler<FOLLOWERS>::command;
	using command_me = typename scheduler<SELF>::command;
	
	talker()
	{
		// CRTP
		_planner_me.add_follower(static_cast<SELF&>(*this));
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

	inline void call_me(const command_me& command, int milli = 0, int priority = 0)
	{
		_planner_me.call(command, milli, priority);
	}
	
	inline void call_others(const command_others& command, int milli = 0, int priority = 0)
	{
		_planner_others.call(command, milli, priority);
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

} // end namespace

class Context
{
public:
	explicit Context()
	{
		
	}
	
	~Context() { ; }

	inline void sleep(int milli)
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(milli) );
	}

	void print(const std::string& name, const std::string& text, int delay = 10)
	{
		std::unique_lock<std::mutex> context(_lock);

		std::cout << "<" << name << "> ";
		for(const char& c : text)
		{
			std::cout << c << std::flush;
			sleep(delay);
		}
		std::cout << std::endl;
	}

	lead::syncronizer& get_talking() { return _talking; }

protected:
	std::mutex _lock;
	lead::syncronizer _talking;
};

class PersonA;

class PersonB : public lead::talker<PersonB, PersonA>
{
public:
	explicit PersonB(const std::string& name, Context& context)
		: _name(name)
		, _context(context)
	{
		
	}
	
	~PersonB() { ; }
	
	void say(const std::string& text, int delay = 10)
	{
		_context.print(_name, text, delay);
	}
protected:
	Context& _context;
	std::string _name;
};


class PersonA : public lead::talker<PersonA, PersonB>
{
public:
	explicit PersonA(const std::string& name, Context& context)
		: _name(name)
		, _context(context)
	{
		
	}
	~PersonA() { ; }	
	
	void say(const std::string& text, int delay = 10)
	{
		_context.print(_name, text, delay);
	}
protected:
	Context& _context;
	std::string _name;
};

/*
lead::CommandTalker<Buyer> operator>>=(const lead::CompositeCommandTalker<Buyer>& a, const lead::CommandTalker<Buyer>& b)
{
	return a(b);
}

lead::CompositeCommandTalker<Buyer> repeat(int n)
{
	return [=](const lead::CommandTalker<Buyer>& f)
	{
		return [&](Buyer& self)
		{
			for(int cont = 0; cont < n; ++cont)
			{
				f(self);
			}
		};
	};
}
*/

int main()
{
	std::ios_base::sync_with_stdio(false);

	{
		Context context;
		PersonA person1("Person A", context);
		PersonB person2("Person B", context);
		/*
		 Person A: "What are you doing now?"
		 Person B: "I'm playing pool with my friends at a pool hall."
		 Person A: "I didn't know you play pool.  Are you having fun?"
		 Person B: "I'm having a great time.  How about you?  What are you doing?"
		 Person A: "I'm taking a break from my homework. There seems to be no end to the amount of work I have to do."
		 Person B: "I'm glad I'm not in your shoes."
		*/
		person1.call_me([&](PersonA& self) {
			self.say("What are you doing now ? ");
			self.sleep(100);
			context.get_talking().signal();
		});
		person2.call_me([&](PersonB& self) {
			context.get_talking().wait();
			self.say("I'm playing pool with my friends at a pool hall.");
			self.sleep(100);
			context.get_talking().signal();
		});
		person1.call_me([&](PersonA& self) {
			context.get_talking().wait();
			self.say("I didn't know you play pool.  Are you having fun?");
			self.sleep(100);
			context.get_talking().signal();
		});
		person2.call_me([&](PersonB& self) {
			context.get_talking().wait();
			self.say("I'm having a great time.  How about you?  What are you doing?");
			self.sleep(100);
			context.get_talking().signal();
		});
		person1.call_me([&](PersonA& self) {
			context.get_talking().wait();
			self.say("I'm taking a break from my homework.");
			self.say("There seems to be no end to the amount of work I have to do.");
			self.sleep(100);
			context.get_talking().signal();
		});
		person2.call_me([&](PersonB& self) {
			context.get_talking().wait();
			self.say("I'm glad I'm not in your shoes.");
			self.sleep(100);
		});
		
		
		for (int i = 0; i < 9000; ++i)
		{
			person1.update();
			person2.update();
			//
			person2.sleep(1);
		}
	}
	return(0);
}

