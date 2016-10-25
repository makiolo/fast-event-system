#ifndef _FES_API_H_
#define _FES_API_H_

#define fes_VERSION_MAJOR 1
#define fes_VERSION_MINOR 0
#define fes_VERSION ((fes_VERSION_MAJOR << 16) | fes_VERSION_MINOR)

#ifdef _WIN32
    #ifdef fes_EXPORTS
        #define fes_API __declspec(dllexport)
    #else
        #define fes_API __declspec(dllimport)
    #endif
#else
    #ifdef fes_EXPORTS
        #define fes_API __attribute__((visibility("default")))
    #else
        #define fes_API
    #endif
#endif

#ifdef _WIN32
using int64 = __int64;
using uint64 = unsigned __int64;
#else
using int64 = long long;
using uint64 = unsigned long long;
#endif

using int32 = signed int;
using int16 = signed short;
using int8 = signed char;

using uint32 = unsigned int;
using uint16 = unsigned short;
using uint8 = unsigned char;

using real64 = double;
using real32 = float;

using uint = uint32;
using real = real32;

#endif

