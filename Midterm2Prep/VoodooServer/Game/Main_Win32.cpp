#define WIN32_LEAN_AND_MEAN
#define UNUSED(x) (void)(x);

//-----------------------------------------------------------------------------------------------
#include <windows.h>
#include <cassert>
#include <crtdbg.h>

#include "AppCommandLineListener.hpp"
#include "Game.hpp"

Game g_theGame;

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	AppCommandLineListener& appListener = AppCommandLineListener::GetInstance();

	g_theGame.RegisterForEvents();
	appListener.ProcessCommandLineArguments( commandLineString );
	g_theGame.Initialize(applicationInstanceHandle);
	
	g_theGame.Run();

#if defined( _WIN32 ) && defined( _DEBUG )
	assert( _CrtCheckMemory() );
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}