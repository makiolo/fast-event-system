// Ricardo Marmolejo Garcia
// 15-04-2015

#include <thread>
#include <scheduler/sas.h>
#include <animator/interpolation.h>
#include <Poco/Mutex.h>

class Context
{
public:
	explicit Context()
	{
		
	}
	
	~Context() { ; }
	
	void sleep(int milli)
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(milli) );
	}

	void print(const std::string& name, const std::string& text, int delay = 10)
	{
		std::cout << "<" << name << "> ";
		for(const char& c : text)
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
	
	void say(const std::string& text, int delay = 10)
	{
		_context.print(_name, text, delay);
	}
protected:
	std::string _name;
	Context& _context;
};

int main()
{
	{
		//std::ios_base::sync_with_stdio(false);
		
		Context context;
		Person person1("Person A", context);
		
		/*
		Order expected:

		 Person A: "1. What are you doing now?"
		 Person B: "2. I'm playing pool with my friends at a pool hall."
		 Person A: "3. I didn't know you play pool.  Are you having fun?"
		 Person B: "4. I'm having a great time.  How about you?  What are you doing?"
		 Person A: "5. I'm taking a break from my homework. There seems to be no end to the amount of work I have to do."
		 Person B: "6. I'm glad I'm not in your shoes."
		 Person A: "7. Bye person B"
		 Person B: "8. Bye person A"
		*/
		
		person1.call_me([&](Person& self) {
			self.say("1. What are you doing now ? ");
		});
		person1.call_me([&](Person& self) {
			self.say("2. I'm playing pool with my friends at a pool hall.");
		});
		person1.call_me([&](Person& self) {
			self.say("3. I didn't know you play pool.  Are you having fun?");
		});
		person1.call_me([&](Person& self) {
			self.say("4. I'm having a great time.  How about you?  What are you doing?");
		});
		person1.call_me([&](Person& self) {
			self.say("5. I'm taking a break from my homework. There seems to be no end to the amount of work I have to do.");
		});
		person1.call_me([&](Person& self) {
			self.say("6. I'm glad I'm not in your shoes.");
		});
		person1.call_me([&](Person& self) {
			self.say("7. bye person B");
		});
		person1.call_me([&](Person& self) {
			self.say("8. bye person A");
		});
		
		for (int i = 0; i < 500; ++i)
		{
			person1.update();
			context.sleep(10);
		}
	}
	Poco::ThreadPool::defaultPool().joinAll();
	return(0);
}

int main2()
{
	{
		asyncply::parallel(
			[]() {
				std::cout << "Hola mundo 11111" << std::endl;
			},
			[]() {
				std::cout << "Hola mundo 22222" << std::endl;
			},
			[]() {
				std::cout << "Hola mundo 3333333" << std::endl;
			},
			[]() {
				std::cout << "Hola mundo 444444" << std::endl;
			}
		);
	}
	Poco::ThreadPool::defaultPool().joinAll();
	return 0;
}

