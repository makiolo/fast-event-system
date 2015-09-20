#include <asyncply/h/sas.h>

int main(int, const char **)
{
	for(int i=0; i<100;++i)
	{
		std::vector<std::shared_ptr<asyncply::task<double> > > vjobs;
		asyncply::parallel(vjobs,
		   [&]()
		   {
			   return 8.0;
		   },
		   [&]()
		   {
			   return 8.0;
		   },
		   [&]()
		   {
			   return 8.0;
		   },
		   [&]()
		   {
			   return 8.0;
		   });
		double aggregation = 0.0;
		for (auto& job : vjobs)
		{
			try
			{
				double partial = job->get();
				aggregation += partial;
			}
			catch (std::exception& e)
			{
				std::cout << "exception: " << e.what() << std::endl;
			}
		}
		if(std::abs(aggregation - 32.0) > 1e-3)
		{
			std::cout << "invalid total " << aggregation << std::endl;
			throw std::exception();
		}
	}
	std::cout << "result ok" << std::endl;
	Poco::ThreadPool::defaultPool().joinAll();
	return 0;
}

