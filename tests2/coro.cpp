#include <boost/bind.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/filesystem.hpp>
#include <coro/pipeline.h>
#include <coro/cmd.h>
#include <asyncply/h/parallel.h>

int main()
{
	std::cout.sync_with_stdio(false);

	// using coro = boost::coroutines::coroutine<StatusCode()>;
	// using yield_type = typename coro::caller_type;
    //
	// coro prev(
	// 	[](yield_type& ret)
	// 	{
	// 		std::cout << "Hola, soy el primero del pipeline" << std::endl;
	// 		ret(ABORTED);
	// 	}
	// );
	// coro f(
	// 	[&prev](yield_type& ret)
	// 	{
	// 		// ignore source
	// 		std::cout << "hola, soy el segundo y recibo " << prev.get() << std::endl;
	// 		ret(COMPLETED);
	// 	}
	// );
	// while(f)
	// {
	// 	f();
	// }

	// pipelines in parallel
	try
	{
		auto result = asyncply::parallel(
			[](){
				return "one";
			},
			[](){
				cmd({
					find(".."),
					grep(".*\\.cpp$|.*\\.h$"),
					cat(),
					grep("class|struct|typedef|using|void|int|double|float"),
					grep_v("enable_if|;|\"|\'"),
					trim(),
					split(" "),
					uniq(),
					join(" "),
					out()
				});
				return "two";
			}
		);
		std::cout << result.size() << std::endl;
		for(auto& r : result)
		{
			std::cout << r << std::endl;
		}
	}
	catch(boost::filesystem::filesystem_error& e)
	{
		std::cout << "exception: " << e.what() << std::endl;
	}
}

