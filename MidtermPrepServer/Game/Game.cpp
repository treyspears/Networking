#define UNUSED(x) (void)(x);

#define BUFFER_LIMIT 512

#include "Game.hpp"

#include <time.h>
#include <random>
#include <string>
#include <sstream>

#include <stdlib.h>
#include <stdio.h>
#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Networking/Network.hpp"
#include "Engine/Utilities/Time.hpp"
#include "Engine/Utilities/Clock.hpp"
#include "Engine/Utilities/XMLUtilities.hpp"
#include "Engine/Utilities/ProfileSection.hpp"
#include "Engine/Utilities/ErrorWarningAssert.hpp"
#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Primitives/Color.hpp"

#include "CS6Packet.hpp"

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment( lib, "WS2_32" ) // Link in the WS2_32.lib static library

//const playerID BAD_COLOR( 0, 0, 0 );
//-----------------------------------------------------------------------------------------------

const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 600;

const double MAX_APP_FRAME_TIME = 0.1;

FMOD::System* Game::m_soundSystem = nullptr;
FMOD::Sound *test = nullptr;
FMOD::Channel *channel = nullptr;

Clock* Game::s_appClock = nullptr;

int g_currentLogIndex = 0;

//-----------------------------------------------------------------------------------------------

//void OnReceiveGameStart( const CS6Packet& packet );
//void OnReceiveReset( const CS6Packet& packet );
//void OnReceiveUpdate( const CS6Packet& buffer );
//void OnReceiveAck( const CS6Packet& bufferAsPacket );
//void OnReceiveVictory( const CS6Packet& bufferAsPacket );

//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
//Registered Commands
//-----------------------------------------------------------------------------------------------
void Command_Quit(const ConsoleCommandArgs& args)
{
	UNUSED(args);
	InputHandler::QuitApplication();
	return;
}

void Command_Clear(const ConsoleCommandArgs& args)
{
	UNUSED(args);
	if(ConsoleLog::s_currentLog != nullptr)
	{
		ConsoleLog::s_currentLog->ClearConsoleLog();
	}
}

//-----------------------------------------------------------------------------------------------
//void Command_ChangeHost( const ConsoleCommandArgs& args )
//{
//	const int numArgs = 3;
//	const Vector4f ERROR_COLOR = Vector4f( 1.f, 0.f, 0.f, 1.f );
//
//	std::string ipAddressAsString;
//	std::string portAsString; 
//
//
//	if( args.m_argsList.size() < numArgs )
//	{
//		ConsoleLog::s_currentLog->ConsolePrint( "Error: changeHost requires 2 parameters, IP Address and Port", ERROR_COLOR, true );
//		return;
//	}
//
//	ipAddressAsString = args.m_argsList[ 1 ];
//
//	portAsString = args.m_argsList[ 2 ];
//
//	if( ipAddressAsString == "" || portAsString == "" )
//	{
//		ConsoleLog::s_currentLog->ConsolePrint( "Error: changeHost requires 2 parameters, IP Address and Port", ERROR_COLOR, true );
//		return;
//	}
//
//	ChangeHost( ipAddressAsString, portAsString );
//}

//-----------------------------------------------------------------------------------------------
Game::Game()
{
	srand( ( unsigned int )time ( NULL ) );
}

//-----------------------------------------------------------------------------------------------
Game::~Game()
{
	for( int i = 0; i < static_cast< int >( ConsoleLog::s_logs.size() ); ++ i )
	{
		delete ConsoleLog::s_logs[ i ];
	}
}

//-----------------------------------------------------------------------------------------------
void Game::Initialize(HINSTANCE applicationInstanceHandle)
{
	OpenGLRenderer::CreateOpenGLWindow(applicationInstanceHandle, SCREEN_WIDTH, SCREEN_HEIGHT);
	InitializeLog();
	Time::InitializeTime();
	m_inputHandler.Initialize( OpenGLRenderer::GetWindow() );
	
	FMOD::System_Create( &Game::m_soundSystem );
	Game::m_soundSystem->init( 100, FMOD_INIT_NORMAL, 0 );

	CommandRegistry& commandRegistry = CommandRegistry::GetInstance();

	commandRegistry.RegisterEvent( "quit", Command_Quit );
	commandRegistry.RegisterEvent( "clear",Command_Clear );
	//commandRegistry.RegisterEvent( "changeHost", Command_ChangeHost );

	Clock& masterClock = Clock::GetMasterClock();
	s_appClock = new Clock( masterClock, MAX_APP_FRAME_TIME );

	m_server.StartUp();
	//m_client.StartUp();
}

//-----------------------------------------------------------------------------------------------
void Game::InitializeLog()
{
	BitmapFont* font = BitmapFont::CreateOrGetFont("Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml");

	ConsoleLog::s_logs.push_back( new ConsoleLog( font,18.f ) );
	ConsoleLog::s_currentLog = ConsoleLog::s_logs.back();
}

//-----------------------------------------------------------------------------------------------
void Game::Run()
{
	while(!m_inputHandler.ShouldQuit())
	{
		m_inputHandler.RunMessagePump();
		m_inputHandler.RunXboxMessagePump();
		m_soundSystem->update();
		ProcessInput();
		Update();
		Render();
	}
}

//-----------------------------------------------------------------------------------------------
void Game::Update()
{
	static double timeAtLastUpdate = Time::GetCurrentTimeInSeconds();
	double timeNow = Time::GetCurrentTimeInSeconds();

	float deltaSeconds = static_cast< float >(timeNow - timeAtLastUpdate);

	ConsoleLog::s_currentLog->Update( deltaSeconds );

	Clock& masterClock = Clock::GetMasterClock();
	masterClock.AdvanceTime( timeNow - timeAtLastUpdate );

	m_server.Update();
	//m_client.Update();
	
	timeAtLastUpdate = timeNow;
}

//-----------------------------------------------------------------------------------------------
void Game::Render()
{
	const Vector4f clearColor = Color( Black ).ToVector4fNormalized();

	OpenGLRenderer::ClearColor( clearColor.x, clearColor.y, clearColor.z, clearColor.w );
	OpenGLRenderer::Clear( GL_COLOR_BUFFER_BIT );

	m_server.Render();
	//m_client.Render();
	
	ConsoleLog::s_currentLog->Render();
	SwapBuffers( OpenGLRenderer::displayDeviceContext );
}

//-----------------------------------------------------------------------------------------------
void Game::ProcessInput()
{
	const InputHandler::InputHandlerState& currentState = InputHandler::GetInputState();

	Vector2f localPlayerMovementVector = Vector2f::Zero();

	if( InputHandler::isKeyPressed(VK_OEM_3) )
	{
		ConsoleLog::s_currentLog->ToggleLogToRender();

		if( currentState == InputHandler::INPUTHANDLER_GAME )
			InputHandler::SetInputState(InputHandler::INPUTHANDLER_DEV_CONSOLE);
		else
			InputHandler::SetInputState(InputHandler::INPUTHANDLER_GAME);
	}

	if( currentState == InputHandler::INPUTHANDLER_DEV_CONSOLE )
	{
		UpdateConsoleLogOnInput();
	}
	else
	{
		if( InputHandler::isKeyDown( VK_UP ) || InputHandler::isKeyDown( 'W' ) )
		{
			localPlayerMovementVector += Vector2f( 0.f, 1.f );
		}
		else if( InputHandler::isKeyDown( VK_DOWN ) || InputHandler::isKeyDown( 'S' ) )
		{
			localPlayerMovementVector += Vector2f( 0.f, -1.f );
		}

		if( InputHandler::isKeyDown( VK_LEFT ) || InputHandler::isKeyDown( 'A' ) )
		{
			localPlayerMovementVector += Vector2f( -1.f, 0.f );
		}
		else if( InputHandler::isKeyDown( VK_RIGHT ) || InputHandler::isKeyDown( 'D' ) )
		{
			localPlayerMovementVector += Vector2f( 1.f, 0.f );
		}
	}

	//m_client.UpdateLocalPlayerMovementVector( localPlayerMovementVector );
}

//-----------------------------------------------------------------------------------------------
void Game::UpdateConsoleLogOnInput()
{
	if(InputHandler::isKeyPressed(VK_PRIOR))
	{
		ConsoleLog::s_currentLog->ScrollDownLog();
	}
	else if(InputHandler::isKeyPressed(VK_NEXT))
	{
		ConsoleLog::s_currentLog->ScrollUpLog();
	}
	else if( InputHandler::isKeyPressed(VK_RIGHT))
	{
		++g_currentLogIndex;
		ConsoleLog::s_currentLog->ToggleLogToRender();
		if( g_currentLogIndex > static_cast< int >( ConsoleLog::s_logs.size() ) - 1 )
		{
			g_currentLogIndex = 0;
		}
		ConsoleLog::s_currentLog = ConsoleLog::s_logs[ g_currentLogIndex ];
		ConsoleLog::s_currentLog->ToggleLogToRender();
	}
	else if( InputHandler::isKeyPressed(VK_LEFT))
	{
		--g_currentLogIndex;
		ConsoleLog::s_currentLog->ToggleLogToRender();
		if( g_currentLogIndex < 0 )
		{
			g_currentLogIndex = ConsoleLog::s_logs.size() - 1;
		}
		ConsoleLog::s_currentLog = ConsoleLog::s_logs[ g_currentLogIndex ];
		ConsoleLog::s_currentLog->ToggleLogToRender();
	}
	else if(InputHandler::isKeyPressed(VK_ESCAPE))
	{
		int logLength = ConsoleLog::s_currentLog->GetCommandPromptLength();
		if(logLength == 1)
		{
			InputHandler::SetInputState(InputHandler::INPUTHANDLER_GAME);
			ConsoleLog::s_currentLog->ToggleLogToRender();
			return;
		}
		else
		{
			ConsoleLog::s_currentLog->ClearCommandPrompt();
		}
	}
	else if(InputHandler::isKeyPressed(VK_RETURN))
	{
		int logLength = ConsoleLog::s_currentLog->GetCommandPromptLength();
		if(logLength == 1)
		{
			InputHandler::SetInputState(InputHandler::INPUTHANDLER_GAME);
			ConsoleLog::s_currentLog->ToggleLogToRender();
			return;
		}
		else
		{
			//need to add thing here
			ConsoleLog::s_currentLog->EnterCommand();
			ConsoleLog::s_currentLog->ClearCommandPrompt();
		}
	}
	else if(InputHandler::isKeyPressed(VK_BACK))
	{
		ConsoleLog::s_currentLog->BackSpace();
	}
	else
	{
		for(int i = 0; i < NUM_VIRTUAL_KEYS; ++i)
		{
			if(InputHandler::m_currentCharState[i])
			{
				std::string string;
				string.push_back((uchar)i);
				ConsoleLog::s_currentLog->PrintToCommandPrompt(string);
				InputHandler::m_currentCharState[i] = false;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
//void Game::PotentiallyRequestGameStartFromServer()
//{
//	if( g_currentElapsedTimeToSendToServer < SEND_FREQUENCY )
//	{
//		return;
//	}
//
//	CS6Packet requestGameStartPacket;
//
//	ZeroMemory( &requestGameStartPacket, sizeof( requestGameStartPacket ) );
//
//	requestGameStartPacket.packetType = TYPE_Acknowledge;
//	requestGameStartPacket.data.acknowledged.packetType = TYPE_Acknowledge;
//
//	UDPSendMessageToServer( sendToServerSocket, UDPServerAddressInfo, ( char* )&requestGameStartPacket, sizeof( CS6Packet ) );
//
//	g_currentElapsedTimeToSendToServer = 0.f;
//
//}


//-----------------------------------------------------------------------------------------------
void Game::CreateSound(const char* songPath, FMOD::Sound* &sound)
{
	FMOD_RESULT result;
	result = Game::m_soundSystem->createSound( songPath,FMOD_DEFAULT, 0, &sound );
	assert( result == FMOD_OK );
}


//-----------------------------------------------------------------------------------------------
void Game::PlaySound( FMOD::Sound* &sound )
{
	FMOD_RESULT result;
	result = Game::m_soundSystem->playSound( FMOD_CHANNEL_FREE, sound, false, 0 );
	assert( result == FMOD_OK );
}


//-----------------------------------------------------------------------------------------------
void Game::PlayRandomSound( std::vector<FMOD::Sound*> &sounds )
{
	int soundToPlay = rand() % sounds.size();
	PlaySound( sounds[soundToPlay] );
}
