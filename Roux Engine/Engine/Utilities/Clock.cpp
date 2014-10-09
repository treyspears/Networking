#include "Clock.hpp"

#include "Engine/Utilities/EventSystem.hpp"

NamedProperties g_emptyProperties = NamedProperties::EmptyProperties();

//-----------------------------------------------------------------------------------------------
Clock::Clock() 
	 : m_parentClock( nullptr )
	 , m_maximumDeltaSeconds( DEFAULT_MAX_DELTA_SECONDS )
	 , m_isPaused( false )
	 , m_timeScale( 1.f )
	 , m_currentElapsedSeconds( 0.f )
	 , m_currentDeltaSeconds( 0.f )
{

};

//-----------------------------------------------------------------------------------------------
Clock::Clock( const Clock& parent, double maxDelta, bool startPaused, float scale )
	: m_maximumDeltaSeconds( maxDelta )
	, m_isPaused( startPaused ) 
	, m_timeScale( scale )
	, m_currentElapsedSeconds( 0.f )
	, m_currentDeltaSeconds( 0.f )
{
	m_parentClock = const_cast< Clock* >( &parent );
	m_parentClock->AddChildClockToHierarchy( this );
}

//-----------------------------------------------------------------------------------------------
Clock::~Clock()
{
	while( !m_childClocks.empty() )
	{
		Clock* clockToDelete = m_childClocks.back();
		m_childClocks.pop_back();

		delete clockToDelete;
	}

	if( m_parentClock )
	{
		m_parentClock->RemoveChildClockFromHierarchy( this );
		m_parentClock = nullptr;
	}

	delete m_parentClock;
}

//-----------------------------------------------------------------------------------------------
void Clock::AddChildClockToHierarchy( Clock* childClock )
{
	m_childClocks.push_back( childClock );
}

//-----------------------------------------------------------------------------------------------
void Clock::RemoveChildClockFromHierarchy( Clock* childClock )
{
	for( int i = 0; i < static_cast< int >( m_childClocks.size() ); ++i )
	{
		if( m_childClocks[ i ] == childClock )
		{
			m_childClocks[ i ] = m_childClocks.back();
			m_childClocks.pop_back();
			break;
		}
	}
}

//-----------------------------------------------------------------------------------------------
void Clock::AdvanceTime( double deltaSeconds )
{
	if( m_isPaused )
	{
		deltaSeconds = 0.0;
	}
	else if( deltaSeconds > m_maximumDeltaSeconds )
	{
		deltaSeconds = m_maximumDeltaSeconds;
	}

	m_currentDeltaSeconds = deltaSeconds * m_timeScale;
	m_currentElapsedSeconds += m_currentDeltaSeconds;

	AdvanceAlarms( m_currentDeltaSeconds );

	for( std::vector< Clock* >::iterator iter = m_childClocks.begin(); iter != m_childClocks.end(); ++iter )
	{
		( *iter )->AdvanceTime( m_currentDeltaSeconds );
	}
}

//-----------------------------------------------------------------------------------------------
void Clock::AdvanceAlarms( double deltaSeconds )
{
	std::string currentAlarmName;
	Alarm currentAlarm;

	auto iter = m_alarms.begin();

	while (iter != m_alarms.end())
	{
		iter->second.elapsedSeconds += static_cast< float >( deltaSeconds );

		currentAlarmName = iter->first;
		currentAlarm = iter->second;

		if( currentAlarm.elapsedSeconds >= currentAlarm.alarmLengthInSeconds )
		{
			EventSystem& eventSystem = EventSystem::GetInstance();

			iter = m_alarms.erase( iter );	
			eventSystem.FireEvent( currentAlarmName, currentAlarm.parametersToSendWhenTriggered );	
		}
		else
		{
			++iter;
		}
	}
}

//-----------------------------------------------------------------------------------------------
NamedProperties& Clock::GetPropertiesFromAlarmWithName( const std::string& alarmName )
{
	auto foundIter = m_alarms.find( alarmName );

	if( foundIter != m_alarms.end() )
	{
		return foundIter->second.parametersToSendWhenTriggered;
	}

	return g_emptyProperties;
}