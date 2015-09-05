#include <iostream>
#include <fes/h/fes.h>

int main(int, const char**)
{
	fes::async_delay<int, std::string, double> sync;
	bool is_dispatched = false;

	// test connect in context
	{
		fes::connection<int, std::string, double> conn(
			sync.connect(
				[&is_dispatched](int n, const std::string& str, double r)
				{
					std::cout << "received message: " << std::endl;
					std::cout << "n = " << n << std::endl;
					std::cout << "str = " << str << std::endl;
					std::cout << "r = " << r << std::endl;
					if(str == "hello world")
					{
						is_dispatched = true;
					}
					else if(str == "kill")
					{
						exit(1);
					}
				}
			)
		);
		// lambda must received this
		sync(fes::deltatime(2000), 5, "hello world", 11.0);
		sync.update_while( fes::deltatime(2001) );

		// autodisconnection
	}
	// kill only if autodisconnection failed
	sync(6, "kill", 12.0);
	sync.update_while( fes::deltatime(1) );

	return is_dispatched ? 0 : 1;
}

