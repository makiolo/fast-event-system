#ifndef _MULTITHREADING_H_
#define _MULTITHREADING_H_

#include <iostream>
#include <functional>
#include <iostream>
#include <algorithm>
#include <assert.h>
#if defined(LINUX) || defined(__APPLE__)
#include <semaphore.h>
#include <pthread.h>
#else
#include <process.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
#endif

#include "fast-event-system/api.h"

namespace asyncply {
	class worker;
	class thread;
	class synchronizer;
	class semaphore;
	template <typename T> class scoped_lock;
	class pool_thread;
	template <typename T> class circular_queue;
	class mutex;
	class job;
	class job_function;
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

#endif // _MULTITHREADING_H_
