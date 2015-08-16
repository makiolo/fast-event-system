#include "fast-event-system/api.h"
#include "multithread/MultiThreading.h"
#include "multithread/Thread.h"

namespace asyncply {

#ifdef _WIN32

	DWORD thread::HandleGlobalMyThread(LPVOID parms)
	{
		thread* t = static_cast<thread*>(parms);
		t->execute();

		return TRUE;
	}

#else
	void* thread::HandleGlobalMyThread(void* parms)
	{
		thread* thread = static_cast<thread*>(parms);
		thread->execute();

#ifdef JOINABLE // Joinable
		pthread_exit(0);
#endif
		return NULL;
	}
#endif

}
