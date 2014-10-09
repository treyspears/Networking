#ifndef ENGINE_COMMAND_LINE_LISTENER
#define ENGINE_COMMAND_LINE_LISTENER

#include "Engine/Utilities/NamedProperties.hpp"
#include "Engine/Utilities/EventSystem.hpp"

//-----------------------------------------------------------------------------------------------
class EngineCommandLineListener
{
public:
	
	friend class AppCommandLineListener;

private:

	static EngineCommandLineListener& GetInstance()
	{
		static EngineCommandLineListener instance; 

		return instance;
	}

	inline void Initialize(); 
	void ExampleFunction( NamedProperties& parameters );

	EngineCommandLineListener();
	~EngineCommandLineListener();

	EngineCommandLineListener(EngineCommandLineListener const& );              
	void operator=( EngineCommandLineListener const& ); 

};

//-----------------------------------------------------------------------------------------------
inline EngineCommandLineListener::EngineCommandLineListener()
{

}

//-----------------------------------------------------------------------------------------------
inline EngineCommandLineListener::~EngineCommandLineListener()
{
	//EventSystem& eventSystem = EventSystem::GetInstance();

	//eventSystem.UnregisterAnObjectFromAllEvents( this );
}

//-----------------------------------------------------------------------------------------------
inline void EngineCommandLineListener::Initialize()
{
	EventSystem& eventSystem = EventSystem::GetInstance();

	eventSystem.RegisterEventWithCallbackAndObject( "exampleFunction", &EngineCommandLineListener::ExampleFunction, this );
}

#endif