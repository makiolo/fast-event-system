#include <boost/bind.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <coro/pipeline.h>
#include <coro/cmd.h>
#include <coro/behaviour_tree.h>

int main()
{
	std::cout.sync_with_stdio(false);
	//
	std::string cls;
	cmd({
		find("../../tests"),
		grep(".*\\.cpp$|.*\\.h$"),
		cat(),
		grep("class|struct"),
		grep_v("enable_if|;"),
		uniq(),
		trim(),
		cut(" ", 1),
		trim(),
		quote(),
		join(" "),
		write(cls)
	});
	std::cout << "class " << cls << std::endl;
	//
	behaviour({
		_jump(),
		_for(3),
	});
}

