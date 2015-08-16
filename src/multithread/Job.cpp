#include "fast-event-system/api.h"
#include "multithread/MultiThreading.h"
#include "multithread/Job.h"

namespace asyncply {

	void job::Execute()
	{
		if (_state == IN_QUEUE)
		{
			// empieza
			Start();
			_state = RUNNING;
		}
		
		do
		{
			// avanzar el algoritmo sliced
			_state = Update();
			
			// dar oportunidad a otros hilos
			//pthread_yield_np();
		
		} while (_state == RUNNING);
		
		// termina
		Finish();
	}
}
