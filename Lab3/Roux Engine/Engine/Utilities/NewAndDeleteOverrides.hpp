#ifndef NEW_AND_DELETE_REDEFINES
#define NEW_AND_DELETE_REDEFINES

#include "Engine/Engine_GameSpecificIncludes.hpp"

#ifndef IGNORE_OVERRIDES

void* operator new( size_t objectSizeInBytes );
void* operator new( size_t objectSizeInBytes, const char* file, unsigned int lineNumber );
void* operator new[]( size_t objectSizeInBytes );
void* operator new[]( size_t objectSizeInBytes, const char* file, unsigned int lineNumber );

void operator delete( void* objectToDelete );
void operator delete( void* objectToDelete, const char* file, unsigned int lineNumber );
void operator delete[]( void* objectToDelete );
void operator delete[]( void* objectToDelete, const char* file, unsigned int lineNumber );

#define new new(__FILE__,__LINE__)

#endif

#endif