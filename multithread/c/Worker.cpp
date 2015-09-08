#include <multithread/h/api.h>
#include "multithread/h/MultiThreading.h"
#include "multithread/h/Worker.h"

namespace asyncply
{

void worker::execute()
{
	circular_queue<job>* queue = _owner->get_queue();
	while (!_interrupted)
	{
		job* pendingJob = queue->pop();
		if (pendingJob)
		{
			pendingJob->Execute();
		}
		else
		{
			// no hay trabajo, esperar a que haya trabajo
			// Tambien puede ser que me despierten porque me han interrumpido
			_owner->_any_job->wait();
		}
	}
}
}
