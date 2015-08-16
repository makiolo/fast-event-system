#pragma once

namespace asyncply {

	class fast_event_system_API thread
	{
	public:
		thread(void)
		{

		}

		virtual ~thread(void)
		{

		}

		void Run()
		{
#ifdef _WIN32
			_thread = CreateThread(
				NULL,       // default security attributes
				0,          // default stack size
				(LPTHREAD_START_ROUTINE)HandleGlobalMyThread,
				this,	// no thread function arguments
				0,          // default creation flags
				&_thread_id); // receive thread identifier
#else

#ifdef JOINABLE // Joinable
			pthread_create(&_thread, NULL, HandleGlobalMyThread, this);
#else // Detachable

			pthread_attr_t attr; // thread attribute

			// set thread detachstate attribute to DETACHED
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

			pthread_create(&_thread, &attr, HandleGlobalMyThread, this);
#endif
#endif
		}

		void Join() const
		{
#ifdef _WIN32
			WaitForSingleObject(_thread, INFINITE);
			CloseHandle(_thread);
#else
#ifdef JOINABLE // joinable
			pthread_join(_thread, NULL);
#endif
#endif
		}

		virtual void execute() = 0;

	private:

#ifdef _WIN32
		/// @brief	Identificador del thread
		DWORD _thread_id;

		/// @brief Manejador del thread
		HANDLE _thread;
#else
		/// @brief Manejador del thread
		pthread_t _thread;
#endif

	public:

#ifdef _WIN32
		static DWORD HandleGlobalMyThread(LPVOID parms);
#else
		static void* HandleGlobalMyThread(void* parms);
#endif
	};

}
