#include <multithread/h/api.h>
#include "multithread/h/MultiThreading.h"
#include "multithread/h/Semaphore.h"

namespace asyncply
{

#if defined(__APPLE__)
#include <sstream>  // std::stringstream
int semaphore::_count_sem = 0;
#endif

semaphore::semaphore(uint32_t concurrency, bool isForSync)
    : _sem()
    , _concurrency( concurrency )
{
	assert(_concurrency > 0);  // "La concurrencia es negativa o cero");

#if defined(LINUX)

	(void)sem_init(&_sem, 0, _concurrency);

	// En MAC y Linux siempre se inicia vacio
	if (!isForSync)
	{
		// hay que llenar el semaforo
		for (uint32_t i(0); i < _concurrency; ++i)
			unlock();
	}

#elif defined(__APPLE__)

	std::stringstream ss;
	ss << "/semaphore" << _count_sem++ << std::endl;
	_name_sem = ss.str();

	_sem = sem_open(_name_sem.c_str(), O_CREAT, 0644, _concurrency);

	// En MAC y Linux siempre se inicia vacio
	if (!isForSync)
	{
		// hay que llenar el semaforo
		for (uint32_t i(0); i < _concurrency; ++i)
			unlock();
	}

#else

	if (!isForSync)
	{
		// un semaforo normal esta lleno
		_semaphore = CreateSemaphore(NULL, _concurrency, _concurrency, NULL);
	}
	else
	{
		// un semaforo para sincronizar empieza vacio
		_semaphore = CreateSemaphore(NULL, 0, _concurrency, NULL);
	}
#endif
}

semaphore::~semaphore()
{
#if defined(LINUX)
	(void)sem_destroy(&_sem);
#elif defined(__APPLE__)
	int code1 = sem_close(_sem);
	int code2 = sem_unlink(_name_sem.c_str());
	assert(code1 != -1);
	assert(code2 != -1);
#else
	CloseHandle(_semaphore);
#endif
}

#ifdef _WIN32

#ifdef _DEBUG

void PrintLastError()
{
	LPWSTR lpMsgBuf;
	DWORD dw = GetLastError();

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);

	printf("Error: %S", lpMsgBuf);

	LocalFree(lpMsgBuf);
}

#endif

#endif
}
