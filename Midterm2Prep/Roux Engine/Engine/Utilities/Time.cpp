//-----------------------------------------------------------------------------------------------
#include "Time.hpp"
#include <windows.h>

double Time::s_frequencyInverse = 0.0; 

//-----------------------------------------------------------------------------------------------
void Time::InitializeTime()
{
	if( s_frequencyInverse == 0.0 )
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		s_frequencyInverse = 1.0 / static_cast< double >(frequency.QuadPart);
	}
}

//-----------------------------------------------------------------------------------------------
double Time::GetCurrentTimeInSeconds()
{
	LARGE_INTEGER counts;
	QueryPerformanceCounter(&counts);
	
	double currentSeconds = static_cast< double >(counts.QuadPart) * s_frequencyInverse;
	return currentSeconds;
}