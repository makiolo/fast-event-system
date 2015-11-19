#ifndef _FES_API_H_
#define _FES_API_H_

#define FES_VERSION_MAJOR 1
#define FES_VERSION_MINOR 0
#define FES_VERSION ((FES_VERSION_MAJOR << 16) | FES_VERSION_MINOR)

#ifdef _WIN32
    #ifdef fes_EXPORTS
        #define fes_API __declspec(dllexport)
    #else
        #ifndef fes_STATIC
            #define fes_API __declspec(dllimport)
        #else
            #define fes_API
        #endif
    #endif
#else
    #if __GNUC__ >= 4
        #define fes_API __attribute__((visibility("default")))
    #else
        #define fes_API
    #endif
#endif

#ifdef _WIN32
using Int64 = __int64;
using Uint64 = unsigned __int64;
#else
using Int64 = long long;
using Uint64 = unsigned long long;
#endif

using Int32 = signed int;
using Int16 = signed short;
using Int8 = signed char;

using Uint32 = unsigned int;
using Uint16 = unsigned short;
using Uint8 = unsigned char;

using Real64 = double;
using Real32 = float;

using Int = Int32;
using Uint = Uint32;
using Real = Real32;

#endif

