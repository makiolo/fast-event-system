// Ricardo Marmolejo Garcia
// 15-04-2015

#include <thread>
#include <memory>
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
	
	inline void sleep(int milli)
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

int main2()
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

int main()
{
#if 0
	{

		struct functor_raii
		{
			double operator()()
			{
				std::cout << "hello world" << std::endl;
				return 2.0;
			}
		};

		auto job = asyncply::run( functor_raii() );
		std::cout << "return: " << job->wait() << std::endl;
	}
#endif
#if 1
	{
		asyncply::sequence(
			[=]() -> int {
				std::cout << "main" << std::endl;
				return 1;
			},
			[=](int data) -> double {
				std::cout << "step 1, initial data = " << data << std::endl;
				std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
				return 1.0;
			},
			[=](double data) -> std::string {
				std::cout << "recibido " << data << std::endl;
				std::cout << "step 2" << std::endl;
				std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
				return "hello world";
			},
			[=](const std::string& data) {
				std::cout << "recibido " << data << std::endl;
				std::cout << "step 3" << std::endl;
				std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
			}
		);
	}
#endif
#if 0
	{
		auto job1 = asyncply::run(
			[=]() {
				std::cout << "step 1" << std::endl;
				std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
				return 1.0;
			}
		);
		job1->then(
			[=](double data){
				//////////////////////////////	
				auto job2 = asyncply::run(
					[=]() -> std::string {
						std::cout << "recibido " << data << std::endl;
						std::cout << "step 2" << std::endl;
						std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
						return "hello world";
					}
				);
				job2->then(
					[=](const std::string& data) {
						/////////////////////
						auto job3 = asyncply::run(
							[=]() {
								std::cout << "recibido " << data << std::endl;
								std::cout << "step 3" << std::endl;
								std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
							}
						);
						// no next
						//  wait 3
						job3->wait();
						////////////////////
					}
				);
				//  wait to 2 and 3
				job2->wait();
				/////////////////////////////
			}
		);
		//  wait to 1, 2 and 3
		job1->wait();
	}
	{
		auto vjobs = asyncply::parallel(
			[]() {
				std::this_thread::sleep_for( std::chrono::milliseconds(2000) );
				return 1.0;
			}
			,[]() {
				std::this_thread::sleep_for( std::chrono::milliseconds(2000) );
				return 2.0;
			},
			[]() {
				std::this_thread::sleep_for( std::chrono::milliseconds(2000) );
				return 3.0;
			},
			[]() {
				std::this_thread::sleep_for( std::chrono::milliseconds(2000) );
				return 4.0;
			}
		);
		for(auto& job : vjobs)
		{
			std::cout << "end with value = " << job->wait() << std::endl;
		}
	}
#endif
	Poco::ThreadPool::defaultPool().joinAll();
	return 0;
}

