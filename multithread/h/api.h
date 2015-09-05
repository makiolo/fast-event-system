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
#define multithread_API
#endif

#ifdef WINDOWS

typedef __int64				Int64;
typedef signed int			Int32;
typedef signed short		Int16;
typedef signed char			Int8;

typedef unsigned __int64	UInt64;
typedef unsigned int		Uint32;
typedef unsigned short		Uint16;
typedef unsigned char		Uint8;

typedef double				Real64;
typedef float				Real32;

#else

typedef long long			Int64;
typedef signed int			Int32;
typedef signed short		Int16;
typedef signed char			Int8;

typedef unsigned long long	UInt64;
typedef unsigned int		Uint32;
typedef unsigned short		Uint16;
typedef unsigned char		Uint8;

typedef double				Real64;
typedef float				Real32;

#endif

#endif // _FES_API_H_
