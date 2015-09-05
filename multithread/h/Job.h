#ifndef _JOB_H_
#define _JOB_H_

namespace asyncply {

	class multithread_API job
	{
	public:
		// http://www.codeproject.com/Articles/7933/Smart-Thread-Pool
		enum StateJob
		{
			IN_QUEUE,
			// valores de retorno válidos
			RUNNING,
			COMPLETED,
			CANCELED
		};

		job() : _state(IN_QUEUE) {}
		virtual ~job(void) {}

		/*
		Metodo llamado por el trabajador
		*/
		void Execute();

	protected:

		// Empieza el trabajo
		virtual void Start() {};

		// Ejecucion del trabajo (sliced)
		virtual StateJob Update() = 0;

		// Termina el trabajo
		virtual void Finish() {};

	protected:

		/*
		Estado del trabajo
		*/
		StateJob _state;

	};

}

#endif

