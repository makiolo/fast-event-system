#include <iostream>
#include <fes/h/fes.h>

int main(int, const char**)
{
	const int N = 9;
	int counter = 0;

	fes::async_fast<std::string> sync;

	// testing autoconnect
	sync.connect(sync);
	sync.connect([&counter](const std::string& str)
	{
		std::cout << str << std::endl;
		++counter;
	});

	sync("autoreference");
	for (int i = 0; i < N; ++i)
	{
		sync.dispatch_one();
	}

	return (counter == N ? 0 : 1);
}
