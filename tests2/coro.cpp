#include <boost/bind.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/filesystem.hpp>
#include <coro/pipeline.h>
#include <coro/cmd.h>
#include <asyncply/h/parallel.h>
#include <fes/h/async_fast.h>

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

	std::vector<std::shared_ptr<asyncply::coro<int> > > coros;
	for(int i=1; i<=10; ++i)
	{
		coros.emplace_back(asyncply::corun<int>(
			[i](asyncply::yield_type<int>& yield)
			{
				std::cout << "create " << i << std::endl;
				yield(0);
				std::cout << "download " << i << std::endl;
				yield(0);
				std::cout << "patching " << i << std::endl;
				yield(0);
				std::cout << "compile " << i << std::endl;
				yield(0);
				std::cout << "tests " << i << std::endl;
				yield(0);
				std::cout << "packing " << i << std::endl;
				yield(0);
			}
		));
	}
	bool pipeline = false;
	if(!pipeline)
	{
		for(auto& c : coros)
		{
			while(*c)
			{
				(*c)();
			}
		}
	}
	else
	{
		bool all_finished = false;
		while(!all_finished)
		{
			all_finished = true;
			for(auto& c : coros)
			{
				if(*c)
				{
					(*c)();
					all_finished = false;
				}
			}
		}
	}


	fes::async_fast< std::shared_ptr<asyncply::coro<int> > > _channel;
	std::atomic<bool> _exit;
	_exit = false;
	asyncply::parallel(
			[&](){
				while(!_exit)
				{
					_channel.connect([&](const std::shared_ptr<asyncply::coro<int> >& coro) {
						while(*coro)
						{
							int x = coro->get();
							_exit = (x == 1);
							(*coro)();
						}
					});
					_channel.update();
				}
			},
			[&](){
				asyncply::run(
					[&](){
						for(int i=0; i<1000; ++i)
						{
							_channel(asyncply::corun<int>(
								[](asyncply::yield_type<int>& yield)
								{
									std::cout << "create " << std::endl;
									yield(0);
									std::cout << "download " << std::endl;
									yield(0);
									std::cout << "patching " << std::endl;
									yield(0);
									std::cout << "compile " << std::endl;
									yield(0);
									std::cout << "tests " << std::endl;
									yield(0);
									std::cout << "packing " << std::endl;
									yield(0);
								}
							));
						}
					}
					,
					[&](){
						_channel(asyncply::corun<int>(
							[](asyncply::yield_type<int>& yield)
							{
								std::cout << "request exit" << std::endl;
								yield(1);
							}
						));
					}
				);
			},
			[&](){
				_channel(asyncply::corun<int>(
					[](asyncply::yield_type<int>& yield)
					{
						std::cout << "step1 - thread3 " << std::endl;
						yield(0);
						std::cout << "step2 - thread3 " << std::endl;
						yield(0);
						std::cout << "step3 - thread3 " << std::endl;
						yield(0);
					}
				));
			}
	);

	// pipelines in parallel
	// try
	// {
	// 	auto result = asyncply::parallel(
	// 		[](){
	// 			return "one";
	// 		},
	// 		[](){
	// 			cmd({
	// 				find(".."),
	// 				grep(".*\\.cpp$|.*\\.h$"),
	// 				cat(),
	// 				grep("class|struct|typedef|using|void|int|double|float"),
	// 				grep_v("enable_if|;|\"|\'"),
	// 				trim(),
	// 				split(" "),
	// 				uniq(),
	// 				join(" "),
	// 				out()
	// 			});
	// 			return "two";
	// 		}
	// 	);
	// 	std::cout << result.size() << std::endl;
	// 	for(auto& r : result)
	// 	{
	// 		std::cout << r << std::endl;
	// 	}
	// }
	// catch(boost::filesystem::filesystem_error& e)
	// {
	// 	std::cout << "exception: " << e.what() << std::endl;
	// }
}

