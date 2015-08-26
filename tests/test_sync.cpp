#include <iostream>
#include <fast-event-system/fes.h>

int main(int, const char**)
{
	fes::sync<int, std::string, double> _sync;

	// test connect in context
	{
		// TODO: operator=
		fes::connection<int, std::string, double> conn(
			_sync.connect(
				[](int n, const std::string& str, double r)
				{
					std::cout << "Se ha recibido el mensaje: " << std::endl;
					std::cout << "n = " << n << std::endl;
					std::cout << "str = " << str << std::endl;
					std::cout << "r = " << r << std::endl;
					if(str == "kill")
					{
						exit(1);
					}
				}		
			)
		);
		// lambda must received this
		_sync(5, "hello world", 11.0);

		// autodisconnection
	}
	// kill only if autodisconnection failed
	_sync(6, "kill", 12.0);
	return 0;
}

