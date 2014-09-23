#define STATIC

#include "ProfileSection.hpp"

#include <sstream>

#include "Engine/Rendering/ConsoleLog.hpp"
#include "Engine/Primitives/Color.hpp"
#include "Engine/Primitives/Vector4.hpp"

#include "Time.hpp"

#include "Engine/Utilities/NewAndDeleteOverrides.hpp"

std::vector< ProfileSection* > ProfileSection::s_profileStack;
std::map< std::string, ProfileSection::CategoryStatistics > ProfileSection::s_categoryAndStatistics;

const Vector4f SlowCategoryColor = Color( Red ).ToVector4fNormalized(); 
const Vector4f FastCategoryColor = Color( Green ).ToVector4fNormalized();

const float SLOW_CATEGORY_TIME_INVERSE = 1.f / 0.025f;

void Command_Print_ProfilerStatistics( const ConsoleCommandArgs& args )
{
	if(ConsoleLog::s_currentLog != nullptr)
	{
		std::string categoryName = args.m_argsList.back();

		if( categoryName == "" )
			ProfileSection::ReportProfileStatsForAllCategories();
		else
			ProfileSection::ReportProfileStatsForCategory( categoryName );
	}
}

//-----------------------------------------------------------------------------------------------
ProfileSection::ProfileSection( const std::string& categoryName )
	: m_categoryName( categoryName )
	, m_startTime( Time::GetCurrentTimeInSeconds() )
{

}

//-----------------------------------------------------------------------------------------------
ProfileSection::~ProfileSection()
{
	const double elapsedTime = Time::GetCurrentTimeInSeconds() - m_startTime;

	AddOrUpdateCategoryEntry( m_categoryName, elapsedTime );
}

//-----------------------------------------------------------------------------------------------
STATIC void ProfileSection::InitializeProfiler()
{
	RegisterEvent( "reportStats", Command_Print_ProfilerStatistics );
}

//-----------------------------------------------------------------------------------------------
STATIC void ProfileSection::Start( const std::string& categoryName )
{
	s_profileStack.push_back( new ProfileSection( categoryName ) );
}

//-----------------------------------------------------------------------------------------------
STATIC void ProfileSection::Stop()
{
	ProfileSection* profilerToDelete = s_profileStack.back();
	s_profileStack.pop_back();
	delete profilerToDelete;
}

//-----------------------------------------------------------------------------------------------
STATIC void ProfileSection::ReportProfileStatsForCategory( const std::string& categoryName )
{
	auto foundIter = s_categoryAndStatistics.find( categoryName );

	if( foundIter != s_categoryAndStatistics.end() )
	{
		double currentElapsedSeconds = foundIter->second.GetMostRecentElapsedSeconds();

		std::ostringstream outputStream;
		outputStream << categoryName << "-> TotalElapsedTimeForFrame: ";
		outputStream << currentElapsedSeconds;
		outputStream << ", AverageElapsedTimeAcrossFrames: ";
		outputStream << foundIter->second.GetAverageElapsedSeconds();

		std::string stringToOutput = outputStream.str();

		double timeNormalized = currentElapsedSeconds * SLOW_CATEGORY_TIME_INVERSE;

		Vector4f outputColor( 1.f, 1.f, 0.f, 1.f );
		outputColor.x *= 2.f * static_cast< float >( timeNormalized );
		outputColor.y *= 2.f * ( 1.f - static_cast< float >( timeNormalized ) );

		ConsoleLog::s_currentLog->ConsolePrint( stringToOutput, outputColor, true );
	}
	else
	{
		ConsoleLog::s_currentLog->ConsolePrint( "Error: No profile section with that category found." );
	}
}

//-----------------------------------------------------------------------------------------------
STATIC void ProfileSection::ReportProfileStatsForAllCategories()
{
	for( auto iter = s_categoryAndStatistics.begin(); iter != s_categoryAndStatistics.end(); ++ iter )
	{
		double currentElapsedSeconds = iter->second.GetMostRecentElapsedSeconds();

		std::ostringstream outputStream;
		outputStream << iter->first << "-> TotalElapsedTimeForFrame: ";
		outputStream << currentElapsedSeconds;
		outputStream << ", AverageElapsedTimeAcrossFrames: ";
		outputStream << iter->second.GetAverageElapsedSeconds();

		std::string stringToOutput = outputStream.str();

		double timeNormalized = currentElapsedSeconds * SLOW_CATEGORY_TIME_INVERSE;

		Vector4f outputColor( 1.f, 1.f, 0.f, 1.f );
		outputColor.x *= 2.f * static_cast< float >( timeNormalized );
		outputColor.y *= 2.f * ( 1.f - static_cast< float >( timeNormalized ) );

		ConsoleLog::s_currentLog->ConsolePrint( stringToOutput, outputColor, true );
	}
}

//-----------------------------------------------------------------------------------------------
STATIC void ProfileSection::AddOrUpdateCategoryEntry( const std::string& categoryName, double elapsedSeconds )
{
	auto foundIter = s_categoryAndStatistics.find( categoryName );
	if( foundIter != s_categoryAndStatistics.end() )
	{
		CategoryStatistics& entry = foundIter->second;

		entry.sumElapsedTime += elapsedSeconds;
		entry.mostRecentElapsedSeconds = elapsedSeconds;
		++entry.numTimesSet;
	}
	else
	{
		s_categoryAndStatistics[ categoryName ] = CategoryStatistics( elapsedSeconds );
	}
}

//-----------------------------------------------------------------------------------------------
double ProfileSection::CategoryStatistics::GetMostRecentElapsedSeconds() const
{
	return mostRecentElapsedSeconds;
}

//-----------------------------------------------------------------------------------------------
double ProfileSection::CategoryStatistics::GetAverageElapsedSeconds() const
{
	return ( sumElapsedTime / numTimesSet );
}