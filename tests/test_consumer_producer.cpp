#include <iostream>
#include <vector>
#include <atomic>
#include <asyncply/h/parallel.h>
#include <asyncply/h/task.h>

int main(int, const char**)
{
	try
	{
		const size_t N = 1e4;
		std::atomic<size_t> f;
		f = 0;
		asyncply::parallel(
			[&]()
			{
				for (size_t i = 0; i < (N+1); ++i)
				{
					f += i;
				}
			},
			[&]()
			{
				for (size_t i = 0; i < N; ++i)
				{
					f -= i;
				}
			});

		std::cout << "f = " << f << std::endl;
		if(f != N)
		{
			throw std::runtime_error("Invalid value f");
		}

		// if(total)
		// 	std::cout << "yes" << std::endl;
		// else
		// 	std::cout << "no" << std::endl;

		// std::cout << "total = " << total << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "general exception " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
