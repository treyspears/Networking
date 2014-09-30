#include "MemoryManager.hpp"
#include <string>

#include <assert.h>
#include <sstream>
#include <Windows.h>
#include "Engine/Utilities/ErrorWarningAssert.hpp"

const size_t POOL_SIZE = 1024 * 1024 * 876;

//-----------------------------------------------------------------------------------------------
MemoryManager::MemoryManager()
	: m_totalNumberOfAllocations( 0 )
	, m_currentTotalBytesAllocated( 0 )
	, m_totalBytesAllocatedOverLifetime( 0 )
	, m_largestAllocationSize( 0 )
{
	m_memoryPool = static_cast< byte_t* >( malloc( POOL_SIZE ) );
	m_startingAddressForAlloc = m_memoryPool;

	memset( m_memoryPool, 0, POOL_SIZE );

	MetaData* block = reinterpret_cast< MetaData* >( m_memoryPool );

	block->m_isOccupied = false;
	block->m_lineNumber = 0;
	block->m_offsetToMetaDataAboveMe = 0;
	block->m_blockDataSegmentSize = POOL_SIZE - sizeof( MetaData );
	block->m_fileName = nullptr;	
}

//-----------------------------------------------------------------------------------------------
MemoryManager::~MemoryManager()
{

}

//-----------------------------------------------------------------------------------------------
void* MemoryManager::AllocateMemory( size_t numBytes, const char* file, size_t lineNumber )
{
	const	byte_t* MAX_ADDRESS = m_memoryPool + POOL_SIZE;
	
	size_t bytesNeeded = numBytes + sizeof( MetaData );
	
	if( m_startingAddressForAlloc >= MAX_ADDRESS )
	{
		m_startingAddressForAlloc = m_memoryPool;
	}

	MetaData* block = reinterpret_cast< MetaData* >( m_startingAddressForAlloc );
	byte_t* addressOfBlock = m_startingAddressForAlloc;

	bool loopedAround = false;

	while( !loopedAround || addressOfBlock < m_startingAddressForAlloc )
	{
		if( !block->m_isOccupied )
		{
			if( block->m_blockDataSegmentSize >= numBytes && block->m_blockDataSegmentSize < bytesNeeded )
			{
				addressOfBlock += numBytes;
				//block->m_blockDataSegmentSize = numBytes;
				m_startingAddressForAlloc = addressOfBlock;

				break;
			}
			else if( block->m_blockDataSegmentSize >= bytesNeeded )
			{
				addressOfBlock += bytesNeeded;
				MetaData* newMetaData = reinterpret_cast< MetaData* > ( addressOfBlock );

				newMetaData->m_isOccupied = false;
				newMetaData->m_lineNumber = 0;
				newMetaData->m_offsetToMetaDataAboveMe = bytesNeeded;
				newMetaData->m_blockDataSegmentSize = block->m_blockDataSegmentSize - bytesNeeded;
				newMetaData->m_fileName = nullptr;	

				block->m_blockDataSegmentSize = numBytes;
				m_startingAddressForAlloc = addressOfBlock;

				break;
			}
		}

		addressOfBlock += sizeof( MetaData ) + block->m_blockDataSegmentSize;

		if( addressOfBlock > MAX_ADDRESS )
		{
			loopedAround = true;
			addressOfBlock = m_memoryPool;
		}

		if( loopedAround && addressOfBlock >= m_startingAddressForAlloc )
		{
			assert( false );
			//FATAL_ASSERTION( false, "crap" );
		}

		block = reinterpret_cast< MetaData* >( addressOfBlock );
	}
	//while( block->m_isOccupied || block->m_blockDataSegmentSize < bytesNeeded )
	//{
	//	block = reinterpret_cast< MetaData* > ( reinterpret_cast< byte_t* >( block ) + sizeof( MetaData ) + block->m_blockDataSegmentSize );
	//}

	//MetaData* newMetaData = reinterpret_cast< MetaData* > ( reinterpret_cast< byte_t* >( block ) + bytesNeeded );

	//newMetaData->m_isOccupied = false;
	//newMetaData->m_lineNumber = 0;
	//newMetaData->m_blockDataSegmentSize = block->m_blockDataSegmentSize - bytesNeeded;
	//newMetaData->m_fileName = nullptr;	

	//block->m_blockDataSegmentSize = numBytes;

	//assert( goodAllocation == true );
	

	block->m_isOccupied = true;
	block->m_fileName = file;
	block->m_lineNumber = lineNumber;

	if( numBytes > m_largestAllocationSize )
		m_largestAllocationSize = numBytes;

	++m_totalNumberOfAllocations;
	m_currentTotalBytesAllocated += numBytes;
	m_totalBytesAllocatedOverLifetime += numBytes;

	return static_cast< void* >( reinterpret_cast< byte_t* >( block ) + sizeof( MetaData ) );
}

//-----------------------------------------------------------------------------------------------
//void* MemoryManager::AllocateMemory( size_t numBytes, const char* file, size_t lineNumber )
//{
//	const	byte_t* MAX_ADDRESS = m_memoryPool + POOL_SIZE;
//
//	size_t bytesNeeded = numBytes + sizeof( MetaData );
//	byte_t* addressOfBlock = m_memoryPool;
//	MetaData* block = reinterpret_cast< MetaData* >( addressOfBlock );
//
//	bool isAtEndOfPool = true;
//
//	while ( addressOfBlock < MAX_ADDRESS )
//	{
//		if( !block->m_isOccupied )
//		{
//			if( block->m_blockDataSegmentSize >= numBytes && block->m_blockDataSegmentSize < bytesNeeded )
//			{
//				addressOfBlock += numBytes;
//				block->m_isOccupied = true;
//				block->m_lineNumber = lineNumber;
//				block->m_fileName = file;
//
//				isAtEndOfPool = false;
//				break;
//			}
//			else if( block->m_blockDataSegmentSize >= bytesNeeded )
//			{
//				MetaData* newMetaData = reinterpret_cast< MetaData* >( addressOfBlock + bytesNeeded );
//
//				newMetaData->m_isOccupied = false;
//				newMetaData->m_lineNumber = 0;
//				newMetaData->m_offsetToMetaDataAboveMe = bytesNeeded;
//				newMetaData->m_blockDataSegmentSize = block->m_blockDataSegmentSize - bytesNeeded;
//				newMetaData->m_fileName = nullptr;	
//
//				block->m_isOccupied = true;
//				block->m_lineNumber = lineNumber;
//				block->m_blockDataSegmentSize = numBytes;	
//				block->m_fileName = file;
//
//				isAtEndOfPool = false;
//				break;	
//			}
//		}
//
//		addressOfBlock += sizeof( MetaData ) + block->m_blockDataSegmentSize;
//		block = reinterpret_cast< MetaData* >( addressOfBlock );
//	}
//	
//	assert( isAtEndOfPool == false );
//
//	return static_cast< void* >( reinterpret_cast< byte_t* >( block ) + sizeof( MetaData ) );
//}

//-----------------------------------------------------------------------------------------------
void MemoryManager::FreeMemory( void* addressOfMemoryToFree )
{
	const byte_t* MAX_ADDRESS = m_memoryPool + POOL_SIZE;

	byte_t* addressAsByte_t = static_cast< byte_t* >( addressOfMemoryToFree ) - sizeof( MetaData );
	MetaData* blockToFree = reinterpret_cast< MetaData* >( addressAsByte_t );

	m_currentTotalBytesAllocated -= blockToFree->m_blockDataSegmentSize;

	if( addressAsByte_t == m_memoryPool )
	{
		FreeTop( addressAsByte_t, blockToFree );
	}
	else if( ( addressAsByte_t + blockToFree->m_blockDataSegmentSize + sizeof( MetaData ) ) >= MAX_ADDRESS )
	{
		FreeBottom( addressAsByte_t, blockToFree );
	}
	else
	{
		FreeGeneral( addressAsByte_t, blockToFree );
	}
}

//-----------------------------------------------------------------------------------------------
void MemoryManager::FreeTop( byte_t* addressOfTop, MetaData* topBlock )
{
	const byte_t* MAX_ADDRESS = m_memoryPool + POOL_SIZE;

	byte_t* addressOfNextBlock = addressOfTop + topBlock->m_blockDataSegmentSize + sizeof( MetaData );

	if( addressOfNextBlock < MAX_ADDRESS )
	{
		MetaData* nextBlock = reinterpret_cast< MetaData* >( addressOfNextBlock );

		if( !nextBlock->m_isOccupied )
		{
			byte_t* addressOfBlockAfterNext = addressOfNextBlock + nextBlock->m_blockDataSegmentSize + sizeof( MetaData );
			
			if( addressOfBlockAfterNext < MAX_ADDRESS )
			{
				MetaData* blockAfterNext = reinterpret_cast< MetaData* >( addressOfBlockAfterNext );
				blockAfterNext->m_offsetToMetaDataAboveMe += topBlock->m_blockDataSegmentSize + sizeof( MetaData );
			}

			topBlock->m_blockDataSegmentSize += nextBlock->m_blockDataSegmentSize + sizeof( MetaData );
		
			if( m_startingAddressForAlloc == addressOfNextBlock )
				m_startingAddressForAlloc = addressOfTop + topBlock->m_blockDataSegmentSize + sizeof( MetaData );
		}
	}
	topBlock->m_isOccupied = false;
}

//-----------------------------------------------------------------------------------------------
void MemoryManager::FreeBottom( byte_t* addressOfBottom, MetaData* bottomBlock )
{
	byte_t* addressOfPreviousBlock = addressOfBottom - bottomBlock->m_offsetToMetaDataAboveMe;

	MetaData* previousBlock = reinterpret_cast< MetaData* >( addressOfPreviousBlock );

	if( !previousBlock->m_isOccupied )
	{
		previousBlock->m_blockDataSegmentSize += bottomBlock->m_blockDataSegmentSize + sizeof( MetaData );
		if( m_startingAddressForAlloc == addressOfBottom )
		{
			m_startingAddressForAlloc = addressOfPreviousBlock + previousBlock->m_blockDataSegmentSize + sizeof( MetaData );
		}
	}
	else
	{
		bottomBlock->m_isOccupied = false;
	}
}

//-----------------------------------------------------------------------------------------------
void MemoryManager::FreeGeneral( byte_t* addressOfBlock, MetaData* block )
{
	const byte_t* MAX_ADDRESS = m_memoryPool + POOL_SIZE;

	byte_t* addressOfPreviousBlock = addressOfBlock - block->m_offsetToMetaDataAboveMe;
	MetaData* previousBlock = reinterpret_cast< MetaData* >( addressOfPreviousBlock );

	byte_t* addressOfNextBlock = addressOfBlock + block->m_blockDataSegmentSize + sizeof( MetaData );
	MetaData* nextBlock = reinterpret_cast< MetaData* >( addressOfNextBlock );

	bool isPreviousBlockOccupied = previousBlock->m_isOccupied;
	bool isNextBlockOccupied = nextBlock->m_isOccupied;

	if( !isPreviousBlockOccupied && !isNextBlockOccupied )
	{
		byte_t* addressOfBlockAfterNext = addressOfNextBlock + nextBlock->m_blockDataSegmentSize + sizeof( MetaData );

		if( addressOfBlockAfterNext < MAX_ADDRESS )
		{
			MetaData* blockAfterNext = reinterpret_cast< MetaData* >( addressOfBlockAfterNext );
			blockAfterNext->m_offsetToMetaDataAboveMe += nextBlock->m_offsetToMetaDataAboveMe + block->m_offsetToMetaDataAboveMe;
		}
		previousBlock->m_blockDataSegmentSize += nextBlock->m_offsetToMetaDataAboveMe + nextBlock->m_blockDataSegmentSize + sizeof( MetaData );

		if( m_startingAddressForAlloc == addressOfBlock || m_startingAddressForAlloc == addressOfNextBlock )
			m_startingAddressForAlloc = addressOfBlockAfterNext;
	}
	else if( !isPreviousBlockOccupied )
	{
		previousBlock->m_blockDataSegmentSize += nextBlock->m_offsetToMetaDataAboveMe;
		nextBlock->m_offsetToMetaDataAboveMe += block->m_offsetToMetaDataAboveMe;

		if( m_startingAddressForAlloc == addressOfBlock )
		{
			m_startingAddressForAlloc = addressOfNextBlock;
		}
	}
	else if( !isNextBlockOccupied )
	{
		block->m_blockDataSegmentSize += nextBlock->m_blockDataSegmentSize + sizeof( MetaData );
		block->m_isOccupied = false;
		if( m_startingAddressForAlloc == addressOfNextBlock )
		{
			m_startingAddressForAlloc = addressOfBlock + block->m_blockDataSegmentSize + sizeof( MetaData );
		}
	}
	else
	{
		block->m_isOccupied = false;
	}
}

//-----------------------------------------------------------------------------------------------
void MemoryManager::ReportMemoryLeaks()
{
	const byte_t* MAX_ADDRESS = m_memoryPool + POOL_SIZE;
	bool leakDetected = false;

	std::ostringstream outputStream;
	std::string output;

	byte_t* currentAddress = m_memoryPool;
	MetaData* currentBlock;

	OutputDebugStringA( "***Memory Manager report:" );

	while( currentAddress < MAX_ADDRESS )
	{
		currentBlock = reinterpret_cast< MetaData* >( currentAddress );

		if( currentBlock->m_isOccupied )
		{
			leakDetected = true;

			outputStream.str( "" );
			outputStream << "\nMemory leak of ";
			outputStream << currentBlock->m_blockDataSegmentSize;
			outputStream << " bytes, allocated on line ";

			if( currentBlock->m_fileName == nullptr )
			{
				outputStream << "unknown of ";
				outputStream << "file unknown.";
			}
			else
			{
				outputStream << currentBlock->m_lineNumber;
				outputStream << " of ";
				outputStream << currentBlock->m_fileName;
			}
			
			output = outputStream.str();
			OutputDebugStringA( output.c_str() );
		}
		currentAddress += currentBlock->m_blockDataSegmentSize + sizeof( MetaData );
	}

	if( !leakDetected )
		OutputDebugStringA( "\nNo leaks detected." );
}