#include "Client.hpp"

#include <sstream>
#include <algorithm>

#include "Engine/Rendering/ConsoleLog.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Utilities/ErrorWarningAssert.hpp"
#include "Engine/Utilities/CommandRegistry.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Utilities/Time.hpp"
#include "Engine/Utilities/Clock.hpp"
#include "Engine/Primitives/Vector4.hpp"

#define UNUSED( x ) ( void )( x )

//-----------------------------------------------------------------------------------------------
const char* STARTING_SERVER_IP_AS_STRING = "129.119.247.159"; //Paul
//const char* STARTING_SERVER_IP_AS_STRING = "129.119.228.99"; //Jeff
const u_short STARTING_PORT = 5000;

const float SEND_TO_HOST_FREQUENCY = 0.05f;
const float SEND_TO_HOST_MAX_DELAY = 1.f;

//FUTUR EDIT: Game should be a singleton and I should be able to ask for this
const float ARENA_WIDTH = 500.f;
const float ARENA_HEIGHT = 500.f;

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
	, m_nextExpectedReliablePacketNumToProcess( 1 )
	, m_mostRecentUnreliablePacketSentNum( 0 )
	, m_mostRecentReliablePacketSentNum( 0 )
	, m_selectedRoomID( 0 )
	, m_currentRoomID( 0 )
	, m_currentJoinRoomRequestNum( 0 )
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
	else if( m_currentState == CLIENT_IN_GAME )
	{
		if( m_mostRecentResetInfo.type == TYPE_GameReset )
		{
			OnReceiveGameResetPacket( m_mostRecentResetInfo );
			ZeroMemory( &m_mostRecentResetInfo, sizeof( m_mostRecentResetInfo ) );
		}

		PotentiallySendKeepAlivePacketToServer( currentElapsedSendKeepAlivePacketSeconds, SEND_TO_HOST_MAX_DELAY );
		currentElapsedSendUpdatePacketSeconds += deltaSeconds;
		currentElapsedSendVictoryPacketSeconds += deltaSeconds;
		currentElapsedSendKeepAlivePacketSeconds += deltaSeconds;
	}

	PotentiallyResendReliablePacketsThatHaventBeenAckedBack();
}


//-----------------------------------------------------------------------------------------------
void Client::Render() const
{
	if( m_currentState == CLIENT_IN_GAME )
	{
		RenderImInARoom();
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
//Private Methods
//-----------------------------------------------------------------------------------------------
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
	const Vector3f initialPosition( 0.f, 1000.f, 0.f ); //LAZY
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
void Client::RenderImInARoom() const
{
	const float fontSize = 24.f;
	const Vector3f initialPosition( 0.f, 1000.f, 0.f ); //LAZY

	BitmapFont* font = BitmapFont::CreateOrGetFont( "Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml" );
	std::ostringstream outputStringStream;
	std::string stringToRender;

	Vector3f textPosition = initialPosition;

	outputStringStream.str( "" );
	outputStringStream << "I'm in a room! Room#" << static_cast< int >( m_currentRoomID );
	stringToRender = outputStringStream.str();

	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition );
}


//-----------------------------------------------------------------------------------------------
void Client::RenderConnecting() const
{
	const float fontSize = 24.f;
	const Vector3f initialPosition( 0.f, 1000.f, 0.f ); //LAZY

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
void Client::ProcessPacket( const FinalPacket& packet )
{
	//bool shouldProcessReliablePacketsInQueue = false;
	bool ignorePacket = false;

	OutputReceivedPacket( packet );

	if( packet.type == TYPE_GameReset )
	{
		m_mostRecentResetInfo = packet;
	}

	if( packet.IsGuaranteed() )
	{
		//FUTURE EDIT: Ack to Server that client received reliable packet!

		AckBackSuccessfulReliablePacketReceive( packet );

		if( packet.number > m_nextExpectedReliablePacketNumToProcess )
		{
			m_queueOfReliablePacketsToParse.insert( packet );
			ignorePacket = true;
		}
		else
		{
			++m_nextExpectedReliablePacketNumToProcess;
		}
	}
	else
	{
		if( packet.number < m_mostRecentlyProcessedUnreliablePacketNum )
		{
			ignorePacket = true;
		}

		m_mostRecentlyProcessedUnreliablePacketNum = packet.number;
	}

	if( !ignorePacket )
	{
		PacketType typeOfPacket = packet.type;

		if( typeOfPacket == TYPE_Ack )
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
	}

	ProcessAnyQueuedReliablePackets();
}


//-----------------------------------------------------------------------------------------------
void Client::ProcessAnyQueuedReliablePackets()
{
	if( !m_queueOfReliablePacketsToParse.empty() && m_queueOfReliablePacketsToParse.begin()->number <= m_nextExpectedReliablePacketNumToProcess )
	{
		m_queueOfReliablePacketsToParse.erase( m_queueOfReliablePacketsToParse.begin() );
		ProcessPacket( *m_queueOfReliablePacketsToParse.begin() );
	}
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
		m_currentState = CLIENT_IN_GAME;
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
	m_currentState = CLIENT_IN_GAME;

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


	OutputDebugStringA( "\nI SHOULD BE IN LOBBY BUT MY STATE IS APPARENTLY UNCONNECTED");

	if( m_currentState == CLIENT_UNCONNECTED )
	{
		return;
	}

	m_currentState = CLIENT_IN_LOBBY;
	OutputDebugStringA( "\nI SHOULD BE IN LOBBY");

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
void Client::OnReceiveGameResetPacket( const FinalPacket& gameResetPacket )
{
	OutputDebugStringA( "\nSETTING PLAYER POSE FROM RESET PACKET" );
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