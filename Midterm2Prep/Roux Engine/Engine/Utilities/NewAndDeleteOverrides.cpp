#include "NewAndDeleteOverrides.hpp"

#include "MemoryManager.hpp"

#ifndef IGNORE_OVERRIDES

#define UNUSED(x) (void)(x);

#undef new

void* operator new( size_t objectSizeInBytes )
{
	MemoryManager& manager = MemoryManager::GetInstance();
	return manager.AllocateMemory( objectSizeInBytes );
}

void* operator new[]( size_t objectSizeInBytes )
{
	MemoryManager& manager = MemoryManager::GetInstance();
	return manager.AllocateMemory( objectSizeInBytes );
}

//-----------------------------------------------------------------------------------------------
void* operator new( size_t objectSizeInBytes, const char* file, unsigned int lineNumber )
{
	MemoryManager& manager = MemoryManager::GetInstance();
	return manager.AllocateMemory( objectSizeInBytes, file, lineNumber );
}

//-----------------------------------------------------------------------------------------------
void* operator new[]( size_t objectSizeInBytes, const char* file, unsigned int lineNumber )
{
	MemoryManager& manager = MemoryManager::GetInstance();
	return manager.AllocateMemory( objectSizeInBytes, file, lineNumber );
}

//-----------------------------------------------------------------------------------------------
void operator delete( void* objectToDelete )
{
	MemoryManager& manager = MemoryManager::GetInstance();
	manager.FreeMemory( objectToDelete );
}

//-----------------------------------------------------------------------------------------------
void operator delete( void* objectToDelete, const char* file, unsigned int lineNumber )
{
	UNUSED( file );
	UNUSED( lineNumber );

	MemoryManager& manager = MemoryManager::GetInstance();
	manager.FreeMemory( objectToDelete );
}

//-----------------------------------------------------------------------------------------------
void operator delete[]( void* objectToDelete )
{
	MemoryManager& manager = MemoryManager::GetInstance();
	manager.FreeMemory( objectToDelete );
}

//-----------------------------------------------------------------------------------------------
void operator delete[]( void* objectToDelete, const char* file, unsigned int lineNumber )
{
	UNUSED( file );
	UNUSED( lineNumber );

	MemoryManager& manager = MemoryManager::GetInstance();
	manager.FreeMemory( objectToDelete );
}

#define new new(__FILE__,__LINE__)

#endif