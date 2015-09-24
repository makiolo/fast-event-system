// Ricardo Marmolejo Garcia
// 15-04-2015
#ifndef _SAS_H_
#define _SAS_H_

#ifdef _WIN32
#include <windows.h>
#endif
#include <functional>
#include <future>
#include <atomic>
#include <mutex>
#include <csignal>
#include <exception>
//
#include <animator/h/interpolation.h>
//
#include <Poco/Thread.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>
//
#include <fes/h/async_delay.h>
//
#include <asyncply/h/allocator.h>
#include <asyncply/h/future.h>
#include <asyncply/h/promise.h>
#include <asyncply/h/task.h>
#include <asyncply/h/run.h>
#include <asyncply/h/parallel.h>
#include <asyncply/h/sequence.h>
#include <asyncply/h/commands_queue.h>
#include <asyncply/h/animations_queue.h>
#include <asyncply/h/talker.h>

#endif  // _SCHEDULER_ADVANCED_SIMPLE_
