// Ricardo Marmolejo Garcia
// 15-04-2015

#include <iostream>
#include <thread>
#include <memory>
#include <asyncply/h/sas.h>
#include <animator/h/interpolation.h>
#include <Poco/Mutex.h>

class Context
{
public:
	explicit Context() {}

	~Context() { ; }

	inline void sleep(int milli) { std::this_thread::sleep_for(std::chrono::milliseconds(milli)); }

	void print(const std::string& name, const std::string& text, int delay = 10)
	{
		std::cout << "<" << name << "> ";
		for (char c : text)
		{
			std::cout << c << std::flush;
			sleep(delay);
		}
		std::cout << std::endl;
	}

protected:
};

class Person : public asyncply::talker<Person, Person>
{
public:
	explicit Person(const std::string& name, Context& context)
		: _name(name)
		, _context(context)
	{
	}
	virtual ~Person() { ; }

	void say(const std::string& text, int delay = 10) { _context.print(_name, text, delay); }

protected:
	std::string _name;
	Context& _context;
};

class test_exception : public std::exception
{
public:
	test_exception(const std::string m = "test fail")
		: msg(m)
	{
		;
	}
	virtual ~test_exception(void) { ; }
	const char* what() const noexcept { return msg.c_str(); }

private:
	std::string msg;
};

int main()
{
	std::ios_base::sync_with_stdio(false);

	Context context;
	Person person1("Person A", context);

	person1.call_me([&](Person& self)
		{
			self.say("1. What are you doing now ? ");
		});
	person1.call_me([&](Person& self)
		{
			self.say("2. I'm playing pool with my friends at a pool hall.");
		});
	person1.call_me([&](Person& self)
		{
			self.say("3. I didn't know you play pool.  Are you having fun?");
		});
	person1.call_me([&](Person& self)
		{
			self.say("4. I'm having a great time.  How about you?  What are you "
					 "doing?");
		});
	person1.call_me([&](Person& self)
		{
			self.say("5. I'm taking a break from my homework. There seems to be no "
					 "end to the amount of work I have to do.");
		});
	person1.call_me([&](Person& self)
		{
			self.say("6. I'm glad I'm not in your shoes.");
		});
	person1.call_me([&](Person& self)
		{
			self.say("7. bye person B");
		});
	person1.call_me([&](Person& self)
		{
			self.say("8. bye person A");
		});

	for (int i = 0; i < 500; ++i)
	{
		person1.update();
		context.sleep(10);
	}

	//Poco::ThreadPool::defaultPool().joinAll();
	return (0);
}

