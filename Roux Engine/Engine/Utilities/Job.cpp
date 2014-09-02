#include "Job.hpp"

#include <cstring>

#include "Engine/Utilities/EventSystem.hpp"

#include "Engine/Utilities/NewAndDeleteOverrides.hpp"

//-----------------------------------------------------------------------------------------------
Job::Job( const std::string& eventNameToFireOnCallback, TypeOfWork typeOfWork, JobPriority priority )
	: m_eventNameToFireOnCallback( eventNameToFireOnCallback )
	, m_typeOfWork( typeOfWork )
	, m_priority( priority )
	, m_callbackProperties( NamedProperties::EmptyProperties() )
{

}

Job::~Job()
{

}

//-----------------------------------------------------------------------------------------------
TypeOfWork Job::GetTypeOfWork() const
{
	return m_typeOfWork;
}

void Job::FireCallbackEvent()
{
	EventSystem& eventSystem = EventSystem::GetInstance();

	eventSystem.FireEvent( m_eventNameToFireOnCallback, m_callbackProperties ); 
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
HashBufferJob::HashBufferJob( const uchar* buffer, int bufferSize, const std::string& eventNameToFireOnCallback , TypeOfWork typeOfWork, JobPriority priority )
	: m_buffer( buffer )
	, m_bufferSize( bufferSize )
	, Job( eventNameToFireOnCallback, typeOfWork, priority )
{

}

//-----------------------------------------------------------------------------------------------
HashBufferJob::~HashBufferJob()
{
	m_buffer = nullptr;
	delete m_buffer;
}

//-----------------------------------------------------------------------------------------------
void HashBufferJob::Execute()
{
	uint hashValue = 0;
	for( uint i = 0; i < static_cast< uint >( m_bufferSize ); ++ i )
	{
		hashValue &= 0x07ffffff;
		hashValue *= 31;
		hashValue += m_buffer[ i ];
	}
	m_callbackProperties.Set( "hashValue", hashValue );
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
LoadFileJob::LoadFileJob( const std::string& fileName, const std::string& eventNameToFireOnCallback , TypeOfWork typeOfWork, JobPriority priority )
	: m_fileName( fileName )
	, Job( eventNameToFireOnCallback, typeOfWork, priority )
{

}

//-----------------------------------------------------------------------------------------------
void LoadFileJob::Execute()
{
	FILE* inputFile = nullptr; 

	inputFile = fopen( m_fileName.c_str(), "rb" );

	size_t bufferSize = GetFileSize( inputFile );

	uchar* buffer = new uchar[ bufferSize ];

	fread( buffer, bufferSize * sizeof(uchar), 1, inputFile );

	m_callbackProperties.Set( "buffer", buffer );
	m_callbackProperties.Set( "bufferSize", bufferSize );

	fclose( inputFile );

	delete buffer;
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
SaveFileJob::SaveFileJob( const std::string& fileName, const uchar* buffer, int bufferSize, const std::string& eventNameToFireOnCallback
	, TypeOfWork typeOfWork, JobPriority priority )
	: m_fileName( fileName )
	, m_buffer( buffer )
	, m_bufferSize( bufferSize )
	, Job( eventNameToFireOnCallback, typeOfWork, priority )
{

}

SaveFileJob::~SaveFileJob()
{
	m_buffer = nullptr;
	delete m_buffer;
}

//-----------------------------------------------------------------------------------------------
void SaveFileJob::Execute()
{
	FILE* outputFile = nullptr; 

	outputFile = fopen( m_fileName.c_str(), "wb+" );
	fwrite( m_buffer, m_bufferSize, 1, outputFile );

	fclose( outputFile );
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
ReverseBufferJob::ReverseBufferJob( uchar* buffer, int bufferSize, const std::string& eventNameToFireOnCallback
	, TypeOfWork typeOfWork, JobPriority priority )
	: m_buffer( buffer )
	, m_bufferSize( bufferSize )
	, Job( eventNameToFireOnCallback, typeOfWork, priority )
{

}

//-----------------------------------------------------------------------------------------------
ReverseBufferJob::~ReverseBufferJob()
{
	m_buffer = nullptr;
	delete m_buffer;
}

//-----------------------------------------------------------------------------------------------
void ReverseBufferJob::Execute()
{
	//const char* bufferEnd = strchr( m_buffer, 0 ); 
	std::reverse( m_buffer, m_buffer + m_bufferSize );
}
