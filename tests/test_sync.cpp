#include <iostream>
#include <fes/h/fes.h>

int main(int, const char**)
{
	// asyncply::run( functor )
	// asyncply::sequence( functor, functor ... )
	// asyncply::parallel( functor, functor ... )
	//
	// safe vs unsafe
	// safe vs fast
	// inter vs intra
	// fes::channel::safe::sync            // (future/promise)
	// fes::channel::safe::async
	// fes::channel::unsafe::sync
	// fes::channel::unsafe::async         (delay)
	//

	fes::sync<int, std::string, double> sync;

	// test connect in context
	{
		// TODO: operator=
		fes::connection<int, std::string, double> conn(
			sync.connect([](int n, const std::string& str, double r)
				{
					std::cout << "received message: " << std::endl;
					std::cout << "n = " << n << std::endl;
					std::cout << "str = " << str << std::endl;
					std::cout << "r = " << r << std::endl;
					if (str == "kill")
					{
						exit(1);
					}
				}));
		// lambda must received this
		sync(5, "hello world", 11.0);

		// autodisconnection
	}
	// kill only if autodisconnection failed
	sync(6, "kill", 12.0);
	return 0;
}
