#ifndef JOB
#define JOB

#include <string>
#include <algorithm>

#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Utilities/NamedProperties.hpp"

enum TypeOfWork
{
	WORK_GENERIC,
	WORK_FILE_IO,
	NUM_WORK_TYPES
};

enum JobPriority
{
	PRIORITY_LOW,
	PRIORITY_AVERAGE,
	PRIORITY_HIGH,
	NUM_PRIORITY_TYPES
};

//-----------------------------------------------------------------------------------------------
class Job
{
public:

	Job( const std::string& eventNameToFireOnCallback, TypeOfWork typeOfWork = WORK_GENERIC, JobPriority priority = PRIORITY_AVERAGE );
	virtual ~Job();

	virtual void	Execute() = 0;
	virtual void	FireCallbackEvent();
	TypeOfWork		GetTypeOfWork() const;

	inline bool operator<( const Job& other ) const;

protected:

	JobPriority		m_priority;
	TypeOfWork		m_typeOfWork;
	std::string		m_eventNameToFireOnCallback;
	NamedProperties m_callbackProperties;
};

inline bool Job::operator <( const Job& other ) const
{
	if( m_priority < other.m_priority )
		return true;

	return false;
}

//-----------------------------------------------------------------------------------------------
class LoadFileJob : public Job
{
public:
	LoadFileJob( const std::string& fileName, const std::string& eventNameToFireOnCallback
		, TypeOfWork typeOfWork = WORK_FILE_IO, JobPriority priority = PRIORITY_AVERAGE );

	virtual void Execute();

private:

	std::string m_fileName;
};

//-----------------------------------------------------------------------------------------------
class HashBufferJob : public Job
{
public:
	HashBufferJob( const uchar* buffer, int bufferSize, const std::string& eventNameToFireOnCallback
		, TypeOfWork typeOfWork = WORK_GENERIC, JobPriority priority = PRIORITY_AVERAGE );
	
	virtual ~HashBufferJob();

	virtual void Execute();

private:

	const uchar* m_buffer;
	int m_bufferSize;
};

//-----------------------------------------------------------------------------------------------
class SaveFileJob : public Job
{
public:
	SaveFileJob( const std::string& fileName, const uchar* buffer, int bufferSize, const std::string& eventNameToFireOnCallback
		, TypeOfWork typeOfWork = WORK_FILE_IO, JobPriority priority = PRIORITY_AVERAGE );
	
	virtual ~SaveFileJob();

	virtual void Execute();

private:

	std::string m_fileName;
	const uchar* m_buffer;
	int m_bufferSize;
};

//-----------------------------------------------------------------------------------------------
class ReverseBufferJob : public Job
{

public:
	ReverseBufferJob( uchar* buffer, int bufferSize, const std::string& eventNameToFireOnCallback
		, TypeOfWork typeOfWork = WORK_GENERIC, JobPriority priority = PRIORITY_AVERAGE );

	virtual ~ReverseBufferJob();

	virtual void Execute();

private:

	uchar* m_buffer;
	int m_bufferSize;
};

#endif