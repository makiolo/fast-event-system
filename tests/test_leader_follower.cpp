// Ricardo Marmolejo Garcia
// 15-04-2015

#include <scheduler/sas.h>

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

	sas::syncronizer& talk() { return _talking; }

protected:
	std::mutex _lock;
	sas::syncronizer _talking;
};

class PersonA;

class PersonB : public sas::talker<PersonB, PersonA>
{
public:
	explicit PersonB(const std::string& name, Context& context)
		: _context(context)
		, _name(name)
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


class PersonA : public sas::talker<PersonA, PersonB>
{
public:
	explicit PersonA(const std::string& name, Context& context)
		: _context(context)
		, _name(name)
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

int main()
{
	std::ios_base::sync_with_stdio(false);
	
	{
		Context context;
		PersonA person1("Person A", context);
		PersonB person2("Person B", context);
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
		// person A
		person1.call_me([&](PersonA& self) {
			self.say("1. What are you doing now ? ");
			self.sleep(10);
			context.talk().signal();
		}, 0, 4);
		person1.call_me([&](PersonA& self) {
			context.talk().wait();
			self.say("3. I didn't know you play pool.  Are you having fun?");
			self.sleep(10);
			context.talk().signal();
		}, 0, 3);
		person1.call_me([&](PersonA& self) {
			context.talk().wait();
			self.say("5. I'm taking a break from my homework. There seems to be no end to the amount of work I have to do.");
			self.sleep(10);
			context.talk().signal();
		}, 0, 2);
		person1.call_me([&](PersonA& self) {
			context.talk().wait();
			self.say("7. bye person B");
			self.sleep(10);
			context.talk().signal();
		}, 0, 1);

		// person B
		person2.call_me([&](PersonB& self) {
			context.talk().wait();
			self.say("2. I'm playing pool with my friends at a pool hall.");
			self.sleep(10);
			context.talk().signal();
		}, 0, 4);
		person2.call_me([&](PersonB& self) {
			context.talk().wait();
			self.say("4. I'm having a great time.  How about you?  What are you doing?");
			self.sleep(10);
			context.talk().signal();
		}, 0, 3);
		person2.call_me([&](PersonB& self) {
			context.talk().wait();
			self.say("6. I'm glad I'm not in your shoes.");
			self.sleep(10);
			context.talk().signal();
		}, 0, 2);
		person2.call_me([&](PersonB& self) {
			context.talk().wait();
			self.say("8. bye person A");
			self.sleep(10);
		}, 0, 1);
		
		for (int i = 0; i < 9000; ++i)
		{
			person1.update();
			person2.update();
			person2.sleep(1);
		}
	}
	return(0);
}

/*
template <typename T> using CommandTalker = std::function<void(T&)>;
template <typename T> using CompositeCommandTalker = std::function<CommandTalker<T>(const CommandTalker<T>&)>;

sas::CommandTalker<Buyer> operator>>=(const sas::CompositeCommandTalker<Buyer>& a, const sas::CommandTalker<Buyer>& b)
{
	return a(b);
}

sas::CompositeCommandTalker<Buyer> repeat(int n)
{
	return [=](const sas::CommandTalker<Buyer>& f)
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
