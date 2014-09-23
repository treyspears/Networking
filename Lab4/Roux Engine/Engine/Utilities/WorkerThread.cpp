#define STATIC

#include "WorkerThread.hpp"

#include "Engine/Utilities/InputHandler.hpp"
#include "Engine/Utilities/JobManager.hpp"

//-----------------------------------------------------------------------------------------------
WorkerThread::WorkerThread( TypeOfWork typeOfWork /* = WORK_GENERIC */ )
	: m_typeOfWork( typeOfWork )
	, m_status( STATUS_IDLE )
	, m_currentJob( nullptr )
{

}

//-----------------------------------------------------------------------------------------------
WorkerThread::~WorkerThread()
{

}

//-----------------------------------------------------------------------------------------------
ThreadStatus WorkerThread::GetThreadStatus() const
{
	return m_status;
}

//-----------------------------------------------------------------------------------------------
STATIC void WorkerThread::EntryFunction( void* workerThreadObj )
{
	WorkerThread* asThread = static_cast< WorkerThread* >( workerThreadObj );

	asThread->Update();
}

//-----------------------------------------------------------------------------------------------
void WorkerThread::Update()
{
	while( !InputHandler::ShouldQuit() )
	{
		JobManager& jobManager = JobManager::GetInstance();

		if( m_status == STATUS_IDLE )
		{
			m_currentJob = jobManager.GetJobWithWorkType( m_typeOfWork );

			if( m_currentJob )
			{
				m_status = STATUS_BUSY;
			}
		}

		if( m_status == STATUS_BUSY )
		{
			m_currentJob->Execute();
			
			jobManager.AddJobToRemovalList( m_currentJob );
			m_currentJob = nullptr;
			m_status = STATUS_IDLE;
		}
		else
		{
			Sleep( 0 );
		}
	}
	m_status = STATUS_READY_TO_CLOSE;
}