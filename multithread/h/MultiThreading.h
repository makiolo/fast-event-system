#ifndef _MULTITHREADING_H_
#define _MULTITHREADING_H_

#include <iostream>
#include <functional>
#include <iostream>
#include <algorithm>
#include <assert.h>
#if defined(LINUX) || defined(__APPLE__) || defined(__GNUC__)
#include <semaphore.h>
#include <pthread.h>
#ifndef LINUX
#define LINUX
#endif
#else
#include <process.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
#endif

#include <multithread/h/api.h>

namespace asyncply
{
class worker;
class thread;
class synchronizer;
class semaphore;
class pool_thread;
template <typename T>
class circular_queue;
class mutex;
class job;
class job_function;
template <typename T>
class scoped_lock;
}

#include "CircularQueue.h"
#include "PoolThread.h"
#include "Worker.h"
#include "Semaphore.h"
#include "Synchronizer.h"
#include "Thread.h"
#include "Mutex.h"
#include "Job.h"
#include "JobFunction.h"

#endif  // _MULTITHREADING_H_
