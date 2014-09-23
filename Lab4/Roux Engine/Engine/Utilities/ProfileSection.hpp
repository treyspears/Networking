#ifndef PROFILE_SECTION
#define PROFILE_SECTION

#include <vector>
#include <map>
#include <string>

#include "Engine/Utilities/CommandRegistry.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"

class ProfileSection
{
public:

	ProfileSection( const std::string& categoryName );
	~ProfileSection();

	static void InitializeProfiler();
	static void Start( const std::string& categoryName );
	static void Stop ();
	static void ReportProfileStatsForCategory( const std::string& categoryName );
	static void ReportProfileStatsForAllCategories();

private:

	void operator=( ProfileSection const& );

	static void AddOrUpdateCategoryEntry( const std::string& categoryName, double elapsedSeconds );

	const std::string m_categoryName;
	const double m_startTime;

	struct CategoryStatistics
	{
		CategoryStatistics() 
			: mostRecentElapsedSeconds( 0.0 )
			, sumElapsedTime( 0.0 )
			, numTimesSet( 0 )
		{}

		CategoryStatistics( double elapsedTime )
			: mostRecentElapsedSeconds( elapsedTime )
			, sumElapsedTime( elapsedTime )
			, numTimesSet( 1 )
		{

		}

		double GetMostRecentElapsedSeconds() const;
		double GetAverageElapsedSeconds() const;

		double mostRecentElapsedSeconds;
		double sumElapsedTime;

		uint numTimesSet;
	};

	static std::map< std::string, CategoryStatistics > s_categoryAndStatistics;
	static std::vector< ProfileSection* > s_profileStack;
};

void Command_Print_ProfilerStatistics( const ConsoleCommandArgs& args );

#endif