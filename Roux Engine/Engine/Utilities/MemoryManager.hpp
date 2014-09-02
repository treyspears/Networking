#ifndef MEMORY
#define MEMORY

#include <stdlib.h>

typedef unsigned int size_t;
typedef unsigned char byte_t;

//void* operator new( size_t objectSizeInBytes )
//{
//	MemoryManager memoryManager& = MemoryManager::GetInstance();
//
//
//}

struct MetaData
{
	bool		m_isOccupied;
	size_t		m_lineNumber;
	size_t		m_offsetToMetaDataAboveMe;
	size_t		m_blockDataSegmentSize;
	const char*	m_fileName;
};

class MemoryManager
{
public:

	static MemoryManager& GetInstance()
	{
		static MemoryManager instance; 

		return instance;
	}

	void*	AllocateMemory( size_t numBytes, const char* file = nullptr, size_t lineNumber = 0 );
	void	FreeMemory( void* addressOfMemoryToFree );
	void	ReportMemoryLeaks();

	inline const size_t GetTotalNumberOfAllocationsRequested() const;
	inline const size_t GetTotalBytesAllocatedCurrently() const;
	inline const size_t GetTotalBytesAllocatedSinceBirth() const;
	inline const size_t GetSizeOfLargetsAllocationRequested() const;
	inline const float	GetAverageSizeOfAllRequests() const;

private:

	byte_t* m_memoryPool;
	byte_t* m_startingAddressForAlloc;

	size_t m_totalNumberOfAllocations;
	size_t m_currentTotalBytesAllocated;
	size_t m_totalBytesAllocatedOverLifetime;
	size_t m_largestAllocationSize;

	void FreeTop( byte_t* addressOfTop, MetaData* topBlock );
	void FreeBottom( byte_t* addressOfBottom, MetaData* bottomBlock );
	void FreeGeneral( byte_t* addressOfBlock, MetaData* block );

	MemoryManager();
	~MemoryManager();

	MemoryManager( MemoryManager const& );              
	void operator=( MemoryManager const& ); 
};

//-----------------------------------------------------------------------------------------------
const size_t MemoryManager::GetTotalNumberOfAllocationsRequested() const
{
	return m_totalNumberOfAllocations;
}

//-----------------------------------------------------------------------------------------------
const size_t MemoryManager::GetTotalBytesAllocatedCurrently() const
{
	return m_currentTotalBytesAllocated;
}

//-----------------------------------------------------------------------------------------------
const size_t MemoryManager::GetTotalBytesAllocatedSinceBirth() const
{
	return m_totalBytesAllocatedOverLifetime;
}

//-----------------------------------------------------------------------------------------------
const size_t MemoryManager::GetSizeOfLargetsAllocationRequested() const
{
	return m_largestAllocationSize;
}

//-----------------------------------------------------------------------------------------------
const float MemoryManager::GetAverageSizeOfAllRequests() const
{
	float average = static_cast< float >( m_totalBytesAllocatedOverLifetime ) / m_totalNumberOfAllocations;

	return average;
}

#endif