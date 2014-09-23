#include "EngineCommandLineListener.hpp"

#include <string>
#include <Windows.h>

//-----------------------------------------------------------------------------------------------
void EngineCommandLineListener::ExampleFunction( NamedProperties& parameters )
{
	std::string outputString;

	parameters.Get( "param1", outputString );

	outputString = "Engine's version of string parameter: " + outputString;

	OutputDebugStringA( outputString.c_str() );
}