#include <iostream>
#include <vector>
#include <asyncply/h/parallel.h>
#include <asyncply/h/task.h>

int main(int, const char **)
{
	try
	{
		for(int i=0; i<100;++i)
		{
			std::vector<std::shared_ptr<asyncply::task<double> > > vjobs;
			asyncply::parallel(vjobs,
			   []()
			   {
				   return 9.0;
			   },
			   []()
			   {
				   return 7.0;
			   },
			   []()
			   {
				   return 10.0;
			   },
			   []()
			   {
				   return 6.0;
			   });
			double aggregation = 0.0;
			for (auto& job : vjobs)
			{
				try
				{
					aggregation += job->get();
				}
				catch (std::exception& e)
				{
					std::cout << "exception: " << e.what() << std::endl;
					throw;
				}
			}
			if(std::abs(aggregation - 32.0) > 1e-3)
			{
				std::cout << "invalid total " << aggregation << std::endl;
				return 1;
			}
			std::cout << "total " << aggregation << std::endl;
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

