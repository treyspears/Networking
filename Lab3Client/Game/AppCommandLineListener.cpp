#include "AppCommandLineListener.hpp"

#include <string>
#include <Windows.h>
#include <fstream>

#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Utilities/ErrorWarningAssert.hpp"
#include "Engine/Utilities/InputHandler.hpp"

#include "Engine/Utilities/NewAndDeleteOverrides.hpp"

//-----------------------------------------------------------------------------------------------
void AppCommandLineListener::ExampleFunction( NamedProperties& parameters )
{
	std::string stringFromParams;
	std::string outputString;

	parameters.Get( "param1", stringFromParams );

	outputString = "Application's version of string parameter: " + stringFromParams;

	stringFromParams = stringFromParams + " and some more";

	parameters.Set( "param1", stringFromParams );

	OutputDebugStringA( outputString.c_str() );
}

//-----------------------------------------------------------------------------------------------
void AppCommandLineListener::GenerateFiles( NamedProperties& parameters )
{
	std::string numFilesAsString;
	std::string sizeOfFilesAsString;

	parameters.Get( "param1", numFilesAsString );
	parameters.Get( "param2", sizeOfFilesAsString );

	std::set< ErrorType > errors, additionalErrors;

	errors = ValidateIsInt( numFilesAsString );
	additionalErrors = ValidateIsInt( sizeOfFilesAsString );

	errors.insert( additionalErrors.begin(), additionalErrors.end() );
	
	RECOVERABLE_ASSERTION( errors.empty(), "Command: generateFiles did not receive the correct parameters.\ngenerateFiles expects two parameters of type integer." );

	if( !errors.empty() )
		return;

	int numFiles = static_cast<int>( strtol( numFilesAsString.c_str(), 0, 10 ) );
	int sizeOfFilesInKB = static_cast<int>( strtol( sizeOfFilesAsString.c_str(), 0, 10 ) );

	CreateFiles( numFiles, sizeOfFilesInKB );
}

//-----------------------------------------------------------------------------------------------
void AppCommandLineListener::CreateFiles( int numFiles, int sizeOfFilesInKB )
{
	char fileNumAsCharacter[ 32 ];
	std::string fileBaseName = "Data/GeneratedFiles/generated_";

	fileBaseName += std::string( _itoa( sizeOfFilesInKB, fileNumAsCharacter, 10 ) );
	fileBaseName += "KB_";

	int bytesToOutput = 1024 * sizeOfFilesInKB;

	std::string fileName;
	int randomUCharAsInt;
	uchar* randomUCharBuffer = new uchar[ bytesToOutput ]; 

	for( int fileIndex = 1; fileIndex <= numFiles; ++ fileIndex )
	{
		FILE *fileToCreate = NULL;

		fileName = fileBaseName + std::string( _itoa( fileIndex, fileNumAsCharacter, 10 ) );
		fileName += ".dat";
		
		for( int byteIndex = 0; byteIndex < bytesToOutput; ++ byteIndex )
		{
			randomUCharAsInt = rand() % 256;

			randomUCharBuffer[ byteIndex ] = static_cast< uchar >( randomUCharAsInt );
		}

		fileToCreate = fopen( fileName.c_str(), "wb+" );
		
		fwrite( &randomUCharBuffer, bytesToOutput * sizeof( uchar ), 1, fileToCreate );
		fclose( fileToCreate );
	}
		
	delete randomUCharBuffer;

	InputHandler::QuitApplication();
}

//-----------------------------------------------------------------------------------------------
void AppCommandLineListener::ProcessCommandLineArguments( const char* commandLineString )
{
	std::vector< std::vector < std::string > > commandsWithArgsList = GetCommandsToFire( commandLineString );

	this->Initialize();

	EventSystem& eventSystem = EventSystem::GetInstance();

	for( unsigned int i = 0; i < commandsWithArgsList.size(); ++ i )
	{
		if( InputHandler::ShouldQuit() )
			break;

		NamedProperties eventParams = NamedProperties::EmptyProperties();

		std::string eventName = commandsWithArgsList[ i ][ 0 ];

		for( unsigned int j = 1; j < commandsWithArgsList[ i ].size(); ++ j )
		{
			char buffer[ 32 ];

			std::string paramName = "param";
			paramName += std::string( _itoa( j, buffer, 10 ) );

			eventParams.Set( paramName, commandsWithArgsList[ i ][ j ] );
		}

		eventSystem.FireEvent( eventName, eventParams );
	}
}

//-----------------------------------------------------------------------------------------------
std::vector< std::vector< std::string > > AppCommandLineListener::GetCommandsToFire( const char* commandLineString )
{
	std::vector< std::vector < std::string > > commandsWithArgsList;

	std::string commandLineUnparsed = commandLineString;

	std::vector< std::string > currentCommandWithArgs;
	std::string currentParsedString;
	bool isParsingString = false;

	for( unsigned int i = 0; i < commandLineUnparsed.size(); ++ i )
	{
		char currentCharacter = commandLineUnparsed[ i ];

		if( currentCharacter == '-' )
		{
			if( !currentCommandWithArgs.empty() )
			{
				commandsWithArgsList.push_back( currentCommandWithArgs );
				currentParsedString.clear();
				currentCommandWithArgs.clear();
			}
		}
		else if( isspace( currentCharacter ) && !isParsingString )
		{
			if( !currentParsedString.empty() )
			{
				currentCommandWithArgs.push_back( currentParsedString );
				currentParsedString.clear();
			}
		}
		else if( currentCharacter == '"' )
		{
			isParsingString = !isParsingString;
		}
		else
		{
			currentParsedString += currentCharacter;
		}
	}

	if( !currentCommandWithArgs.empty() )
	{
		if( !currentParsedString.empty() )
			currentCommandWithArgs.push_back( currentParsedString );

		commandsWithArgsList.push_back( currentCommandWithArgs );
	}

	return commandsWithArgsList;
}