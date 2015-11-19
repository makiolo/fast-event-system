#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <chrono>
#include <api.h>

namespace fes {

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

bool init_clock();
using marktime = double;
using deltatime = double;

#else  // gcc, clang ...

// using clock_t = std::chrono::high_resolution_clock;
// using clock_t = std::chrono::steady_clock;
using clock_t = std::chrono::system_clock;
//
using marktime = clock_t::time_point;
using deltatime = std::chrono::milliseconds;

#endif

fes_API marktime high_resolution_clock();

}

#endif

