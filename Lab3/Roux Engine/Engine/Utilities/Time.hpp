#ifndef TIME
#define TIME

//-----------------------------------------------------------------------------------------------
class Time
{

public:

	static double GetCurrentTimeInSeconds();
	static void InitializeTime();

private:

	static double s_frequencyInverse;
};

#endif