#include "Client.hpp"

#include "Engine/Rendering/ConsoleLog.hpp"
#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Utilities/ErrorWarningAssert.hpp"
#include "Engine/Utilities/CommandRegistry.hpp"
#include "Engine/Utilities/Clock.hpp"
#include "Engine/Primitives/Vector4.hpp"


//-----------------------------------------------------------------------------------------------
const char* STARTING_SERVER_IP_AS_STRING = "127.0.0.1";
const u_short STARTING_PORT = 5000;

const PlayerID LOCAL_PLAYER_ID = Color( Orange );
const float SEND_TO_HOST_FREQUENCY = 0.025f;
const float SEND_TO_HOST_MAX_DELAY = 1.f;

//FUTUR EDIT: Game should be a singleton and I should be able to ask for this
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;
const float ARENA_WIDTH = 500.f;
const float ARENA_HEIGHT = 500.f;


float g_currentElapsedSendToHostSeconds = 0.f;

//-----------------------------------------------------------------------------------------------
//Public Methods
//-----------------------------------------------------------------------------------------------
Client::Client()
	: m_connectionToHostID( 0 )
	, m_currentHostIPAddressAsString( STARTING_SERVER_IP_AS_STRING )
	, m_currentHostPort( STARTING_PORT )
	, m_sendPacketsToHostFrequency( SEND_TO_HOST_FREQUENCY )
	, m_localPlayerMovementVector( Vector2f::Zero() )
{
	m_localPlayer.id = LOCAL_PLAYER_ID;
}


//-----------------------------------------------------------------------------------------------
Client::~Client()
{

}


//-----------------------------------------------------------------------------------------------
void Client::StartUp()
{
	Network& theNetwork = Network::GetInstance();

	m_connectionToHostID = theNetwork.CreateUDPSocketFromIPAndPort( m_currentHostIPAddressAsString.c_str(), m_currentHostPort );
}


//-----------------------------------------------------------------------------------------------
void Client::ShutDown()
{
	Network& theNetwork = Network::GetInstance();

	theNetwork.ShutDown();
}


//-----------------------------------------------------------------------------------------------
void Client::UpdateLocalPlayerMovementVector( const Vector2f& movementVector )
{
	m_localPlayerMovementVector = movementVector;
}

//-----------------------------------------------------------------------------------------------
void Client::Update()
{
	Clock& appClock = Clock::GetMasterClock();

	ReceiveMessagesFromHostIfAny();
	UpdatePlayers();
	PotentiallySendUpdatePacketToServer();

	g_currentElapsedSendToHostSeconds += static_cast< float >( appClock.m_currentDeltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Client::Render() const
{
	RenderPlayers();
}


//-----------------------------------------------------------------------------------------------
void Client::ChangeHost( const std::string& ipAddressAsString, const std::string& portAsString )
{
	Network& theNetwork = Network::GetInstance();

	m_currentHostIPAddressAsString = ipAddressAsString;
	m_currentHostPort = u_short( atoi( portAsString.c_str() ) );

	m_connectionToHostID = theNetwork.CreateUDPSocketFromIPAndPort( m_currentHostIPAddressAsString.c_str(), m_currentHostPort );


	m_otherClientsPlayers.clear();

	m_localPlayer.Reset();
}


//-----------------------------------------------------------------------------------------------
ClientPlayer& Client::GetLocalPlayer()
{
	return m_localPlayer;
}


//-----------------------------------------------------------------------------------------------
std::vector< ClientPlayer >& Client::GetOtherClientsPlayers()
{
	return m_otherClientsPlayers;
}


//-----------------------------------------------------------------------------------------------
//Private Methods
//-----------------------------------------------------------------------------------------------
void Client::ReceiveMessagesFromHostIfAny()
{
	Network& theNetwork = Network::GetInstance();

	int bytesReceived = 0;
	CS6Packet receivedPacket;

	do 
	{
		bytesReceived = 0;
		ZeroMemory( &receivedPacket, sizeof( receivedPacket ) );

		bytesReceived = theNetwork.ReceiveUDPMessage( ( char* )&receivedPacket, sizeof( receivedPacket ), m_connectionToHostID );

		if( bytesReceived == sizeof( CS6Packet ) )
		{
			ProcessPacket( receivedPacket );
		}
	} 
	while ( bytesReceived > 0 );
}


//-----------------------------------------------------------------------------------------------
void Client::UpdatePlayers()
{
	m_localPlayer.SetCurrentVelocityAndPositionFromMovementVector( m_localPlayerMovementVector );

	auto iter = m_otherClientsPlayers.begin();

	for( ; iter != m_otherClientsPlayers.end(); ++ iter )
	{
		iter->SeekTarget();
	}
}


//-----------------------------------------------------------------------------------------------
void Client::PotentiallySendUpdatePacketToServer()
{
	if( g_currentElapsedSendToHostSeconds < m_sendPacketsToHostFrequency )
	{
		return;
	}

	CS6Packet updatePacketToSend = GetUpdatePacketFromPlayer( m_localPlayer );

	Network& theNetwork = Network::GetInstance();
	theNetwork.SendUDPMessage( ( char* )&updatePacketToSend, sizeof( updatePacketToSend ), m_connectionToHostID );

	g_currentElapsedSendToHostSeconds = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Client::RenderPlayers() const
{
	RenderPlayer( m_localPlayer );

	for( auto iter = m_otherClientsPlayers.begin(); iter != m_otherClientsPlayers.end(); ++iter )
	{
		RenderPlayer( *iter );
	}
}


//-----------------------------------------------------------------------------------------------
void Client::RenderPlayer( const ClientPlayer& playerToRender ) const
{
	Vector3f renderColorAsNormalizedVector;
	Vector2f minPosition;
	Vector2f maxPosition;
	float offset = 10.f;

	renderColorAsNormalizedVector = playerToRender.id.ToVector3fNormalizedAndFullyOpaque();

	minPosition = playerToRender.currentPos;
	maxPosition = Vector2f( minPosition.x + offset, minPosition.y + offset );

	OpenGLRenderer::UseShaderProgram( OpenGLRenderer::s_fixedFunctionPipelineShaderID );
	OpenGLRenderer::Uniform1i( OpenGLRenderer::s_fixedFunctionUseTexturesUniformLocation, 0 );

	OpenGLRenderer::s_rendererStack.Push( MatrixStack44f::MODEL_STACK );
	{
		OpenGLRenderer::Disable( GL_DEPTH_TEST );
		OpenGLRenderer::Disable( GL_CULL_FACE );

		const float ASPECT_RATIO = static_cast< float >( SCREEN_HEIGHT ) / SCREEN_WIDTH;

		OpenGLRenderer::s_rendererStack.ApplyOrtho( 0.f, ARENA_WIDTH, 0.f, ARENA_HEIGHT * ASPECT_RATIO, 0.f, 1.f ) ;
		OpenGLRenderer::LoadMatrix( OpenGLRenderer::s_rendererStack.GetMVWithProjection() );

		OpenGLRenderer::Render2DQuad( minPosition, maxPosition, renderColorAsNormalizedVector );
	}
	OpenGLRenderer::s_rendererStack.Pop( MatrixStack44f::MODEL_STACK );
}


//-----------------------------------------------------------------------------------------------
void Client::ProcessPacket( const CS6Packet& packet )
{
	OnReceivedUpdatePacket( packet );
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceivedUpdatePacket( const CS6Packet& updatePacket )
{
	PlayerID idFromPacket;
	memcpy( &idFromPacket, &updatePacket.playerColorAndID, sizeof( updatePacket.playerColorAndID ) );

	Vector2f newPosition( updatePacket.data.updated.xPosition, updatePacket.data.updated.yPosition );
	Vector2f newVelocity( updatePacket.data.updated.xVelocity, updatePacket.data.updated.yVelocity );

	float newOrientationDegrees( updatePacket.data.updated.yawDegrees );

	auto iter = m_otherClientsPlayers.begin();

	for( ; iter != m_otherClientsPlayers.end(); ++ iter )
	{
		if( iter->id == idFromPacket )
		{
			iter->desiredPos = newPosition;
			iter->desiredVelocity = newVelocity;
			iter->orientationAsDegrees = newOrientationDegrees;

			return;
		}
	}


	if( !( m_localPlayer.id == idFromPacket ) )
	{
		ClientPlayer newPlayer;

		newPlayer.currentPos = newPosition;
		newPlayer.currentVelocity = newVelocity;
		newPlayer.desiredPos = newPosition;
		newPlayer.desiredVelocity = newVelocity;

		newPlayer.orientationAsDegrees = newOrientationDegrees;
		newPlayer.id = idFromPacket;

		m_otherClientsPlayers.push_back( newPlayer );
	}
}


//-----------------------------------------------------------------------------------------------
CS6Packet Client::GetUpdatePacketFromPlayer( const ClientPlayer& player )
{
	CS6Packet updatePacket;

	ZeroMemory( &updatePacket, sizeof( updatePacket ) );

	updatePacket.packetType = TYPE_Update;

	updatePacket.data.updated.xPosition = player.currentPos.x;
	updatePacket.data.updated.yPosition = player.currentPos.y;

	updatePacket.data.updated.xVelocity = player.currentVelocity.x;
	updatePacket.data.updated.yVelocity = player.currentVelocity.y;

	updatePacket.data.updated.yawDegrees = player.orientationAsDegrees;

	memcpy( &updatePacket.playerColorAndID, &player.id, sizeof( player.id ) );

	return updatePacket;
}


//-----------------------------------------------------------------------------------------------
void Client::SetPlayerParameters( NamedProperties& parameters )
{
	std::string playerRedAsString;
	std::string playerGreenAsString;
	std::string playerBlueAsString;

	parameters.Get( "param1", playerRedAsString );
	parameters.Get( "param2", playerGreenAsString );
	parameters.Get( "param3", playerBlueAsString );

	std::set< ErrorType > errors, additionalErrors;

	errors = ValidateIsInt( playerRedAsString );
	additionalErrors = ValidateIsInt( playerGreenAsString );
	errors.insert( additionalErrors.begin(), additionalErrors.end() );

	additionalErrors = ValidateIsUChar( playerBlueAsString );
	errors.insert( additionalErrors.begin(), additionalErrors.end() );

	RECOVERABLE_ASSERTION( errors.empty(), "Command: playerColor did not receive the correct parameters.\nplayerColor expects three parameters of type int." );

	if( !errors.empty() )
		return;

	m_localPlayer.id.r = static_cast< uchar >( strtol( playerRedAsString.c_str(), 0, 10 ) );
	m_localPlayer.id.g = static_cast< uchar >( strtol( playerGreenAsString.c_str(), 0, 10 ) );
	m_localPlayer.id.b = static_cast< uchar >( strtol( playerBlueAsString.c_str(), 0, 10 ) );
}


//-----------------------------------------------------------------------------------------------
void Client::SetServerIpFromParameters( NamedProperties& parameters )
{
	std::string ipAddressAsString;

	parameters.Get( "param1", ipAddressAsString );

	FATAL_ASSERTION( ipAddressAsString != "", "Command: ip did not receive the correct parameters.\nip expects a non empty string." );

	m_currentHostIPAddressAsString = ipAddressAsString;
}


//-----------------------------------------------------------------------------------------------
void Client::SetServerPortFromParameters( NamedProperties& parameters )
{
	std::string portAsString;

	parameters.Get( "param1", portAsString );

	FATAL_ASSERTION( portAsString != "", "Command: port did not receive the correct parameters.\nport expects a non empty string." );

	m_currentHostPort = u_short( atoi( portAsString.c_str() ) );
}

