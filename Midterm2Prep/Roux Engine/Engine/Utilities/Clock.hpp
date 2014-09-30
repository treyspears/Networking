#ifndef CLOCK
#define CLOCK

#include <vector>
#include <map>
#include <string>

#include "Engine/Utilities/NamedProperties.hpp"

const double DEFAULT_MAX_DELTA_SECONDS = 99999.0;

class Clock
{
public:
	
	Clock( const Clock& parent
		, double maxDelta = DEFAULT_MAX_DELTA_SECONDS
		, bool startPaused = false
		, float scale = 1.f );

	~Clock();

	void			AdvanceTime( double deltaSeconds );

	inline bool		IsClockPaused() const;
	inline void		Pause();
	inline void		UnPause();

	inline void		AddAlarmWithLengthAndParameters( const std::string& alarmName, float lengthInSeconds, NamedProperties& parameters );
	inline float	GetPercentageElapsedForAlarm( const std::string& alarmName ) const;
	inline float	GetSecondsElapsedForAlarm( const std::string& alarmName ) const;
	NamedProperties& GetPropertiesFromAlarmWithName( const std::string& alarmName );

	static Clock&	GetMasterClock()
	{
		static Clock masterClockInstance;

		return masterClockInstance;
	}

	float m_timeScale;
	
	double m_maximumDeltaSeconds;
	double m_currentDeltaSeconds;
	double m_currentElapsedSeconds;

private:
	
	Clock();
	void operator=( Clock const& );

	void AddChildClockToHierarchy( Clock* childClock );
	void RemoveChildClockFromHierarchy( Clock* childClock );
	void AdvanceAlarms( double deltaSeconds );

	bool m_isPaused;
	Clock* m_parentClock;
	std::vector< Clock* > m_childClocks;

	struct Alarm
	{
		Alarm() 
			: alarmLengthInSeconds( 0.f )
			, elapsedSeconds( 0.f )
			, parametersToSendWhenTriggered( NamedProperties::EmptyProperties() )
		{}

		Alarm( float length, NamedProperties& eventParameters )
			: alarmLengthInSeconds( length )
			, elapsedSeconds( 0.f )
			, parametersToSendWhenTriggered( eventParameters )
		{}

		float alarmLengthInSeconds;
		float elapsedSeconds;

		NamedProperties parametersToSendWhenTriggered;
	};

	std::map< std::string, Alarm > m_alarms;
};

//-----------------------------------------------------------------------------------------------
bool Clock::IsClockPaused() const
{
	return m_isPaused;
}

//-----------------------------------------------------------------------------------------------
void Clock::Pause()
{
	m_isPaused = true;
}

//-----------------------------------------------------------------------------------------------
void Clock::UnPause()
{
	m_isPaused = false;
}

//-----------------------------------------------------------------------------------------------
void Clock::AddAlarmWithLengthAndParameters( const std::string& alarmName, float lengthInSeconds, NamedProperties& parameters )
{
	Clock::Alarm newAlarm( lengthInSeconds, parameters );
	
	m_alarms[ alarmName ] = newAlarm;
}

//-----------------------------------------------------------------------------------------------
float Clock::GetPercentageElapsedForAlarm( const std::string& alarmName ) const
{
	float result = 0.f;

	auto foundIter = m_alarms.find( alarmName );

	if( foundIter != m_alarms.end() )
	{
		const Alarm& foundAlarm = foundIter->second;

		if( foundAlarm.alarmLengthInSeconds > 0.f )
			result = foundAlarm.elapsedSeconds / foundAlarm.alarmLengthInSeconds;
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
float Clock::GetSecondsElapsedForAlarm( const std::string& alarmName ) const
{
	float result = -1.f;

	auto foundIter = m_alarms.find( alarmName );

	if( foundIter != m_alarms.end() )
	{
		result = foundIter->second.elapsedSeconds;
	}

	return result;
}



#endif
