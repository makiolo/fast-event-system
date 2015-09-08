#ifndef _MULTITHREAD_API_H_
#define _MULTITHREAD_API_H_

#if defined(_WIN32) || defined(WIN32)
#ifdef multithread_EXPORTS
#define multithread_API __declspec(dllexport)
#else
#ifndef multithread_STATIC
#define multithread_API __declspec(dllimport)
#else
#define multithread_API
#endif
#endif
#else
#if __GNUC__ >= 4
#define multithread_API __attribute__((visibility("default")))
#else
#define multithread_API
#endif
#endif

namespace asyncply
{

#ifdef WINDOWS

typedef __int64 int64_t;
typedef signed int int32_t;
typedef signed short int16_t;
typedef signed char int8_t;

typedef unsigned __int64 uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef double real64_t;
typedef float real32_t;

#else

typedef long long int64_t;
typedef signed int int32_t;
typedef signed short int16_t;
typedef signed char int8_t;

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef double real64_t;
typedef float real32_t;

#endif
}

#endif  // _FES_API_H_
