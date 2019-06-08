#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "api.h"
// win no
#include <chrono>

#ifdef _MSC_VER

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace fes {

	bool init_clock();
	using marktime = double;
	using deltatime = double;

}

#else  // gcc, clang ...

namespace fes {

	using clock_t = std::chrono::high_resolution_clock;
	// using clock_t = std::chrono::steady_clock;
	// using clock_t = std::chrono::system_clock;
	//
	using marktime = clock_t::time_point;
	using deltatime = std::chrono::milliseconds;

}

#endif

namespace fes {
	fes_API marktime high_resolution_clock();
}

#endif

