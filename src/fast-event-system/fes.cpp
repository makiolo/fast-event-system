#include <fast-event-system/fes.h>

namespace fes {

#ifdef _WIN32

static double _freq;
static __int64 _counter_start;
static bool _init_clock = init_clock();
// 1.0 = seconds
// 1000.0 = milliseconds
// 1000000.0 = microseconds
// 1000000000.0 = nanoseconds
static double accuracy = 1000.0;

bool init_clock()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
	{
		std::cerr << "QueryPerformanceFrequency failed!\n";
	}

	_freq = double(li.QuadPart) / accuracy;

	QueryPerformanceCounter(&li);
	_counter_start = li.QuadPart;
	return true;
}

fes::marktime high_resolution_clock()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return fes::marktime(double(li.QuadPart - _counter_start) / _freq);
}

#else

fes::marktime high_resolution_clock()
{
	return std::chrono::high_resolution_clock::now();
}

#endif

}
