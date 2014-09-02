#define UNUSED(x) (void)(x);

#include "Game.hpp"

#include <time.h>
#include <random>
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Utilities/Time.hpp"
#include "Engine/Utilities/Clock.hpp"
#include "Engine/Utilities/XMLUtilities.hpp"
#include "Engine/Utilities/NamedProperties.hpp"
#include "Engine/Utilities/EventSystem.hpp"
#include "Engine/Utilities/ProfileSection.hpp"
#include "Engine/Utilities/ErrorWarningAssert.hpp"
#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Primitives/Color.hpp"

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment( lib, "WS2_32" ) // Link in the WS2_32.lib static library


//-----------------------------------------------------------------------------------------------

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;

const double MAX_APP_FRAME_TIME = 0.1;

FMOD::System* Game::m_soundSystem = nullptr;
FMOD::Sound *test = nullptr;
FMOD::Channel *channel = nullptr;

Clock* Game::s_appClock = nullptr;

int g_currentLogIndex = 0;

#define UDP_PORT_FOR_LOCAL "0"
#define LOCAL_IP_ADDRESS "127.0.0.1"
#define BUFFER_LIMIT 512

#define STARTING_SERVER_IP "127.0.0.1"
#define STARTING_SERVER_PORT "5554"

const playerID PLAYER_ID( 255, 0, 0 );

 u_long BLOCKING = 0;
 u_long NON_BLOCKING = 1;

struct packet
{
	char id;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	float x;
	float y;
};

struct playerID
{
	playerID()
		: r( 0 )
		, g( 0 )
		, b( 0 )
		
	{

	}
	playerID( unsigned char red, unsigned char green, unsigned char blue )
		: r( red )
		, g( green )
		, b( blue )
	{

	}

	bool operator==( const playerID& other )
	{
		return r == other.r && g == other.g && b == other.b;
	}

	unsigned char r;
	unsigned char g;
	unsigned char b;

};

struct player
{
	player()
		: id()
		, currentPos( Vector2f::Zero() )
		, desiredPos( Vector2f::Zero() )
		, currentInterpolatedTime( 0.f )
		, interpolationDuration( 0.f )
	{

	}

	playerID id;
	Vector2f currentPos;
	Vector2f desiredPos;
	float currentInterpolatedTime;
	float interpolationDuration;
};

const Vector2f INITIAL_PLAYER_POSITION( 0.f, 0.f );
const float INTERPOLATION_SCALE = 0.1f;
player g_localPlayer;

std::vector< player > g_players;

const char PACKET_ID = 2;

const float SEND_FREQUENCY = 0.1f;
float g_currentElapsedTimeToSendToServer = 0.f;

sockaddr_in UDPServerAddressInfo;
sockaddr_in UDPClientAddressInfo;
SOCKET sendToServerSocket = INVALID_SOCKET;

//-----------------------------------------------------------------------------------------------
bool InitializeWinSocket();
bool CreateAddressInfoForUDP( const std::string& ipAddress, const std::string& udpPort, sockaddr_in& udpAddressInfo );
bool CreateUDPSocket( sockaddr_in& addressInfo, SOCKET& newSocket );
bool BindSocket( SOCKET& socketToBind, sockaddr_in& addressInfo );
bool UDPSendMessage( const SOCKET& socketToSendThru, const char* message, int messageLength, sockaddr_in& udpAddressInfo );
bool UDPClientRecieveMessage( const SOCKET& serverSocket );
bool ChangeServerInfo( const std::string& serverIP, const std::string& serverPort, sockaddr_in& UDPServerAddressInfo_out );
void RenderPlayers();
void RenderPlayer( const player& playerToRender );
void UpdatePlayersElapsedTimes();
void UpdateOrAddClientToListOfPlayers( const char* buffer );
packet CreatePacketFromBuffer( const char* buffer );
void UpdateCurrentPlayer( player& playerToUpdate, float desiredX, float desiredY );
//-----------------------------------------------------------------------------------------------

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

	Clock& masterClock = Clock::GetMasterClock();
	s_appClock = new Clock( masterClock, MAX_APP_FRAME_TIME );

	InitializeLog();
	Time::InitializeTime();
	ProfileSection::InitializeProfiler();

	FMOD::System_Create( &Game::m_soundSystem );

	Game::m_soundSystem->init( 100, FMOD_INIT_NORMAL, 0 );

	m_inputHandler.Initialize( OpenGLRenderer::GetWindow() );

	RegisterEvent("quit", Command_Quit);
	RegisterEvent("clear",Command_Clear);

	g_localPlayer.id.r = PLAYER_ID.r;
	g_localPlayer.id.g = PLAYER_ID.g;
	g_localPlayer.id.b = PLAYER_ID.b;
	g_localPlayer.currentPos = INITIAL_PLAYER_POSITION;

	bool winsocketSetup = true;

	winsocketSetup = InitializeWinSocket();
	FATAL_ASSERTION( winsocketSetup == true, "Winsocket didn't initiliaze." );
	
	winsocketSetup = ChangeServerInfo( STARTING_SERVER_IP, STARTING_SERVER_PORT, UDPServerAddressInfo );
	FATAL_ASSERTION( winsocketSetup == true, "Creating addr info for server failed." );

	winsocketSetup = CreateAddressInfoForUDP( LOCAL_IP_ADDRESS, UDP_PORT_FOR_LOCAL, UDPClientAddressInfo );
	FATAL_ASSERTION( winsocketSetup == true, "Creating addr info for client failed." );

	CreateUDPSocket( UDPServerAddressInfo, sendToServerSocket );

	BindSocket( sendToServerSocket, UDPClientAddressInfo );

	ioctlsocket( sendToServerSocket, FIONBIO, &NON_BLOCKING );
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
		ProfileSection profileGameUpdate( "GameRun" );

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

	UDPClientRecieveMessage( sendToServerSocket );
	UpdatePacketAndPotentiallySendToServer();
	UpdatePlayersElapsedTimes();

	timeAtLastUpdate = timeNow;
}

//-----------------------------------------------------------------------------------------------
void Game::Render()
{
	Vector4f clearColor = OpenGLRenderer::s_clearColor;

	OpenGLRenderer::ClearColor( clearColor.x, clearColor.y, clearColor.z, clearColor.w );
	OpenGLRenderer::Clear( GL_COLOR_BUFFER_BIT );

	RenderPlayers();

	ConsoleLog::s_currentLog->Render();
	SwapBuffers( OpenGLRenderer::displayDeviceContext );
}

//-----------------------------------------------------------------------------------------------
void RenderPlayers()
{
	RenderPlayer( g_localPlayer );

	for( auto iter = g_players.begin(); iter != g_players.end(); ++iter )
	{
		RenderPlayer( *iter );
	}
}

//-----------------------------------------------------------------------------------------------
void RenderPlayer( const player& playerToRender )
{
	Color renderColor;
	Vector3f renderColorAsNormalizedVector;
	Vector2f minPosition;
	Vector2f maxPosition;
	float currentTime;
	float maxTime;
	float normalizedTime;

	renderColor.r = playerToRender.id.r;
	renderColor.g = playerToRender.id.g;
	renderColor.b = playerToRender.id.b;
	renderColorAsNormalizedVector = renderColor.ToVector3fNormalizedAndFullyOpaque();

	currentTime = playerToRender.currentInterpolatedTime;
	maxTime = playerToRender.interpolationDuration;

	if( maxTime > 0.f )
	{
		normalizedTime = currentTime / maxTime;
	}
	else
	{
		normalizedTime = 0.f;
	}

	minPosition = playerToRender.currentPos;
	minPosition = ( 1.f - normalizedTime ) * minPosition + ( normalizedTime ) * playerToRender.desiredPos;
	maxPosition = Vector2f( minPosition.x + 1.f, minPosition.y + 1.f );

	OpenGLRenderer::UseShaderProgram( OpenGLRenderer::s_fixedFunctionPipelineShaderID );
	OpenGLRenderer::Uniform1i( OpenGLRenderer::s_fixedFunctionUseTexturesUniformLocation, 0 );

	OpenGLRenderer::s_rendererStack.Push( MatrixStack44f::MODEL_STACK );
	{
		OpenGLRenderer::Disable( GL_DEPTH_TEST );
		OpenGLRenderer::Disable( GL_CULL_FACE );

		OpenGLRenderer::s_rendererStack.ApplyOrtho( 0.f, ( float )OpenGLRenderer::s_screenSize.x, 0.f, ( float )OpenGLRenderer::s_screenSize.y, 0.f, 1.f ) ;
		OpenGLRenderer::LoadMatrix( OpenGLRenderer::s_rendererStack.GetMVWithProjection() );

		OpenGLRenderer::Render2DQuad( minPosition, maxPosition, renderColorAsNormalizedVector );
	}
	OpenGLRenderer::s_rendererStack.Pop( MatrixStack44f::MODEL_STACK );
}

//-----------------------------------------------------------------------------------------------
void UpdatePlayersElapsedTimes()
{
	const Clock& clock = Clock::GetMasterClock();
	for( auto iter = g_players.begin(); iter != g_players.end(); ++iter )
	{
		iter->currentInterpolatedTime += static_cast< float >( clock.m_currentDeltaSeconds );
	}
}

//-----------------------------------------------------------------------------------------------
void Game::ProcessInput()
{
	const InputHandler::InputHandlerState& currentState = InputHandler::GetInputState();

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
			g_localPlayer.currentPos.y += 1.f;
		}
		else if( InputHandler::isKeyDown( VK_DOWN ) || InputHandler::isKeyDown( 'S' ) )
		{
			g_localPlayer.currentPos.y -= 1.f;
		}

		if( InputHandler::isKeyDown( VK_LEFT ) || InputHandler::isKeyDown( 'A' ) )
		{
			g_localPlayer.currentPos.x -= 1.f;
		}
		else if( InputHandler::isKeyDown( VK_RIGHT ) || InputHandler::isKeyDown( 'D' ) )
		{
			g_localPlayer.currentPos.x += 1.f;
		}
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
void Game::UpdatePacketAndPotentiallySendToServer()
{
	const Clock& masterClock = Clock::GetMasterClock();
	g_currentElapsedTimeToSendToServer += static_cast< float >( masterClock.m_currentDeltaSeconds );

	if( g_currentElapsedTimeToSendToServer >= SEND_FREQUENCY )
	{
		SendSimplePacketToServer();
		g_currentElapsedTimeToSendToServer = 0.f;
	}
}

//-----------------------------------------------------------------------------------------------
void Game::SendSimplePacketToServer()
{
	packet packetToSend;

	packetToSend.id = PACKET_ID;
	packetToSend.r = g_localPlayer.id.r;
	packetToSend.g = g_localPlayer.id.g;
	packetToSend.b = g_localPlayer.id.b;
	packetToSend.x = g_localPlayer.currentPos.x;
	packetToSend.y = g_localPlayer.currentPos.y;

	bool result = UDPSendMessage( sendToServerSocket, ( char* )&packetToSend, sizeof( packetToSend ), UDPServerAddressInfo );

	FATAL_ASSERTION( result == true, "Couldn't send message." );
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
bool InitializeWinSocket()
{
	WSAData winSocketAData;

	int initializationResult;
	initializationResult = WSAStartup( MAKEWORD( 2,2 ), &winSocketAData );

	if( initializationResult != 0 )
	{
		printf( "WSAStartup failed %d\n", initializationResult );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool CreateAddressInfoForUDP( const std::string& ipAddress, const std::string& udpPort, sockaddr_in& udpAddressInfo )
{
	ZeroMemory( &udpAddressInfo, sizeof( udpAddressInfo ) );
	udpAddressInfo.sin_family = AF_INET;
	udpAddressInfo.sin_addr.S_un.S_addr = inet_addr( ipAddress.c_str() );
	udpAddressInfo.sin_port = htons( u_short( atoi( udpPort.c_str() ) ) );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool CreateUDPSocket( sockaddr_in& addressInfo, SOCKET& newSocket )
{
	newSocket = socket( addressInfo.sin_family, SOCK_DGRAM, IPPROTO_UDP );

	if( newSocket == INVALID_SOCKET )
	{
		printf( "socket failed with error: %ld\n", WSAGetLastError() );
		WSACleanup();

		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool BindSocket( SOCKET& socketToBind, sockaddr_in& addressInfo )
{
	int bindResultAsInt;

	bindResultAsInt = bind( socketToBind, (struct sockaddr *)&addressInfo, sizeof( addressInfo ) );

	if( bindResultAsInt == SOCKET_ERROR )
	{
		printf( "bind failed with error: %d\n", WSAGetLastError() );

		closesocket( socketToBind );
		WSACleanup();

		return false;
	}	

	return true;
}

//-----------------------------------------------------------------------------------------------
bool UDPSendMessage( const SOCKET& socketToSendTo, const char* message, int messageLength, sockaddr_in& udpAddressInfo )
{
	int sendResultAsInt;

	sendResultAsInt = sendto( socketToSendTo, message, messageLength, 0, (struct sockaddr *)&udpAddressInfo, sizeof( udpAddressInfo ) );

	if ( sendResultAsInt == SOCKET_ERROR ) 
	{
		printf( "send failed with error: %d\n", WSAGetLastError() );
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------------------------
bool UDPClientRecieveMessage( const SOCKET& serverSocket )
{
	char receiveBuffer[ BUFFER_LIMIT ];
	memset( receiveBuffer, '\0', BUFFER_LIMIT );

	int bytesReceived = 0;
	int senderInfoLength = sizeof( UDPServerAddressInfo );

	bytesReceived = recvfrom( serverSocket, receiveBuffer, BUFFER_LIMIT, 0, (struct sockaddr *)&UDPServerAddressInfo, &senderInfoLength );

	if( bytesReceived == sizeof( packet ) )
	{
		UpdateOrAddClientToListOfPlayers( receiveBuffer );
		return true;
	}
	else
	{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
bool ChangeServerInfo( const std::string& serverIP, const std::string& serverPort, sockaddr_in& UDPServerAddressInfo_out )
{
	return CreateAddressInfoForUDP( serverIP, serverPort, UDPServerAddressInfo_out );
}


//-----------------------------------------------------------------------------------------------
void UpdateOrAddClientToListOfPlayers( const char* buffer )
{
	packet newPacket = CreatePacketFromBuffer( buffer );

	playerID idFromPacket;
	idFromPacket.r = newPacket.r;
	idFromPacket.g = newPacket.g;
	idFromPacket.b = newPacket.b;

	auto iter = g_players.begin();

	for( ; iter != g_players.end(); ++iter )
	{
		if( iter->id == idFromPacket )
		{
			UpdateCurrentPlayer( *iter, newPacket.x, newPacket.y );
			break;
		}	
	}

	if( iter == g_players.end() && !(iter->id == g_localPlayer.id) )
	{
		player newPlayer;

		newPlayer.currentPos = Vector2f( newPacket.x, newPacket.y );
		newPlayer.desiredPos = newPlayer.currentPos;
		newPlayer.interpolationDuration = 0.f;
		newPlayer.currentInterpolatedTime = 0.f;

		g_players.push_back( newPlayer );
	}
}

//-----------------------------------------------------------------------------------------------
packet CreatePacketFromBuffer( const char* buffer )
{
	packet* result = nullptr;

	result = ( packet* )( buffer );

	return *result;
}

//-----------------------------------------------------------------------------------------------
void UpdateCurrentPlayer( player& playerToUpdate, float desiredX, float desiredY )
{
	float normalizedTime = 0.f;
	float currentTime = playerToUpdate.currentInterpolatedTime;
	float maxTime = playerToUpdate.interpolationDuration;

	if( maxTime > 0.f )
	{
		normalizedTime = currentTime / maxTime;
	}

	playerToUpdate.currentPos = ( 1.f - normalizedTime ) * playerToUpdate.currentPos + ( normalizedTime ) * playerToUpdate.desiredPos;
	
	playerToUpdate.currentInterpolatedTime = 0.f;
	playerToUpdate.desiredPos = Vector2f( desiredX, desiredY );

	playerToUpdate.interpolationDuration = Vector2f::Magnitude( playerToUpdate.desiredPos - playerToUpdate.currentPos ) * INTERPOLATION_SCALE;
}