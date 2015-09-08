#ifndef _FES_API_H_
#define _FES_API_H_

#if defined(_WIN32) || defined(WIN32)
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

#define FES_VERSION_MAJOR 1
#define FES_VERSION_MINOR 0
#define FES_VERSION ((FES_VERSION_MAJOR << 16) | FES_VERSION_MINOR)

#ifdef WINDOWS

typedef __int64 Int64;
typedef signed int Int32;
typedef signed short Int16;
typedef signed char Int8;

typedef unsigned __int64 UInt64;
typedef unsigned int Uint32;
typedef unsigned short Uint16;
typedef unsigned char Uint8;

typedef double Real64;
typedef float Real32;

#else

typedef long long Int64;
typedef signed int Int32;
typedef signed short Int16;
typedef signed char Int8;

typedef unsigned long long UInt64;
typedef unsigned int Uint32;
typedef unsigned short Uint16;
typedef unsigned char Uint8;

typedef double Real64;
typedef float Real32;

#endif

typedef Int32 Int;
typedef Uint32 Uint;
typedef Real32 Real;

#endif  // _FES_API_H_
