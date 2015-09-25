#include <iostream>
#include <complex>
#include <asyncply/h/sequence.h>

int main(int, const char **)
{
	try
	{
		for(int i=0; i<100;++i)
		{
			double total = 1.0;
			total = asyncply::sequence(total,
				[](double data)
				{
					return data + 2.0;
				},
				[](double data)
				{
					return data + 3.0;
				},
				[](double data)
				{
					return data + 4.0;
				},
				[](double data)
				{
					return data + 5.0;
				},
				[](double data)
				{
					return data + 6.0;
				}
			);
			if(std::abs(total - 21.0) > 1e-3)
			{
				std::cout << "not expected result" << std::endl;
				throw std::exception();
			}
			std::cout << "total: " << total << std::endl;
		}
		std::cout << "result ok" << std::endl;
	}
	catch(std::exception& e)
	{
		std::cout << "general exception " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

