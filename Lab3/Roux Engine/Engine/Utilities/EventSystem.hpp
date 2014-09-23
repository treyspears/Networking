#ifndef EVENT_SYSTEM
#define EVENT_SYSTEM

#include <map>
#include <vector>
#include <string>

#include "Engine/Utilities/NamedProperties.hpp"

//-----------------------------------------------------------------------------------------------
class EventSubscription
{

public:
	virtual ~EventSubscription() {}
	virtual void FireRegisteredEvent( NamedProperties& parameters ) = 0;
	virtual void* GetObjectAddress() = 0;
};

//-----------------------------------------------------------------------------------------------
template< typename T_SubscriberType >
class TypedEventSubscription : public EventSubscription
{

public:
	typedef void( T_SubscriberType::*SubscriberFunctionPtr )( NamedProperties& parameters );

	TypedEventSubscription( SubscriberFunctionPtr callBackFunction, T_SubscriberType* subscriber );

	inline virtual void FireRegisteredEvent( NamedProperties& parameters );
	inline virtual void* GetObjectAddress();

	T_SubscriberType* m_subscriber;

private:
	SubscriberFunctionPtr m_function;
};

//-----------------------------------------------------------------------------------------------
template< typename T_SubscriberType >
TypedEventSubscription<T_SubscriberType>::TypedEventSubscription( SubscriberFunctionPtr callBackFunction, T_SubscriberType* subscriber )
	: m_function( callBackFunction )
	, m_subscriber( subscriber )
{

}

//-----------------------------------------------------------------------------------------------
template< typename T_SubscriberType >
void TypedEventSubscription<T_SubscriberType>::FireRegisteredEvent( NamedProperties& parameters )
{
	( m_subscriber->*m_function )( parameters );
}

//-----------------------------------------------------------------------------------------------
template< typename T_SubscriberType >
void* TypedEventSubscription<T_SubscriberType>::GetObjectAddress()
{
	return ( void* )m_subscriber;
}

//-----------------------------------------------------------------------------------------------
class EventSystem
{
public:

	static EventSystem& GetInstance()
	{
		static EventSystem instance; 

		return instance;
	}

	template< typename T_RegisteringFunctionType, typename T_RegisteringObjectType >
	void RegisterEventWithCallbackAndObject( const std::string& eventName, T_RegisteringFunctionType registeredCallBackFunction, T_RegisteringObjectType* registeredObject );

	template< typename T_RegisteringObjectType >
	void UnregisterAnObjectFromAnEvent( const std::string& eventName, T_RegisteringObjectType* registeredObject );

	template< typename T_RegisteringObjectType >
	void UnregisterAnObjectFromAllEvents( T_RegisteringObjectType* registeredObject );

	void FireEvent( const std::string& eventName, NamedProperties& arguments );

private:

	EventSystem() {}; 

	EventSystem(EventSystem const&);              
	void operator=(EventSystem const&); 

	std::map< std::string, std::vector<EventSubscription*> > m_subscribers;
};

//-----------------------------------------------------------------------------------------------
template< typename T_RegisteringFunctionType, typename T_RegisteringObjectType >
inline void EventSystem::RegisterEventWithCallbackAndObject( const std::string& eventName, T_RegisteringFunctionType registeredCallBackFunction, T_RegisteringObjectType* registeredObject )
{
	TypedEventSubscription< T_RegisteringObjectType >* newTypedEventSubscription = new TypedEventSubscription< T_RegisteringObjectType >( registeredCallBackFunction, registeredObject );
	
	m_subscribers[eventName].push_back( newTypedEventSubscription );
}

//-----------------------------------------------------------------------------------------------
template< typename T_RegisteringObjectType >
inline void EventSystem::UnregisterAnObjectFromAnEvent( const std::string& eventName, T_RegisteringObjectType* registeredObject )
{
	auto foundIter = m_subscribers.find( eventName );

	if( foundIter != m_subscribers.end() )
	{
		for( int i = 0; i < (int)foundIter->second.size(); ++ i )
		{
			if( foundIter->second[i]->GetObjectAddress() == registeredObject )
			{
				EventSubscription* temp = foundIter->second.back();
				if( i != (int)foundIter->second.size() - 1 )
				{
					delete foundIter->second[i];
					foundIter->second[i] = temp;
					temp = nullptr;
					--i;
				}
				delete temp;
				foundIter->second.pop_back();
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
template< typename T_RegisteringObjectType >
 inline void EventSystem::UnregisterAnObjectFromAllEvents( T_RegisteringObjectType* registeredObject )
 {
	for( auto iter = m_subscribers.begin(); iter != m_subscribers.end(); ++ iter )
	{
		UnregisterAnObjectFromAnEvent( iter->first, registeredObject );
	}
 }

//-----------------------------------------------------------------------------------------------
inline void EventSystem::FireEvent( const std::string& eventName, NamedProperties& arguments )
{
	auto foundIter = m_subscribers.find( eventName );

	if( foundIter != m_subscribers.end() )
	{
		auto eventIter = foundIter->second.begin();

		for( ; eventIter != foundIter->second.end(); ++ eventIter )
		{
			( *eventIter )->FireRegisteredEvent( arguments );
		}
	}
}


#endif