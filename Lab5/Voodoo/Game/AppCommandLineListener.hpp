#ifndef APP_COMMAND_LINE_LISTENER
#define APP_COMMAND_LINE_LISTENER

#include <vector>
#include <string>

#include "Engine/Utilities/NamedProperties.hpp"
#include "Engine/Utilities/EventSystem.hpp"
#include "Engine/Utilities/EngineCommandLineListener.hpp"

//-----------------------------------------------------------------------------------------------
class AppCommandLineListener
{
public:

	static AppCommandLineListener& GetInstance()
	{
		static AppCommandLineListener instance; 

		return instance;
	}

	void ProcessCommandLineArguments( const char* commandLineString );
	
private:

	inline void Initialize();
	std::vector< std::vector< std::string > > GetCommandsToFire( const char* commandLineString );
	void ExampleFunction( NamedProperties& parameters );
	void GenerateFiles( NamedProperties& parameters );
	void CreateFiles( int numFiles, int sizeOfFilesInKB );

	AppCommandLineListener();
	~AppCommandLineListener();

	AppCommandLineListener( AppCommandLineListener const& );              
	void operator=( AppCommandLineListener const& ); 

};

//-----------------------------------------------------------------------------------------------
inline AppCommandLineListener::AppCommandLineListener()
{

}

//-----------------------------------------------------------------------------------------------
inline AppCommandLineListener::~AppCommandLineListener()
{
	//EventSystem& eventSystem = EventSystem::GetInstance();

	//eventSystem.UnregisterAnObjectFromAllEvents( this );
}

//-----------------------------------------------------------------------------------------------
inline void AppCommandLineListener::Initialize()
{
	EventSystem& eventSystem = EventSystem::GetInstance();

	eventSystem.RegisterEventWithCallbackAndObject( "exampleFunction", &AppCommandLineListener::ExampleFunction, this );
	eventSystem.RegisterEventWithCallbackAndObject( "generateFiles", &AppCommandLineListener::GenerateFiles, this );

	EngineCommandLineListener& engineListener = EngineCommandLineListener::GetInstance(); 
	engineListener.Initialize();
}

#endif