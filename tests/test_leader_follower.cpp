// Ricardo Marmolejo Garcia
// 15-04-2015

#include <thread>
#include <memory>
#include <scheduler/h/sas.h>
#include <animator/h/interpolation.h>
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

class test_exception : public std::exception
{
public:
	test_exception(const std::string m = "test fail") : msg(m) { ; }
	virtual ~test_exception(void) { ; }
	const char* what() const noexcept
	{ return msg.c_str(); }
private:
	std::string msg;
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
	Poco::ThreadPool::defaultPool().addCapacity(32);

	{
#if 1
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
		std::cout << "return: " << job->get() << std::endl;
	}
#endif
#if 1
	{
		int resul = asyncply::sequence(0,
			[](int data) {
				std::cout << "recibido " << data << std::endl;
				std::cout << "step 1" << std::endl;
				std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
				return 1;
			},
			[](int data) {
				std::cout << "recibido " << data << std::endl;
				std::cout << "step 2" << std::endl;
				std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
				return 2;
			},
			[](int data) {
				std::cout << "recibido " << data << std::endl;
				std::cout << "step 3" << std::endl;
				std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
				return 3;
			},
			[](int data) {
				std::cout << "recibido " << data << std::endl;
				std::cout << "step 4" << std::endl;
				std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
				return 4;
			}
		);
		std::cout << "resul final " << resul << std::endl;
	}
#endif
#if 1
	double total = 0.0;
	for(int i=0; i<100; ++i)
	{
		total = asyncply::sequence(total, // initial value in subprocess
			[](double data) {
				return data + 1.0;
			},
			[](double data) {
				return data + 1.0;
			},
			[](double data) {
				return data + 1.0;
			},
			[](double data) {
				return data + 1.0;
			},
			[](double data) {
				return data + 1.0;
			}
		);
	}
	std::cout << "total = " << total << std::endl;
#endif
#if 1
	std::atomic<int> count;
	count = 0;
	{
		fes::async_fast<int, std::string> c(1000);

		{
			std::vector<std::shared_ptr<asyncply::task<void> > > vjobs;
			vjobs.reserve(4);
			asyncply::parallel(vjobs,
					[&]()
					{
						for (int i = 0; i < 3; ++i)
						{
							// productor
							c(i, "saludo de t1");
						}
					},
					[&]()
					{
						for (int i = 0; i < 3; ++i)
						{
							// productor
							c(i, "saludo de t2");
						}
					},
					[&]()
					{
						c.connect([&](int iter, const std::string& data) {
							std::cout << "recibido en t3 " << data << ", iter " << iter << std::endl;
							++count;
						});
					},
					[&]()
					{
						c.connect([&](int iter, const std::string& data) {
							std::cout << "recibido en t4 " << data << ", iter " << iter << std::endl;
							++count;
						});
					}
			);
		}
		c.update_while(fes::deltatime(2000));
	}
	std::cout << "total = " << count << std::endl;
	if(count != 12)
	{
		throw test_exception();
	}
#endif
#if 1

	std::vector<std::shared_ptr<asyncply::task<double> > > vjobs;
	vjobs.reserve(5);
	asyncply::parallel(vjobs,
		[&]() {
			return asyncply::sequence(1.0, 
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				}
			);
		},
		[&]() {
			return asyncply::sequence(1.0,
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				}
			);
		},
		[&]() {
			return asyncply::sequence(1.0,
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				}
			);
		},
		[&]() {
			return asyncply::sequence(1.0,
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				}
			);
		},
		[&]() {
			return asyncply::sequence(1.0,
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				},
				[](double data) {
					return data + 1.0;
				}
			);
		}
	);
	double aggregation = 0.0;
	for(auto& job : vjobs)
	{
		try
		{
			double partial = job->get();
			std::cout << "partial " << partial << std::endl;
			aggregation += partial;
		}
		catch(std::exception& e)
		{
			std::cout << "exception: " << e.what() << std::endl;
		}
	}
	std::cout << "total = " << aggregation << std::endl;
	#endif
	}
	Poco::ThreadPool::defaultPool().joinAll();
	return 0;
}

