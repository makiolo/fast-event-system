#include <multithread/h/api.h>
#include "multithread/h/MultiThreading.h"
#include "multithread/h/PoolThread.h"

namespace asyncply
{

pool_thread::pool_thread(uint32_t numThreads /*= 4*/)
    : _number_threads( std::min<uint32_t>(numThreads, THREADCOUNT_MAX) )
    , _started( false )
    , _queue(nullptr)
    , _any_job(nullptr)
    , _workers_finished(nullptr)
{

	assert(_number_threads > 0);
	assert(_number_threads <= THREADCOUNT_MAX);

	_started = false;
	for (uint32_t i = 0; i < _number_threads; ++i)
	{
		_workers[i] = new worker(this);
	}

	_queue = new circular_queue<job>();
	_any_job = new synchronizer(_number_threads);
	_workers_finished = new synchronizer(_number_threads);
}

pool_thread::~pool_thread(void)
{
	delete _workers_finished;
	delete _any_job;
	delete _queue;
	for (uint32_t i = 0; i < _number_threads; ++i)
	{
		delete _workers[i];
	}
}

void pool_thread::Start()
{
	if (!_started)
	{
		_started = true;

		for (uint32_t i = 0; i < _number_threads; ++i)
		{
#ifdef _WIN32
			_threads[i] = CreateThread(NULL,  // default security attributes
				0,							  // default stack size
				(LPTHREAD_START_ROUTINE)HandleGlobalMyPoolThread,
				_workers[i],	   // no thread function arguments
				0,				   // default creation flags
				&_threads_id[i]);  // receive thread identifier
#else
#ifdef JOINABLE  // Joinable
			pthread_create(&_threads[i], NULL, HandleGlobalMyPoolThread, _workers[i]);
#else			 // Detachable

			pthread_attr_t attr;  // thread attribute

			// set thread detachstate attribute to DETACHED
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

			pthread_create(&_threads[i], &attr, HandleGlobalMyPoolThread, _workers[i]);
#endif
#endif
		}

		// esperar a que los hilos terminen
		//
		_workers_finished->wait(_number_threads);
	}
}

void pool_thread::execute()
{
	// Empieza el pool (desde el hilo)
	Start();
}

void pool_thread::Stop()
{
	mutex::scoped_lock_t lock(_queue->get_mutex());

	// Despides a los trabajadores
	for (uint32_t i = 0; i < _number_threads; ++i)
	{
		_workers[i]->set_interrupted(true);
	}

	// Avisas a los trabajadores que no se han enterado por estar durmiendo
	for (uint32_t i = 0; i < _number_threads; ++i)
	{
		_any_job->signal();
	}

	// El hilo terminar� cuando todos los hilos terminen
	join();

	// ya no esta iniciado
	_started = false;
}

void pool_thread::submit(job* j)
{
	_queue->push(j);
	// avisar de que hay trabajo
	_any_job->signal();
}

#ifdef _WIN32

DWORD pool_thread::HandleGlobalMyPoolThread(LPVOID parms)
{
	worker* w = static_cast<worker*>(parms);

	// lo pone a trabajar
	w->execute();

	// avisa que el hilo a terminado
	w->get_owner()->get_synchronizer_workers_finished()->signal();

	return TRUE;
}

#else

void* pool_thread::HandleGlobalMyPoolThread(void* parms)
{
	worker* w = static_cast<worker*>(parms);

	// lo pone a trabajar
	w->execute();

	// avisa que el hilo a terminado
	w->get_owner()->get_synchronizer_workers_finished()->signal();

#ifdef JOINABLE  // Joinable
	pthread_exit(0);
#endif
	return NULL;
}

#endif
}
