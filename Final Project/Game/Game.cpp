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
#include "Engine/Physics/AABB3.hpp"
#include "Engine/Primitives/EulerAngles.hpp"

#include "FinalPacket.hpp"

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment( lib, "WS2_32" ) // Link in the WS2_32.lib static library

//const playerID BAD_COLOR( 0, 0, 0 );
//-----------------------------------------------------------------------------------------------

int g_screenWidth = 1600;
int g_screenHeight = 900;

const double MAX_APP_FRAME_TIME = 0.1;
const double FIELD_OF_VIEW_Y = 45.0;
const double ASPECT_RATIO = 16.0 / 9.0;
const double FAR_CLIPPING_PLANE = 2000.0;

FMOD::System* Game::m_soundSystem = nullptr;
FMOD::Sound *test = nullptr;
FMOD::Channel *channel = nullptr;

Clock* Game::s_appClock = nullptr;

int g_currentLogIndex = 0;

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
	: m_isClient( true )
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
	OpenGLRenderer::CreateOpenGLWindow( applicationInstanceHandle, g_screenWidth, g_screenHeight );
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

	if( m_isClient )
	{
		m_client.StartUp();
	}
	else
	{
		//m_server.StartUp();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SetupCameraPositionAndOrientation( const Camera3D& camera ) const
{
	OpenGLRenderer::s_rendererStack.Push( MatrixStack44f::VIEW_STACK );

	OpenGLRenderer::s_rendererStack.ApplyRollDegreesAboutXRotation( MatrixStack44f::VIEW_STACK, -90.f );
	OpenGLRenderer::s_rendererStack.ApplyYawDegreesAboutZRotation( MatrixStack44f::VIEW_STACK, 90.f );

	const EulerAnglesf& cameraOrientation = camera.GetOrientation();
	const Vector3f& cameraPosition = camera.GetPosition();

	OpenGLRenderer::s_rendererStack.ApplyRollDegreesAboutXRotation( MatrixStack44f::VIEW_STACK, -cameraOrientation.rollDegreesAboutX );
	OpenGLRenderer::s_rendererStack.ApplyPitchDegreesAboutYRotation( MatrixStack44f::VIEW_STACK, -cameraOrientation.pitchDegreesAboutY );
	OpenGLRenderer::s_rendererStack.ApplyYawDegreesAboutZRotation( MatrixStack44f::VIEW_STACK, -cameraOrientation.yawDegreesAboutZ );

	OpenGLRenderer::s_rendererStack.ApplyTranslation( MatrixStack44f::VIEW_STACK, -cameraPosition );
	OpenGLRenderer::LoadMatrix( OpenGLRenderer::s_rendererStack.GetMVWithProjection() );
}


//-----------------------------------------------------------------------------------------------
void Game::ResetCameraPositionAndOrientation() const
{
	OpenGLRenderer::s_rendererStack.Pop(MatrixStack44f::VIEW_STACK);
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

	if( m_isClient )
	{
		m_client.Update();
	}
	else
	{
		//m_server.Update();
	}

	OpenGLRenderer::Update( deltaSeconds );

	timeAtLastUpdate = timeNow;
}

//-----------------------------------------------------------------------------------------------
void Game::Render()
{
	OpenGLRenderer::Enable( GL_DEPTH_TEST );
	OpenGLRenderer::Enable( GL_CULL_FACE );

	const Vector4f clearColor = Color( Black ).ToVector4fNormalized();

	OpenGLRenderer::ClearColor( clearColor.x, clearColor.y, clearColor.z, clearColor.w );
	OpenGLRenderer::Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	OpenGLRenderer::ClearDepth( 1.f );

	if( m_isClient )
	{
		RenderArena();
	}
	else
	{
		//m_server.Render();
	}

	ConsoleLog::s_currentLog->Render();
	SwapBuffers( OpenGLRenderer::displayDeviceContext );
}

//-----------------------------------------------------------------------------------------------
void Game::ProcessInput()
{
	const InputHandler::InputHandlerState& currentState = InputHandler::GetInputState();

	float movementMagnitude = 0.f;

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
	else if( m_isClient )
	{
		if( m_client.GetClientState() == CLIENT_IN_GAME )
		{
			ClientPlayer* localPlayer = m_client.GetLocalPlayer();

			if( InputHandler::isKeyDown( VK_UP ) || InputHandler::isKeyDown( 'W' ) )
			{
				movementMagnitude = 1.f;
			}

			if( InputHandler::isKeyDown( VK_LEFT ) || InputHandler::isKeyDown( 'A' ) && localPlayer != nullptr )
			{
				localPlayer->RotateForTimeStep( ClientPlayer::LEFT );
			}
			else if( InputHandler::isKeyDown( VK_RIGHT ) || InputHandler::isKeyDown( 'D' ) && localPlayer != nullptr )
			{
				localPlayer->RotateForTimeStep( ClientPlayer::RIGHT );
			}

			if( InputHandler::isKeyPressed( VK_SPACE ) )
			{
				m_client.Shoot();
			}
		}
		else if( m_client.GetClientState() == CLIENT_IN_LOBBY )
		{
			if( InputHandler::isKeyPressed( VK_UP ) || InputHandler::isKeyPressed( 'W' ) )
			{
				m_client.SetGameTargetToPrevious();
			}
			else if( InputHandler::isKeyPressed( VK_DOWN ) || InputHandler::isKeyPressed( 'S' ) )
			{
				m_client.SetGameTargetToNext();
			}

			if( InputHandler::isKeyPressed( 'H' ) )
			{
				m_client.CreateGame();
			}
			else if( InputHandler::isKeyPressed( 'J' ) )
			{
				m_client.JoinTargetedGame();
			}
		}

	}

	if( m_isClient && m_client.GetClientState() == CLIENT_IN_GAME )
	{
		m_client.UpdateLocalPlayerMovementMagnitude( movementMagnitude );
	}
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
void Game::SetScreenResolutionEvent( NamedProperties& parameters )
{
	std::string resXAsString;
	std::string resYAsString;

	parameters.Get( "param1", resXAsString );
	parameters.Get( "param2", resYAsString );

	std::set< ErrorType > errors, additionalErrors;

	errors = ValidateIsInt( resXAsString );
	additionalErrors = ValidateIsInt( resYAsString );
	errors.insert( additionalErrors.begin(), additionalErrors.end() );

	RECOVERABLE_ASSERTION( errors.empty(), "Command: setRes did not receive the correct parameters.\nsetRes expects two parameters of type int that indicate the new x and y resolution." );

	if( !errors.empty() )
		return;

	g_screenWidth = static_cast< int >( strtol( resXAsString.c_str(), 0, 10 ) );
	g_screenHeight = static_cast< int >( strtol( resYAsString.c_str(), 0, 10 ) );
}


//-----------------------------------------------------------------------------------------------
void Game::SetToServerEvent( NamedProperties& parameters )
{
	UNUSED( parameters );

	m_isClient = false;
}


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


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
void Game::RenderArena()
{
	const Camera3D& playerCamera = m_client.GetCameraToRenderFrom();

	if( m_client.GetClientState() == CLIENT_IN_GAME )
	{	
		OpenGLRenderer::s_rendererStack.Push( MatrixStack44f::MODEL_STACK );
		{
			OpenGLRenderer::s_rendererStack.ApplyPerspective( (float) FIELD_OF_VIEW_Y, (float) ASPECT_RATIO, 0.1f, (float) FAR_CLIPPING_PLANE );
			OpenGLRenderer::LoadMatrix( OpenGLRenderer::s_rendererStack.GetMVWithProjection() );

			SetupCameraPositionAndOrientation( playerCamera );
			{
				RenderFloor();

				RenderWall( 0 );
				RenderWall( 1 );
				RenderWall( 2 );
				RenderWall( 3 );

				OpenGLRenderer::RenderDebugShapes();

				m_client.RenderPlayers(); //3d stuff
			}
			ResetCameraPositionAndOrientation();	
		}
		OpenGLRenderer::s_rendererStack.Pop( MatrixStack44f::MODEL_STACK );
	}

	if( m_client.GetClientState() == CLIENT_IN_GAME )
	{	
		OpenGLRenderer::RenderCrossHair( OpenGLRenderer::CROSSHAIR_CROSS );
	}

	m_client.Render2DSprites(); //2d stuff
}

//-----------------------------------------------------------------------------------------------
//0 - East, 1 - North, 2 - West, 3 - South
void Game::RenderWall( int cardinalDirection )
{
	const float ARENA_WIDTH    = 500.f;
	const float ARENA_HEIGHT   = 500.f;
	const float ARENA_DEPTH    = 125.f;
	const float WALL_DEPTH	   = 5.f;
	const float MIN_Z_POSITION = -5.f;

	Vector3f minPosition( Vector3f::Zero() );
	Vector3f maxPosition( Vector3f::Zero() );
	AABB3    wallAABB( minPosition, maxPosition );
	Color	 wallColor = Color( Black );

	switch( cardinalDirection )
	{
	case 0:

		minPosition = Vector3f( ARENA_WIDTH, 0.f, MIN_Z_POSITION );
		maxPosition = Vector3f( ARENA_WIDTH + WALL_DEPTH, ARENA_HEIGHT, ARENA_DEPTH );
		wallColor = Color( DarkSlateBlue );

		break;
	case 1:

		minPosition = Vector3f( 0.f, ARENA_HEIGHT, MIN_Z_POSITION );
		maxPosition = Vector3f( ARENA_WIDTH, ARENA_HEIGHT + WALL_DEPTH, ARENA_DEPTH );
		wallColor = Color( DarkOrchid );

		break;
	case 2:

		minPosition = Vector3f( 0.f, 0.f, MIN_Z_POSITION );
		maxPosition = Vector3f( -WALL_DEPTH, ARENA_HEIGHT, ARENA_DEPTH );
		wallColor = Color( DarkRed );

		break;
	case 3:
	default:

		minPosition = Vector3f( 0.f, 0.f, MIN_Z_POSITION );
		maxPosition = Vector3f( ARENA_WIDTH, -WALL_DEPTH, ARENA_DEPTH );
		wallColor = Color( DarkGoldenrod );

		break;
	}

	wallAABB = AABB3( minPosition, maxPosition );

	OpenGLRenderer::RenderDebugAABB3( wallAABB, wallColor, wallColor );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderFloor()
{
	const Color FLOOR_COLOR = Color( LightGray );
	const float ARENA_WIDTH   = 500.f;
	const float ARENA_HEIGHT  = 500.f;
	const float Z_POSITION    = -5.f;

	Vector3f minPosition( 0.f, 0.f, Z_POSITION );
	Vector3f maxPosition( ARENA_WIDTH, ARENA_HEIGHT, Z_POSITION );
	AABB3    floorAABB( minPosition, maxPosition );

	OpenGLRenderer::RenderDebugAABB3( floorAABB, FLOOR_COLOR, FLOOR_COLOR );
}