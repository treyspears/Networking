#ifndef WORKER_THREAD
#define WORKER_THREAD

#include <Windows.h>

#include "Engine/Utilities/Job.hpp"

enum ThreadStatus
{
	STATUS_IDLE,
	STATUS_BUSY,
	STATUS_READY_TO_CLOSE,
	NUM_THREAD_STATUS
	
};

class WorkerThread
{

public:

	WorkerThread( TypeOfWork typeOfWork = WORK_GENERIC );
	~WorkerThread();

	static void		EntryFunction( void* workerThreadObj );
	void			Update();
	TypeOfWork		GetThreadWorkType() const;
	ThreadStatus	GetThreadStatus() const;

private:

	ThreadStatus m_status;
	TypeOfWork	 m_typeOfWork;
	Job*		 m_currentJob;
};
#endif