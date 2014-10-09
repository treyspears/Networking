#ifndef JOB_MANAGER
#define JOB_MANAGER

#include <queue>
#include <vector>
#include <map>
#include <Windows.h>

#include "Engine/Utilities/Job.hpp"
#include "Engine/Utilities/WorkerThread.hpp"

typedef std::priority_queue< Job*, std::vector< Job* > > JobPriorityQueue;

//-----------------------------------------------------------------------------------------------
class JobManager
{
public:

	static JobManager& GetInstance()
	{
		static JobManager instance; 

		return instance;
	}

	void AddJobToPendingList( Job* jobToAdd );
	void AddJobToRemovalList( Job* jobToRemove );
	void CreateAndAddWorkerThreads( TypeOfWork threadTypeOfWork, int count );
	void Update();
	Job* GetJobWithWorkType( TypeOfWork typeOfWork = WORK_GENERIC );
	Job* GetJobWithAnyWorkType();

private:

	JobManager();
	~JobManager();

	JobManager( JobManager const& );              
	void operator=( JobManager const& ); 

	std::map< TypeOfWork, JobPriorityQueue > m_pendingJobQueues;
	std::vector< Job* >						 m_jobsToRemove;
	std::vector< WorkerThread* >			 m_workerThreads; 

	mutable CRITICAL_SECTION				 m_threadCS;
	mutable CRITICAL_SECTION				 m_removeCS;
	mutable CRITICAL_SECTION				 m_generalCS;
};

#endif