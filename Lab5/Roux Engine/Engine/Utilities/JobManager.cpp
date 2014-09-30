#include "JobManager.hpp"

#include <process.h>

#include "Engine/Utilities/NewAndDeleteOverrides.hpp"

//-----------------------------------------------------------------------------------------------
JobManager::JobManager()
{
	InitializeCriticalSection( &m_threadCS );
	InitializeCriticalSection( &m_generalCS );
	InitializeCriticalSection( &m_removeCS );
};

//-----------------------------------------------------------------------------------------------
JobManager::~JobManager()
{
	while( !m_workerThreads.empty() )
	{
		WorkerThread* threadToDelete = nullptr;
		if( m_workerThreads.back()->GetThreadStatus() == STATUS_READY_TO_CLOSE )
		{
			threadToDelete = m_workerThreads.back();
			m_workerThreads.pop_back();
			delete threadToDelete;
		}
	}
}

//-----------------------------------------------------------------------------------------------
void JobManager::AddJobToPendingList( Job* jobToAdd )
{
	EnterCriticalSection( &m_generalCS );
	{
		TypeOfWork jobType = jobToAdd->GetTypeOfWork();

		m_pendingJobQueues[ jobType ].push( jobToAdd );
	}
	LeaveCriticalSection( &m_generalCS );
}

//-----------------------------------------------------------------------------------------------
void JobManager::AddJobToRemovalList( Job* jobToRemove )
{
	EnterCriticalSection( &m_removeCS );
	{
		m_jobsToRemove.push_back( jobToRemove );
	}
	LeaveCriticalSection( &m_removeCS );
}

//-----------------------------------------------------------------------------------------------
void JobManager::CreateAndAddWorkerThreads( TypeOfWork threadTypeOfWork, int count )
{
	for( int i = 0; i < count; ++ i )
	{

		WorkerThread* newThread = new WorkerThread( threadTypeOfWork );
		m_workerThreads.push_back( newThread );

		_beginthread( &WorkerThread::EntryFunction, 0, static_cast< void* >( newThread ) );

	}
}

//-----------------------------------------------------------------------------------------------
void JobManager::Update()
{
	if( m_workerThreads.empty() )
	{
		Job* jobToExecute = GetJobWithAnyWorkType();

		if( jobToExecute )
		{
			jobToExecute->Execute();
			AddJobToRemovalList( jobToExecute );
		}
	}

	while( !m_jobsToRemove.empty() )
	{
		EnterCriticalSection( &m_removeCS );
		{
			Job* jobToDelete = m_jobsToRemove.back();
			jobToDelete->FireCallbackEvent();

			m_jobsToRemove.pop_back();
			delete jobToDelete;
		}
		LeaveCriticalSection( &m_removeCS );
	}
}

//-----------------------------------------------------------------------------------------------
Job* JobManager::GetJobWithWorkType( TypeOfWork typeOfWork )
{
	EnterCriticalSection( &m_generalCS );
		
		Job* result = nullptr;

		std::map< TypeOfWork, JobPriorityQueue >::iterator foundIter;

		foundIter = m_pendingJobQueues.find( typeOfWork );
		
		if( foundIter != m_pendingJobQueues.end() )
		{
			JobPriorityQueue& queueToGetJobFrom = foundIter->second;

			if( !queueToGetJobFrom.empty() )
			{
				result = queueToGetJobFrom.top();
				queueToGetJobFrom.pop();
			}
		}
	LeaveCriticalSection( &m_generalCS );

	return result;
}

//-----------------------------------------------------------------------------------------------
Job* JobManager::GetJobWithAnyWorkType()
{
	EnterCriticalSection( &m_generalCS );

	Job* result = nullptr;
	std::map< TypeOfWork, JobPriorityQueue >::iterator foundIter;

	for( int i = 0; i < NUM_WORK_TYPES; ++ i )
	{
		TypeOfWork typeOfWork = static_cast< TypeOfWork >( i );
		foundIter = m_pendingJobQueues.find( typeOfWork );

		if( foundIter != m_pendingJobQueues.end() )
		{
			JobPriorityQueue& queueToGetJobFrom = foundIter->second;

			if( !queueToGetJobFrom.empty() )
			{
				result = queueToGetJobFrom.top();
				queueToGetJobFrom.pop();
				break;
			}
		}
	}

	LeaveCriticalSection( &m_generalCS );

	return result;
}