#include <asyncply/h/sas.h>

int main(int, const char **)
{
	for(int i=0; i<100;++i)
	{
		template <typename T> using shared_task = std::shared_ptr<asyncply::task<T> >;
		std::vector<shared_task<double> > vjobs;
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
			throw std::exception();
		}
		std::cout << "total " << aggregation << std::endl;
	}
	std::cout << "result ok" << std::endl;
	return 0;
}

