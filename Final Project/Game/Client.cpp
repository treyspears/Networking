#include "Client.hpp"

#include <sstream>
#include <algorithm>

#include "Engine/Rendering/ConsoleLog.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Utilities/ErrorWarningAssert.hpp"
#include "Engine/Utilities/CommandRegistry.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Utilities/XMLUtilities.hpp"
#include "Engine/Utilities/Time.hpp"
#include "Engine/Utilities/Clock.hpp"
#include "Engine/Primitives/Vector4.hpp"


#include "CameraBlueprint.hpp"

#define UNUSED( x ) ( void )( x )

//-----------------------------------------------------------------------------------------------
//const char* STARTING_SERVER_IP_AS_STRING = "129.119.247.159";		//Paul
//const char* STARTING_SERVER_IP_AS_STRING = "129.119.228.99";		//Jeff
//const char* STARTING_SERVER_IP_AS_STRING = "129.119.249.151";		//Vince
const char* STARTING_SERVER_IP_AS_STRING = "127.0.0.1";				//Self
const u_short STARTING_PORT = 5000;

const float SEND_TO_HOST_FREQUENCY = 0.05f;
const float SEND_TO_HOST_MAX_DELAY = 1.f;

//FUTUR EDIT: Game should be a singleton and I should be able to ask for this
const float ARENA_WIDTH = 700.f;
const float ARENA_HEIGHT = 700.f;

const float RESEND_RELIABLE_PACKETS_DELAY = 0.5f;

//-----------------------------------------------------------------------------------------------
//Public Methods
//-----------------------------------------------------------------------------------------------
Client::Client()
	: m_currentState( CLIENT_UNCONNECTED )
	, m_connectionToHostID( 0 )
	, m_currentHostIPAddressAsString( STARTING_SERVER_IP_AS_STRING )
	, m_currentHostPort( STARTING_PORT )
	, m_mostRecentlyProcessedUnreliablePacketNum( 0 )
	, m_mostRecentlyProcessedReliablePacketNum( 0 )
	, m_mostRecentUnreliablePacketSentNum( 0 )
	, m_mostRecentReliablePacketSentNum( 0 )
	, m_selectedRoomID( 0 )
	, m_currentRoomID( 0 )
	, m_currentJoinRoomRequestNum( 0 )
	, m_localPlayerMovementMagnitude( 0.f )
	, m_shouldFire( false )
	, m_localPlayer( nullptr )
	, m_localPlayerStatsToSendToServer( nullptr )
{
	ZeroMemory( &m_mostRecentResetInfo, sizeof( m_mostRecentResetInfo ) );
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

	m_localPlayer = new ClientPlayer();
	m_localPlayerStatsToSendToServer = new ClientPlayer();

	LoadCameraXMLDefinitions();

	m_cameraToRenderFrom = new Camera3D();
	Camera3D* firstPersonCamera = Camera3D::CreateCameraFromBlueprintAndAttachTo( "FPS Camera", m_localPlayer, 1.f, 1.f );

	m_cameras.push_back( firstPersonCamera );

	std::vector< Camera3D* >::iterator cameraIter = m_cameras.begin();
	for( ; cameraIter != m_cameras.end(); ++ cameraIter )
	{
		( *cameraIter )->Initialize();
	}
}


//-----------------------------------------------------------------------------------------------
void Client::ShutDown()
{
	Network& theNetwork = Network::GetInstance();

	theNetwork.ShutDown();
}


//-----------------------------------------------------------------------------------------------
void Client::Update()
{
	static float currentElapsedSendJoinLobbySeconds = SEND_TO_HOST_MAX_DELAY;
	static float currentElapsedSendKeepAlivePacketSeconds = 0.f;
	static float currentElapsedSendUpdatePacketSeconds = 0.f;
	static float currentElapsedSendVictoryPacketSeconds = SEND_TO_HOST_FREQUENCY;

	Clock& appClock = Clock::GetMasterClock();
	float deltaSeconds = static_cast< float >( appClock.m_currentDeltaSeconds );

	ReceiveMessagesFromHostIfAny();

	if( m_currentState == CLIENT_UNCONNECTED )
	{
		PotentiallySendJoinLobbyPacketToServer( currentElapsedSendJoinLobbySeconds, SEND_TO_HOST_MAX_DELAY );
		currentElapsedSendJoinLobbySeconds += deltaSeconds;
	}
	else if( m_currentState == CLIENT_IN_LOBBY )
	{
		PotentiallySendKeepAlivePacketToServer( currentElapsedSendKeepAlivePacketSeconds, SEND_TO_HOST_MAX_DELAY );
		currentElapsedSendKeepAlivePacketSeconds += deltaSeconds;
	}
	else if( m_currentState == CLIENT_AWAITING_RESET )
	{
		PotentiallySendKeepAlivePacketToServer( currentElapsedSendKeepAlivePacketSeconds, SEND_TO_HOST_MAX_DELAY );
		currentElapsedSendKeepAlivePacketSeconds += deltaSeconds;

		if( m_mostRecentResetInfo.type == TYPE_GameReset )
		{
			OnReceiveGameResetPacket( m_mostRecentResetInfo );
			ZeroMemory( &m_mostRecentResetInfo, sizeof( m_mostRecentResetInfo ) );
			m_currentState = CLIENT_IN_GAME;
		}
	}
	else if( m_currentState == CLIENT_IN_GAME )
	{
		if( m_shouldFire )
		{
			SendFirePacket();
			m_shouldFire = false;
		}
		UpdatePlayers();
		PotentiallySendUpdatePacketToServer( currentElapsedSendUpdatePacketSeconds, SEND_TO_HOST_FREQUENCY );
		currentElapsedSendUpdatePacketSeconds += deltaSeconds;
		currentElapsedSendVictoryPacketSeconds += deltaSeconds;
	}

	PotentiallyResendReliablePacketsThatHaventBeenAckedBack();
}


//-----------------------------------------------------------------------------------------------
void Client::RenderPlayers() const
{
	for( auto iter = m_otherClientsPlayers.begin(); iter != m_otherClientsPlayers.end(); ++iter )
	{
		RenderPlayer( *iter );
	}

	//RenderPlayer( *m_localPlayer );
}


//-----------------------------------------------------------------------------------------------
void Client::Render2DSprites() const
{
	if( m_currentState == CLIENT_IN_GAME )
	{
		RenderInGameStateInfo();
	}
	else if( m_currentState == CLIENT_IN_LOBBY )
	{
		RenderListedRooms();
	}
	else
	{
		RenderConnecting();
	}
}


//-----------------------------------------------------------------------------------------------
void Client::ChangeHost( const std::string& ipAddressAsString, const std::string& portAsString )
{
	Network& theNetwork = Network::GetInstance();

	m_currentHostIPAddressAsString = ipAddressAsString;
	m_currentHostPort = u_short( atoi( portAsString.c_str() ) );

	m_connectionToHostID = theNetwork.CreateUDPSocketFromIPAndPort( m_currentHostIPAddressAsString.c_str(), m_currentHostPort );
}


//-----------------------------------------------------------------------------------------------
void Client::SetGameTargetToPrevious()
{
	--m_selectedRoomID;

	if( m_selectedRoomID < 0 )
	{
		m_selectedRoomID = NUM_ROOMS - 1;
	}
}


//-----------------------------------------------------------------------------------------------
void Client::SetGameTargetToNext()
{
	++m_selectedRoomID;

	if( m_selectedRoomID >= NUM_ROOMS )
	{
		m_selectedRoomID = 0;
	}
}


//-----------------------------------------------------------------------------------------------
void Client::CreateGame()
{
	m_currentJoinRoomRequestNum = m_selectedRoomID + 1;
	FinalPacket gameStartAckPacket = GetCreateGamePacket( m_currentJoinRoomRequestNum );

	SendMessageToHost( gameStartAckPacket );
}


//-----------------------------------------------------------------------------------------------
void Client::JoinTargetedGame()
{
	m_currentJoinRoomRequestNum = m_selectedRoomID + 1;
	FinalPacket gameStartAckPacket = GetJoinRoomPacket( m_currentJoinRoomRequestNum );

	SendMessageToHost( gameStartAckPacket );
}


//-----------------------------------------------------------------------------------------------
void Client::UpdateLocalPlayerMovementMagnitude( float magnitude )
{
	m_localPlayerMovementMagnitude = magnitude;
}


//-----------------------------------------------------------------------------------------------
ClientPlayer* Client::GetLocalPlayer()
{
	return m_localPlayerStatsToSendToServer;
}


//-----------------------------------------------------------------------------------------------
std::vector< ClientPlayer >& Client::GetOtherClientsPlayers()
{
	return m_otherClientsPlayers;
}


//-----------------------------------------------------------------------------------------------
void Client::Shoot()
{
	m_shouldFire = true;
}


//-----------------------------------------------------------------------------------------------
const Camera3D& Client::GetCameraToRenderFrom() const
{
	return *m_cameraToRenderFrom;
}


//-----------------------------------------------------------------------------------------------
//Private Methods
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
void Client::LoadCameraXMLDefinitions()
{
	XMLDocumentParser& parser = XMLDocumentParser::GetInstance();
	parser.LoadDocument( "Data/CameraData/CameraDefinitions.xml" );

	pugi::xml_node root = parser.m_doc.child( "CameraDefinitions" );

	if( !root.empty() )
		CameraBlueprint::CreateAndStoreCameraBlueprints( root );
}


void Client::ReceiveMessagesFromHostIfAny()
{
	Network& theNetwork = Network::GetInstance();
	std::set< FinalPacket > packetQueueForThisFrame;

	int bytesReceived = 0;

	do 
	{
		FinalPacket receivedPacket;
		bytesReceived = 0;

		ZeroMemory( &receivedPacket, sizeof( receivedPacket ) );

		bytesReceived = theNetwork.ReceiveUDPMessage( ( char* )&receivedPacket, sizeof( receivedPacket ), m_connectionToHostID );
			
		if( bytesReceived == sizeof( FinalPacket ) )
		{
			packetQueueForThisFrame.insert( receivedPacket );
		}

	} 
	while ( bytesReceived > 0 );


	auto packetQueueIterator = packetQueueForThisFrame.begin();


	for( ; packetQueueIterator != packetQueueForThisFrame.end(); ++packetQueueIterator )
	{
		ProcessPacket( *packetQueueIterator );
	}	
}


//-----------------------------------------------------------------------------------------------
void Client::PotentiallySendJoinLobbyPacketToServer( float& elapsedSendTime, float sendToTime )
{
	if( elapsedSendTime < sendToTime )
	{
		return;
	}

	FinalPacket gameStartAckPacket = GetJoinRoomPacket( ROOM_Lobby );

	SendMessageToHost( gameStartAckPacket );

	elapsedSendTime = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Client::PotentiallySendKeepAlivePacketToServer( float& elapsedSendTime, float sendToTime )
{
	if( elapsedSendTime < sendToTime )
	{
		return;
	}

	FinalPacket keepAlivePacket;

	ZeroMemory( &keepAlivePacket, sizeof( keepAlivePacket ) );

	keepAlivePacket.type = TYPE_KeepAlive;

	SendMessageToHost( keepAlivePacket );

	elapsedSendTime = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Client::PotentiallySendUpdatePacketToServer( float& elapsedSendTime, float sendToTime )
{
	if( elapsedSendTime < sendToTime )
	{
		return;
	}

	FinalPacket updatePacket = GetUpdatePacket();

	SendMessageToHost( updatePacket );

	elapsedSendTime = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Client::ResendReliablePacket( FinalPacket& packetToSend )
{
	packetToSend.timestamp = Time::GetCurrentTimeInSeconds();

	Network& theNetwork = Network::GetInstance();
	theNetwork.SendUDPMessage( ( char* )&packetToSend, sizeof( packetToSend ), m_connectionToHostID );
}


//-----------------------------------------------------------------------------------------------
void Client::PotentiallyResendReliablePacketsThatHaventBeenAckedBack()
{
	for( auto iter = m_queueOfReliablePacketsSentToServer.begin(); iter != m_queueOfReliablePacketsSentToServer.end(); ++iter )
	{
		if( Time::GetCurrentTimeInSeconds() - iter->timestamp >= RESEND_RELIABLE_PACKETS_DELAY )
		{
			ResendReliablePacket( *iter );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Client::SendFirePacket()
{
	FinalPacket firePacket;

	firePacket.type = TYPE_Fire;
	firePacket.data.gunfire.instigatorID = m_localPlayer->GetID();

	SendMessageToHost( firePacket );
}


//-----------------------------------------------------------------------------------------------
void Client::SendMessageToHost( FinalPacket& packetToSend )
{
	packetToSend.timestamp = Time::GetCurrentTimeInSeconds();

	if( packetToSend.IsGuaranteed() )
	{
		packetToSend.number = m_mostRecentReliablePacketSentNum;
		++m_mostRecentReliablePacketSentNum;

		m_queueOfReliablePacketsSentToServer.push_back( packetToSend );
	}
	else
	{
		packetToSend.number = m_mostRecentUnreliablePacketSentNum;
		++m_mostRecentUnreliablePacketSentNum;
	}

	//if( packetToSend.type != TYPE_GameUpdate )
	//{
	//	
	//}

	Network& theNetwork = Network::GetInstance();
	theNetwork.SendUDPMessage( ( char* )&packetToSend, sizeof( packetToSend ), m_connectionToHostID );
}


//-----------------------------------------------------------------------------------------------
void Client::AckBackSuccessfulReliablePacketReceive( const FinalPacket& packet )
{
	FinalPacket ackBackPacket;

	ZeroMemory( &ackBackPacket, sizeof( ackBackPacket ) );

	ackBackPacket.type = TYPE_Ack;
	ackBackPacket.data.acknowledged.type = packet.type;
	if( packet.type == TYPE_Ack )
	{
		ackBackPacket.data.acknowledged.type = packet.data.acknowledged.type;
	}
	ackBackPacket.data.acknowledged.number = packet.number;

	SendMessageToHost( ackBackPacket );
}


//-----------------------------------------------------------------------------------------------
void Client::RenderListedRooms() const
{
	const float fontSize = 24.f;
	const Vector3f offsetVector( 0.f, fontSize, 0.f );
	const Vector3f initialPosition( 0.f, 550.f, 0.f ); //LAZY
	const Vector4f HIGHLIGHTED_COLOR( 1.f, 1.f, 0.f, 1.f );


	BitmapFont* font = BitmapFont::CreateOrGetFont( "Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml" );
	std::ostringstream outputStringStream;
	std::string stringToRender;

	Vector3f textPosition = initialPosition;

	outputStringStream.str( "" );
	outputStringStream << "Available Games:";
	stringToRender = outputStringStream.str();

	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition );
	textPosition -= offsetVector;

	for( int i = 0; i < NUM_ROOMS; ++ i  )
	{
		outputStringStream.str( "" );
		outputStringStream << "Game # " << i+1 << ": Num Players In Room - " << static_cast< int >( m_numPlayersInRoom[ i ] ); 
		stringToRender = outputStringStream.str();

		if( m_selectedRoomID == i )
		{
			OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition, HIGHLIGHTED_COLOR );
		}
		else
		{
			OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition );
		}
		
		textPosition -= offsetVector;
	}

	textPosition -= offsetVector;

	outputStringStream.str( "" );
	outputStringStream << "Press J To Join Game Highlighted In Yellow";
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition );

	textPosition -= offsetVector;

	outputStringStream.str( "" );
	outputStringStream << "Press H To Create And Start A New Game";
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition );
}


//-----------------------------------------------------------------------------------------------
void Client::RenderInGameStateInfo() const
{
	const float fontSize = 32.f;
	const Vector3f offsetVector( 0.f, -fontSize, 0.f );
	const Vector3f initialPosition( 0.f, 538.f, 0.f ); //LAZY

	const Vector4f roomColor( 0.f, 1.f, 0.f, 1.f );
	const Vector4f scoreColor( 1.f, 1.f, 0.f, 1.f );

	BitmapFont* font = BitmapFont::CreateOrGetFont( "Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml" );
	std::ostringstream outputStringStream;
	std::string stringToRender;

	Vector3f textPosition = initialPosition;

	outputStringStream.str( "" );
	outputStringStream << "Room# " << static_cast< int >( m_currentRoomID );
	stringToRender = outputStringStream.str();

	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition, roomColor, true );

	outputStringStream.str( "" );
	outputStringStream << "Score: " << static_cast< int >( m_localPlayer->score );
	stringToRender = outputStringStream.str();

	textPosition += offsetVector;

	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition, scoreColor, true );

}


//-----------------------------------------------------------------------------------------------
void Client::RenderConnecting() const
{
	const float fontSize = 24.f;
	const Vector3f initialPosition( 0.f, 550.f, 0.f ); //LAZY

	BitmapFont* font = BitmapFont::CreateOrGetFont( "Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml" );
	std::ostringstream outputStringStream;
	std::string stringToRender;

	Vector3f textPosition = initialPosition;

	outputStringStream.str( "" );
	outputStringStream << "Connecting...";
	stringToRender = outputStringStream.str();

	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition );
}


//-----------------------------------------------------------------------------------------------
void Client::RenderPlayer( const ClientPlayer& playerToRender ) const
{
	OpenGLRenderer::UseShaderProgram( OpenGLRenderer::s_fixedFunctionPipelineShaderID );
	OpenGLRenderer::Uniform1i( OpenGLRenderer::s_fixedFunctionUseTexturesUniformLocation, 0 );

	OpenGLRenderer::s_rendererStack.Push( MatrixStack44f::MODEL_STACK );
	{
		//OpenGLRenderer::Disable( GL_DEPTH_TEST );
		//OpenGLRenderer::Disable( GL_CULL_FACE );

		//const float ASPECT_RATIO = static_cast< float >( OpenGLRenderer::s_screenSize.y ) / OpenGLRenderer::s_screenSize.x;

		//OpenGLRenderer::s_rendererStack.ApplyOrtho( 0.f, ARENA_WIDTH, 0.f, ARENA_HEIGHT * ASPECT_RATIO, 0.f, 1.f ) ;
		//OpenGLRenderer::LoadMatrix( OpenGLRenderer::s_rendererStack.GetMVWithProjection() );

		playerToRender.RenderShip();
	}
	OpenGLRenderer::s_rendererStack.Pop( MatrixStack44f::MODEL_STACK );
}


//-----------------------------------------------------------------------------------------------
void Client::UpdatePlayers()
{
	if( m_currentState != CLIENT_IN_GAME )
	{
		return;
	}

	m_localPlayer->SeekTarget();

	m_localPlayerStatsToSendToServer->SetCurrentVelocityAndPositionFromMagnitude( m_localPlayerMovementMagnitude );

	UpdateCameras();

	auto iter = m_otherClientsPlayers.begin();

	for( ; iter != m_otherClientsPlayers.end(); ++ iter )
	{
		iter->SeekTarget();
	}
}


//-----------------------------------------------------------------------------------------------
void Client::UpdateCameras()
{
	std::vector< Camera3D* >::iterator cameraIter = m_cameras.begin();
	for( ; cameraIter != m_cameras.end(); ++ cameraIter )
	{
		( *cameraIter )->Update();
	}

	Vector3f blendedCameraPosition = Camera3D::GetBlendedPositionFromVectorOfCameras( m_cameras );
	EulerAnglesf blendedCameraOrientation = Camera3D::GetBlendedOrientationFromVectorOfCameras( m_cameras, blendedCameraPosition );

	m_cameraToRenderFrom->SetPose( blendedCameraPosition, blendedCameraOrientation );
	m_cameraToRenderFrom->SetViewDirectionVector();
}


//-----------------------------------------------------------------------------------------------
void Client::ProcessPacket( const FinalPacket& packet )
{
	//bool shouldProcessReliablePacketsInQueue = false;
	bool ignorePacket = false;

	OutputReceivedPacket( packet );

	if( packet.type == TYPE_GameReset && m_currentState == CLIENT_IN_LOBBY || m_currentState == CLIENT_AWAITING_RESET )
	{
		m_mostRecentResetInfo = packet;
	}

	if( packet.IsGuaranteed() )
	{
		//FUTURE EDIT: Ack to Server that client received reliable packet!

		AckBackSuccessfulReliablePacketReceive( packet );

		if( packet.number <= m_mostRecentlyProcessedReliablePacketNum )
		{
			ignorePacket = true;
		}
		else
		{
			m_mostRecentlyProcessedReliablePacketNum = packet.number;
		}
	}
	else
	{
		if( packet.number <= m_mostRecentlyProcessedUnreliablePacketNum )
		{
			ignorePacket = true;
		}
		else
		{
			m_mostRecentlyProcessedUnreliablePacketNum = packet.number;
		}	
	}

	if( !ignorePacket )
	{
		PacketType typeOfPacket = packet.type;

		if( typeOfPacket == TYPE_GameUpdate )
		{
			OnReceiveGameUpdatePacket( packet );
		}
		else if( typeOfPacket == TYPE_Ack )
		{
			OnReceiveAckPacket( packet );
		}
		else if( typeOfPacket == TYPE_LobbyUpdate )
		{
			OnReceiveLobbyUpdatePacket( packet );
		}
		else if( typeOfPacket == TYPE_Nack )
		{
			OnReceiveNackPacket( packet );
		}
		else if( typeOfPacket == TYPE_ReturnToLobby )
		{
			OnReceiveReturnToLobbyPacket( packet );
		}
		else if( typeOfPacket == TYPE_Fire )
		{
			OnReceiveFirePacket( packet );
		}
		else if( typeOfPacket == TYPE_Hit )
		{
			OnReceiveHitPacket( packet );
		}
		else if( typeOfPacket == TYPE_Respawn )
		{
			OnReceiveRespawnPacket( packet );
		}
	}

	//ProcessAnyQueuedReliablePackets();
}


//-----------------------------------------------------------------------------------------------
void Client::ProcessAnyQueuedReliablePackets()
{
	//if( !m_queueOfReliablePacketsToParse.empty() && m_queueOfReliablePacketsToParse.begin()->number <= m_nextExpectedReliablePacketNumToProcess )
	//{
	//	m_queueOfReliablePacketsToParse.erase( m_queueOfReliablePacketsToParse.begin() );
	//	ProcessPacket( *m_queueOfReliablePacketsToParse.begin() );
	//}
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveAckJoinRoomPacket( const FinalPacket& joinRoomAckPacket )
{
	UNUSED( joinRoomAckPacket );

	memset( &m_numPlayersInRoom, 0, sizeof( m_numPlayersInRoom ) );

	if( m_currentState == CLIENT_UNCONNECTED && m_currentJoinRoomRequestNum == 0 )
	{
		m_currentRoomID = ROOM_Lobby;
		m_currentState = CLIENT_IN_LOBBY;

	}
	else if( m_currentJoinRoomRequestNum > 0 )
	{
		m_currentRoomID = m_selectedRoomID + 1;

		if( m_mostRecentResetInfo.type == TYPE_GameReset )
		{
			OnReceiveGameResetPacket( m_mostRecentResetInfo );
			ZeroMemory( &m_mostRecentResetInfo, sizeof( m_mostRecentResetInfo ) );

			m_currentState = CLIENT_IN_GAME;
		}
		else
		{
			m_currentState = CLIENT_AWAITING_RESET;
		}
	}

	m_selectedRoomID = 0;
	m_currentJoinRoomRequestNum = 0;
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveAckCreateRoomPacket( const FinalPacket& createRoomAckPacket )
{
	UNUSED( createRoomAckPacket );

	if( m_currentState != CLIENT_IN_LOBBY )
	{
		return;
	}

	memset( &m_numPlayersInRoom, 0, sizeof( m_numPlayersInRoom ) );

	m_currentRoomID = m_selectedRoomID + 1;
	
	if( m_mostRecentResetInfo.type == TYPE_GameReset )
	{
		OnReceiveGameResetPacket( m_mostRecentResetInfo );
		ZeroMemory( &m_mostRecentResetInfo, sizeof( m_mostRecentResetInfo ) );

		m_currentState = CLIENT_IN_GAME;
	}
	else
	{
		m_currentState = CLIENT_AWAITING_RESET;
	}

	m_selectedRoomID = 0;
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveAckPacket( const FinalPacket& ackPacket )
{
	PacketType ackType = ackPacket.data.acknowledged.type;

	RemoveReliablePacketFromQueue( ackPacket );

	if( ackType == TYPE_JoinRoom )
	{
		OnReceiveAckJoinRoomPacket( ackPacket );
	}
	else if( ackType == TYPE_CreateRoom )
	{
		OnReceiveAckCreateRoomPacket( ackPacket );
	}
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveNackPacket( const FinalPacket& nackPacket )
{
	RemoveReliablePacketFromQueue( nackPacket );
}


//-----------------------------------------------------------------------------------------------
void Client::RemoveReliablePacketFromQueue( const FinalPacket& packetToRemove )
{
	for( int i = 0; i < static_cast< int >( m_queueOfReliablePacketsSentToServer.size() ); ++ i )
	{
		if( packetToRemove.data.acknowledged.number == m_queueOfReliablePacketsSentToServer[ i ].number )
		{
			FinalPacket temp = m_queueOfReliablePacketsSentToServer.back();

			if( i != static_cast< int >( m_queueOfReliablePacketsSentToServer.size() ) - 1 )
			{
				m_queueOfReliablePacketsSentToServer[ i ] = temp;
				--i;
			}

			m_queueOfReliablePacketsSentToServer.pop_back();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveReturnToLobbyPacket( const FinalPacket& returnToLobbyPacket )
{
	UNUSED( returnToLobbyPacket );

	if( m_currentState == CLIENT_UNCONNECTED )
	{
		return;
	}

	m_currentState = CLIENT_IN_LOBBY;

	memset( &m_numPlayersInRoom, 0, sizeof( m_numPlayersInRoom ) );

	m_selectedRoomID = 0;
	m_currentJoinRoomRequestNum = 0;
	m_currentRoomID = ROOM_Lobby;
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveLobbyUpdatePacket( const FinalPacket& lobbyUpdatePacket )
{
	if( m_currentState != CLIENT_IN_LOBBY )
	{
		return;
	}

	memcpy( &m_numPlayersInRoom, &lobbyUpdatePacket.data.updatedLobby.playersInRoomNumber, sizeof( m_numPlayersInRoom ) );
	//m_gameIDsFromLobby.insert( lobbyUpdatePacket.data.updatedLobby.playersInRoomNumber );
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveGameUpdatePacket( const FinalPacket& gameUpdatePacket )
{
	PlayerID idFromPacket = gameUpdatePacket.clientID;

	Vector3f newPosition( gameUpdatePacket.data.updatedGame.xPosition, gameUpdatePacket.data.updatedGame.yPosition, 0.f );
	Vector3f newVelocity( gameUpdatePacket.data.updatedGame.xVelocity, gameUpdatePacket.data.updatedGame.yVelocity, 0.f );

	float newOrientationDegrees = gameUpdatePacket.data.updatedGame.orientationDegrees;

	auto iter = m_otherClientsPlayers.begin();

	for( ; iter != m_otherClientsPlayers.end(); ++ iter )
	{
		if( iter->GetID() == idFromPacket )
		{
			iter->desiredPos = newPosition;
			iter->desiredVelocity = newVelocity;
			iter->orientationAsDegrees = newOrientationDegrees;
			iter->health = gameUpdatePacket.data.updatedGame.health;
			iter->score = gameUpdatePacket.data.updatedGame.score;

			return;
		}
	}


	if( m_localPlayer->GetID() != idFromPacket )
	{
		ClientPlayer newPlayer;

		newPlayer.SetID( idFromPacket );
		newPlayer.currentPos = newPosition;
		newPlayer.currentVelocity = newVelocity;
		newPlayer.desiredPos = newPosition;
		newPlayer.desiredVelocity = newVelocity;
		newPlayer.orientationAsDegrees = newOrientationDegrees;

		newPlayer.health = gameUpdatePacket.data.updatedGame.health;
		newPlayer.score = gameUpdatePacket.data.updatedGame.score;

		m_otherClientsPlayers.push_back( newPlayer );
	}
	else
	{
		m_localPlayer->desiredPos = newPosition;
		m_localPlayer->desiredVelocity = newVelocity;
		m_localPlayer->orientationAsDegrees = newOrientationDegrees;
		m_localPlayer->health = gameUpdatePacket.data.updatedGame.health;
		m_localPlayer->score = gameUpdatePacket.data.updatedGame.score;
	}
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveGameResetPacket( const FinalPacket& gameResetPacket )
{
	m_otherClientsPlayers.clear();
	m_localPlayer->Reset();
	m_localPlayerStatsToSendToServer->Reset();

	m_localPlayer->currentPos.x = gameResetPacket.data.reset.xPosition;
	m_localPlayer->currentPos.y = gameResetPacket.data.reset.yPosition;
	m_localPlayer->orientationAsDegrees = gameResetPacket.data.reset.orientationDegrees;
	m_localPlayer->SetID( gameResetPacket.data.reset.id );

	m_localPlayer->desiredPos = m_localPlayer->currentPos;


	*m_localPlayerStatsToSendToServer = *m_localPlayer;
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveFirePacket( const FinalPacket& firePacket )
{
	PlayerID instigatorID = firePacket.data.gunfire.instigatorID;
	ClientPlayer instigator;

	for( auto iter = m_otherClientsPlayers.begin(); iter != m_otherClientsPlayers.end(); ++iter )
	{
		if( iter->GetID() == instigatorID )
		{
			instigator = *iter;
			break;
		}
	}

	if( m_localPlayer->GetID() == instigatorID )
	{
		instigator = *m_localPlayer;
	}

	if( instigator.GetID() != BAD_PLAYER_ID )
	{
		//fire from their orientation
		Vector3f directionVector( cos( ConvertDegreesToRadians( instigator.orientationAsDegrees ) ), sin( ConvertDegreesToRadians( instigator.orientationAsDegrees ) ), 0.f );
		Vector3f offsetVector( 0.f, 0.f, -1.5f );
		OpenGLRenderer::RenderDebugLine( instigator.currentPos + offsetVector, instigator.currentPos + directionVector * 500.f + offsetVector, instigator.GetShipColor(), instigator.GetShipColor(), OpenGLRenderer::DEPTH_TEST, 2.5f );
	}
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveHitPacket( const FinalPacket& hitPacket )
{
	PlayerID hitPlayerID = hitPacket.data.hit.targetID;
	PlayerID instigatorID = hitPacket.data.hit.instigatorID;

	ClientPlayer hitPlayer;
	ClientPlayer instigator;

	for( int i = 0; i < static_cast< int >( m_otherClientsPlayers.size() ); ++i )
	{
		if( m_otherClientsPlayers[ i ].GetID() == instigatorID && instigator.GetID() == BAD_PLAYER_ID )
		{
			instigator = m_otherClientsPlayers[ i ];
		}

		if( m_otherClientsPlayers[ i ].GetID() == hitPlayerID && hitPlayer.GetID() == BAD_PLAYER_ID )
		{
			ClientPlayer temp = m_otherClientsPlayers.back();

			if( i != static_cast< int >( m_otherClientsPlayers.size() ) - 1 )
			{
				m_otherClientsPlayers[ i ] = temp;
				--i;
			}

			m_otherClientsPlayers.pop_back();
		}
	}


	if( m_localPlayer->GetID() == instigatorID )
	{
		instigator = *m_localPlayer;
	}


	if( instigator.GetID() != BAD_PLAYER_ID )
	{
		++instigator.score;
	}
}


//-----------------------------------------------------------------------------------------------
void Client::OnReceiveRespawnPacket( const FinalPacket& respawnPacket )
{
	m_localPlayer->Reset();

	m_localPlayer->currentPos.x = respawnPacket.data.respawn.xPosition;
	m_localPlayer->currentPos.y = respawnPacket.data.respawn.yPosition;
	m_localPlayer->desiredPos = m_localPlayer->currentPos;

	m_localPlayer->orientationAsDegrees = respawnPacket.data.respawn.orientationDegrees;

	*m_localPlayerStatsToSendToServer = *m_localPlayer;
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


//-----------------------------------------------------------------------------------------------
FinalPacket	Client::GetJoinRoomPacket( RoomID roomToConnectTo ) const
{
	FinalPacket joinRoomPacket;

	ZeroMemory( &joinRoomPacket, sizeof( joinRoomPacket ) );

	joinRoomPacket.type = TYPE_JoinRoom;
	joinRoomPacket.data.joining.room = roomToConnectTo;

	return joinRoomPacket;
}


//-----------------------------------------------------------------------------------------------
FinalPacket Client::GetCreateGamePacket( RoomID roomToConnectTo ) const
{
	FinalPacket createGamePacket;

	ZeroMemory( &createGamePacket, sizeof( createGamePacket ) );

	createGamePacket.type = TYPE_CreateRoom;
	createGamePacket.data.creating.room = roomToConnectTo;

	return createGamePacket;

}


//-----------------------------------------------------------------------------------------------
FinalPacket Client::GetUpdatePacket() const
{
	FinalPacket updatePacket;

	ZeroMemory( &updatePacket, sizeof( updatePacket ) );

	updatePacket.type = TYPE_GameUpdate;
	updatePacket.clientID = m_localPlayer->GetID();

	updatePacket.data.updatedGame.xPosition = m_localPlayerStatsToSendToServer->currentPos.x;
	updatePacket.data.updatedGame.yPosition = m_localPlayerStatsToSendToServer->currentPos.y;
	updatePacket.data.updatedGame.xVelocity = m_localPlayerStatsToSendToServer->currentVelocity.x;
	updatePacket.data.updatedGame.yVelocity = m_localPlayerStatsToSendToServer->currentVelocity.y;
	updatePacket.data.updatedGame.xAcceleration = m_localPlayerStatsToSendToServer->currentAcceleration.x;
	updatePacket.data.updatedGame.yAcceleration = m_localPlayerStatsToSendToServer->currentAcceleration.y;
	updatePacket.data.updatedGame.orientationDegrees = m_localPlayerStatsToSendToServer->orientationAsDegrees;
	updatePacket.data.updatedGame.health = m_localPlayer->health;
	updatePacket.data.updatedGame.score = m_localPlayer->score;
	
	return updatePacket;
}

//-----------------------------------------------------------------------------------------------
void Client::OutputReceivedPacket( const FinalPacket& packetToOutput ) const
{
	std::string			outputStringToRender;
	std::ostringstream  outputStringStream;

	outputStringStream << "\nPacket Received Type = " << GetReceivedPacketTypeAsString( packetToOutput.type );

	if( packetToOutput.type == TYPE_Ack )
	{
		outputStringStream << "\n\tAck Packet Type = " << GetReceivedPacketTypeAsString( packetToOutput.data.acknowledged.type );
	}
	else if( packetToOutput.type == TYPE_Nack )
	{
		outputStringStream << "\n\tNack Packet Type = " << GetReceivedPacketTypeAsString( packetToOutput.data.refused.type );
		outputStringStream << "\n\t\tError Code = " << GetReceivedNackErrorCodeAsString( packetToOutput.data.refused.errorCode );
	}

	if( packetToOutput.type != TYPE_LobbyUpdate && packetToOutput.type != TYPE_GameUpdate )
	{
		outputStringToRender = outputStringStream.str();
		OutputDebugStringA( outputStringToRender.c_str() );
	}
}


//-----------------------------------------------------------------------------------------------
std::string Client::GetReceivedPacketTypeAsString( const PacketType& typeOfPacket ) const
{
	std::string result;

	if( typeOfPacket == TYPE_None )
	{
		result = "TYPE_None";
	}
	else if( typeOfPacket == TYPE_Ack )
	{
		result = "TYPE_Ack";
	}
	else if( typeOfPacket == TYPE_Nack )
	{
		result = "TYPE_Nack";
	}
	else if( typeOfPacket == TYPE_KeepAlive )
	{
		result = "TYPE_KeepAlive";
	}
	else if( typeOfPacket == TYPE_CreateRoom )
	{
		result = "TYPE_CreateRoom";
	}
	else if( typeOfPacket == TYPE_JoinRoom )
	{
		result = "TYPE_JoinRoom";
	}
	else if( typeOfPacket == TYPE_LobbyUpdate )
	{
		result = "TYPE_LobbyUpdate";
	}
	else if( typeOfPacket == TYPE_GameUpdate )
	{
		result = "TYPE_GameUpdate";
	}
	else if( typeOfPacket == TYPE_GameReset)
	{
		result = "TYPE_GameReset";
	}
	else if( typeOfPacket == TYPE_Respawn )
	{
		result = "TYPE_Respawn";
	}
	else if( typeOfPacket == TYPE_Hit )
	{
		result = "TYPE_Hit";
	}
	else if( typeOfPacket == TYPE_Fire )
	{
		result = "TYPE_Fire";
	}
	else if( typeOfPacket == TYPE_ReturnToLobby )
	{
		result = "TYPE_ReturnToLobby";
	}
	else
	{
		result = "UNKNOWN TYPE";
	} 

	return result;
}


//-----------------------------------------------------------------------------------------------
std::string Client::GetReceivedNackErrorCodeAsString( const ErrorCode& codeError ) const
{
	std::string result;

	if( codeError == ERROR_None )
	{
		result = "ERROR_None";
	}
	else if( codeError == ERROR_RoomEmpty )
	{
		result = "ERROR_RoomEmpty";
	}
	else if( codeError == ERROR_RoomFull )
	{
		result = "ERROR_RoomFull";
	}
	else if( codeError == ERROR_BadRoomID )
	{
		result = "ERROR_BadRoomID";
	}
	else if( codeError == ERROR_Unknown )
	{
		result = "ERROR_Unknown";
	}
	else
	{
		result = "UNKNOWN ERROR CODE";
	}

	return result;
}

