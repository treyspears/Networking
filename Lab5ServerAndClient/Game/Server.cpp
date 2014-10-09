#include "Server.hpp"

#include <sstream>
#include <algorithm>

#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Utilities/Clock.hpp"
#include "Engine/Utilities/Time.hpp"
#include "Engine/Utilities/ErrorWarningAssert.hpp"

const float MAX_SECONDS_OF_INACTIVITY = 5.f;
const float RESEND_RELIABLE_PACKET_TIME = 2.f;
const float SEND_DELAY = 0.005f;

const char* IP_AS_STRING = "127.0.0.1";
const u_short STARTING_PORT = 5000;

const int ARENA_WIDTH = 500;
const int ARENA_HEIGHT = 500;
const unsigned int LOBBY_ID = 0;

float g_currentSendElapsedTime = 0.f;

GameID Server::s_currentMaxGameID = 0;

//const PlayerID BAD_IT_PLAYER = Color( Black );

//-----------------------------------------------------------------------------------------------
//Public Methods
//-----------------------------------------------------------------------------------------------
Server::Server()
	: m_listenConnectionID( 0 )
	, m_currentServerIPAddressAsString( IP_AS_STRING )
	, m_currentServerPort( STARTING_PORT )
	//, m_currentItPlayerID( BAD_IT_PLAYER )
{
}


//-----------------------------------------------------------------------------------------------
Server::~Server()
{

}


//-----------------------------------------------------------------------------------------------
void Server::StartUp()
{
	Network& theNetwork = Network::GetInstance();

	m_listenConnectionID = theNetwork.CreateUDPSocketFromIPAndPort( "0.0.0.0", m_currentServerPort );
	theNetwork.BindSocket( m_listenConnectionID );
}


//-----------------------------------------------------------------------------------------------
void Server::ShutDown()
{
	Network& theNetwork = Network::GetInstance();

	theNetwork.ShutDown();
}


//-----------------------------------------------------------------------------------------------
void Server::Update()
{
	Clock& appClock = Clock::GetMasterClock();

	ReceiveMessagesFromClientsIfAny();
	RemoveInactiveClients();
	ResendAnyGuaranteedPacketsThatHaveTimedOut();
	SendUpdatePacketsToAllClients();

	g_currentSendElapsedTime += static_cast< float >( appClock.m_currentDeltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Server::Render() const
{
	RenderConnectedClients();
}


//-----------------------------------------------------------------------------------------------
//Private Methods
//-----------------------------------------------------------------------------------------------
void Server::ReceiveMessagesFromClientsIfAny()
{
	Network& theNetwork = Network::GetInstance();

	int bytesReceived = 0;
	CS6Packet receivedPacket;

	do 
	{
		bytesReceived = 0;
		ZeroMemory( &receivedPacket, sizeof( receivedPacket ) );

		bytesReceived = theNetwork.ReceiveUDPMessage( ( char* )&receivedPacket, sizeof( receivedPacket ), m_listenConnectionID );

		if( bytesReceived == sizeof( CS6Packet ) )
		{
			ProcessPacket( receivedPacket );
		}
	} 
	while ( bytesReceived > 0 );
}


//-----------------------------------------------------------------------------------------------
void Server::ProcessPacket( const CS6Packet& packet )
{
	PacketType typeOfPacket = packet.packetType;

	if( typeOfPacket == TYPE_Update )
	{
		OnReceiveUpdatePacket( packet );
	}
	else if( typeOfPacket == TYPE_Reset )
	{
		//OnReceiveResetPacket( packet );
	}
	else if( typeOfPacket == TYPE_Acknowledge )
	{
		OnReceiveAckPacket( packet );
	}
	else if( typeOfPacket == TYPE_GameStart )
	{
		//OnReceiveGameStartPacket( packet );
	}
	else if( typeOfPacket == TYPE_Victory )
	{
		OnReceiveVictoryPacket( packet );
	}
	else if( typeOfPacket == TYPE_HostGame )
	{
		OnReceiveHostGamePacket( packet );
	}
	else if( typeOfPacket == TYPE_JoinGame )
	{
		OnReceiveJoinGamePacket( packet );
	}


	//this will need to be moved to on a received ack of type ack!
}


//-----------------------------------------------------------------------------------------------
void Server::OnReceiveUpdatePacket( const CS6Packet& packet )
{
	AddOrUpdateConnectedClient( packet );
}


//-----------------------------------------------------------------------------------------------
void Server::OnReceiveVictoryPacket( const CS6Packet& packet )
{
	//reset server state
	//reset flag position
	//send resets to all clients

	std::string ipAddressOfClient = "";
	std::string portAsString = "";

	Network& theNetwork = Network::GetInstance();

	ipAddressOfClient = theNetwork.GetIPAddressAsStringFromConnection( m_listenConnectionID );
	portAsString = theNetwork.GetPortAsStringFromConnection( m_listenConnectionID );

	if( ipAddressOfClient == "" || portAsString == "" )
	{
		return;
	}

	std::string idOfClient = portAsString + ipAddressOfClient;

	auto foundIter = m_connectedAndActiveClients.begin();

	for( ; foundIter != m_connectedAndActiveClients.end(); ++foundIter )
	{
		if( foundIter->second == nullptr )
		{
			continue;
		}

		if( foundIter->second->clientID == idOfClient )
		{
			break;
		}
	}

	if( foundIter == m_connectedAndActiveClients.end() )
	{
		return;
	}


	CS6Packet packetToSend;


	GameID roomID = foundIter->second->gameID;
	if( m_gamesAndTheirClients.find( roomID ) == m_gamesAndTheirClients.end() )
	{
		return;
	}

	while( m_gamesAndTheirClients[ roomID ].size() > 0 )
	{
		auto clientIter = m_connectedAndActiveClients.find( m_gamesAndTheirClients[ roomID ].back() );
		if( clientIter->second != nullptr )
		{
			RemoveClientFromRoom( roomID, *clientIter->second );
		}		
	}
}


//-----------------------------------------------------------------------------------------------
void Server::OnReceiveHostGamePacket( const CS6Packet& packet )
{
	std::string ipAddressOfClient = "";
	std::string portAsString = "";

	Network& theNetwork = Network::GetInstance();

	ipAddressOfClient = theNetwork.GetIPAddressAsStringFromConnection( m_listenConnectionID );
	portAsString = theNetwork.GetPortAsStringFromConnection( m_listenConnectionID );

	if( ipAddressOfClient == "" || portAsString == "" )
	{
		return;
	}

	std::string idOfClient = portAsString + ipAddressOfClient;

	auto foundIter = m_connectedAndActiveClients.begin();

	for( ; foundIter != m_connectedAndActiveClients.end(); ++foundIter )
	{
		if( foundIter->second == nullptr )
		{
			continue;
		}

		if( foundIter->second->clientID == idOfClient )
		{
			break;
		}
	}

	if( foundIter == m_connectedAndActiveClients.end() )
	{
		return;
	}

	if( foundIter->second != nullptr )
	{
		++Server::s_currentMaxGameID;

		foundIter->second->gameID = Server::s_currentMaxGameID;
		RemoveClientFromRoom( LOBBY_ID, *foundIter->second );
		m_gamesAndTheirClients[ Server::s_currentMaxGameID ].push_back( foundIter->second->connectionID );

		SendAGameStartPacketToNewClient( *foundIter->second );
	}
}


//-----------------------------------------------------------------------------------------------
void Server::OnReceiveJoinGamePacket( const CS6Packet& packet )
{

	std::string ipAddressOfClient = "";
	std::string portAsString = "";

	Network& theNetwork = Network::GetInstance();

	ipAddressOfClient = theNetwork.GetIPAddressAsStringFromConnection( m_listenConnectionID );
	portAsString = theNetwork.GetPortAsStringFromConnection( m_listenConnectionID );

	if( ipAddressOfClient == "" || portAsString == "" )
	{
		return;
	}

	std::string idOfClient = portAsString + ipAddressOfClient;

	auto foundIter = m_connectedAndActiveClients.begin();

	for( ; foundIter != m_connectedAndActiveClients.end(); ++foundIter )
	{
		if( foundIter->second == nullptr )
		{
			continue;
		}

		if( foundIter->second->clientID == idOfClient )
		{
			break;
		}
	}

	if( foundIter == m_connectedAndActiveClients.end() )
	{
		return;
	}

	if( foundIter->second != nullptr )
	{
		GameID gameIDToJoin = packet.data.joinGame.gameID;

		RemoveClientFromRoom( LOBBY_ID, *foundIter->second );

		if( m_gamesAndTheirClients.size() > gameIDToJoin )
		{
			m_gamesAndTheirClients[ gameIDToJoin ].push_back( foundIter->second->connectionID );
			foundIter->second->gameID = gameIDToJoin;

			SendAGameStartPacketToNewClient( *foundIter->second );
		}
	}
}

//-----------------------------------------------------------------------------------------------
void Server::OnReceiveAckPacket( const CS6Packet& packet )
{
	PacketType typeOfAck = packet.data.acknowledged.packetType;

	if( typeOfAck == TYPE_Acknowledge )
	{
		OnAckAcknowledge( packet );
	}
	else if( typeOfAck == TYPE_GameStart  || typeOfAck == TYPE_Reset || typeOfAck == TYPE_LobbyStart || typeOfAck == TYPE_HostGame || typeOfAck == TYPE_JoinGame )
	{
		OnAckReliablePacket( packet );
	}
}


//-----------------------------------------------------------------------------------------------
void Server::OnAckAcknowledge( const CS6Packet& packet )
{
	AddOrUpdateConnectedClient( packet );
}


//-----------------------------------------------------------------------------------------------
void Server::OnAckReliablePacket( const CS6Packet& packet )
{
	std::string ipAddressOfClient = "";
	std::string portAsString = "";

	Network& theNetwork = Network::GetInstance();

	ipAddressOfClient = theNetwork.GetIPAddressAsStringFromConnection( m_listenConnectionID );
	portAsString = theNetwork.GetPortAsStringFromConnection( m_listenConnectionID );

	if( ipAddressOfClient == "" || portAsString == "" )
	{
		return;
	}

	std::string idOfClient = portAsString + ipAddressOfClient;

	auto foundIter = m_connectedAndActiveClients.begin();

	for( ; foundIter != m_connectedAndActiveClients.end(); ++foundIter )
	{
		if( foundIter->second == nullptr )
		{
			continue;
		}

		if( foundIter->second->clientID == idOfClient )
		{
			break;
		}
	}

	if( foundIter != m_connectedAndActiveClients.end() )
	{
		foundIter->second->m_reliablePacketsAwaitingAckBack.erase( packet.data.acknowledged.packetNumber );
	}
}

//-----------------------------------------------------------------------------------------------
void Server::RemoveInactiveClients()
{
	Clock& appClock = Clock::GetMasterClock();
	float deltaSeconds = static_cast< float >( appClock.m_currentDeltaSeconds );

	for( int i = 0; i < static_cast< int >( m_connectedAndActiveClients.size() ); ++ i )
	{
		if( m_connectedAndActiveClients[ i ] == nullptr )
		{
			continue;
		}

		m_connectedAndActiveClients[ i ]->timeSinceLastReceivedMessage += deltaSeconds;

		if( m_connectedAndActiveClients[ i ]->timeSinceLastReceivedMessage >= MAX_SECONDS_OF_INACTIVITY )
		{
			RemoveClientFromRoom( m_connectedAndActiveClients[ i ]->gameID, *m_connectedAndActiveClients[ i ] );

			delete m_connectedAndActiveClients[ i ];
			m_connectedAndActiveClients[ i ] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Server::ResendAnyGuaranteedPacketsThatHaveTimedOut()
{
	Network& theNetwork = Network::GetInstance();

	float currentTime = Time::GetCurrentTimeInSeconds();

	for( auto iter = m_connectedAndActiveClients.begin(); iter != m_connectedAndActiveClients.end(); ++iter )
	{
		if( iter->second == nullptr )
		{
			continue;
		}

		for( auto guaranteedPacketIter = iter->second->m_reliablePacketsAwaitingAckBack.begin(); guaranteedPacketIter != iter->second->m_reliablePacketsAwaitingAckBack.end(); ++ guaranteedPacketIter )
		{
			if( currentTime - guaranteedPacketIter->second.timestamp > RESEND_RELIABLE_PACKET_TIME )
			{
				CS6Packet& packetToSend = guaranteedPacketIter->second;

				packetToSend.timestamp = Time::GetCurrentTimeInSeconds();

				theNetwork.SetIPAddressAsStringForConnection( m_listenConnectionID, iter->second->ipAddressAsString );
				theNetwork.SetPortAsStringForConnection( m_listenConnectionID,iter->second->portAsString );

				theNetwork.SendUDPMessage( ( char* )&packetToSend, sizeof( packetToSend ), m_listenConnectionID );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Server::SendUpdatePacketsToAllClients()
{
	CS6Packet updatePacket;

	updatePacket.packetType = TYPE_Update;

	for( auto iter = m_connectedAndActiveClients.begin(); iter != m_connectedAndActiveClients.end(); ++iter )
	{
		if( iter->second == nullptr )
		{
			continue;
		}
		updatePacket.data.updated = iter->second->mostRecentUpdateInfo;
		memcpy( &updatePacket.playerColorAndID, &iter->second->playerIDAsRGB, sizeof( updatePacket.playerColorAndID ) );

		if( iter->second->gameID != LOBBY_ID )
		{
			BroadCastMessageToAllClientsInRoom( iter->second->gameID, updatePacket, sizeof( updatePacket ) );
		}	
	}
}


//-----------------------------------------------------------------------------------------------
void Server::BroadCastMessageToAllClients( const CS6Packet& messageAsPacket, int messageLength )
{
	for( auto iter = m_connectedAndActiveClients.begin(); iter != m_connectedAndActiveClients.end(); ++iter )
	{
		SendMessageToClient( messageAsPacket, *( iter->second ) );
	}
}


//-----------------------------------------------------------------------------------------------
void Server::BroadCastMessageToAllClientsInRoom( GameID room, const CS6Packet& messageAsPacket, int messageLength )
{
	auto foundIter = m_gamesAndTheirClients.find( room );

	if( foundIter == m_gamesAndTheirClients.end() )
	{
		return;
	}

	for( auto iter = foundIter->second.begin(); iter != foundIter->second.end(); ++iter )
	{
		auto clientInMap = m_connectedAndActiveClients.find( *iter );

		if( clientInMap->second != nullptr )
		{
			SendMessageToClient( messageAsPacket, *( clientInMap->second ) );
		}	
	}
}

//-----------------------------------------------------------------------------------------------
void Server::SendAGameStartPacketToNewClient( ConnectedClient& clientToSendTo )
{
	CS6Packet packetToSend;

	ZeroMemory( &packetToSend, sizeof( packetToSend ) );

	packetToSend.packetType = TYPE_GameStart;
	packetToSend.data.gameStart.playerXPosition = rand() % ARENA_WIDTH;
	packetToSend.data.gameStart.playerYPosition = rand() % ARENA_HEIGHT;

	if( m_gamesAndTheirClients[ clientToSendTo.gameID ].size() <= 1 )
	{
		m_currentFlagPositions[ clientToSendTo.gameID ].x = rand() % ARENA_WIDTH;
		m_currentFlagPositions[ clientToSendTo.gameID ].y = rand() % ARENA_HEIGHT;
	}

	clientToSendTo.mostRecentUpdateInfo.xPosition = packetToSend.data.gameStart.playerXPosition;
	clientToSendTo.mostRecentUpdateInfo.yPosition = packetToSend.data.gameStart.playerYPosition;
	clientToSendTo.mostRecentUpdateInfo.xVelocity = 0.f;
	clientToSendTo.mostRecentUpdateInfo.yVelocity = 0.f;
	clientToSendTo.mostRecentUpdateInfo.yawDegrees = 0.f;

	packetToSend.data.gameStart.flagXPosition = m_currentFlagPositions[ clientToSendTo.gameID ].x;
	packetToSend.data.gameStart.flagYPosition = m_currentFlagPositions[ clientToSendTo.gameID ].y;
	//memcpy( &packetToSend.data.gameStart.itPlayerColorAndID, &m_currentItPlayerID, sizeof( packetToSend.data.gameStart.itPlayerColorAndID ) );
	memcpy( &packetToSend.data.gameStart.playerColorAndID, &clientToSendTo.playerIDAsRGB, sizeof( packetToSend.data.gameStart.playerColorAndID ) );

	SendMessageToClient( packetToSend, clientToSendTo );
};


//-----------------------------------------------------------------------------------------------
void Server::PutNewClientInLobbyAndSendListOfCurrentGames( ConnectedClient& clientToSendTo )
{
	CS6Packet lobbyPacket;

	m_gamesAndTheirClients[ LOBBY_ID ].push_back( clientToSendTo.connectionID );
	clientToSendTo.gameID = LOBBY_ID;

	lobbyPacket.packetType = TYPE_Acknowledge;
	lobbyPacket.data.acknowledged.packetType = TYPE_LobbyStart;

	SendMessageToClient( lobbyPacket, clientToSendTo );

	auto iter = m_gamesAndTheirClients.begin();

	for( ; iter != m_gamesAndTheirClients.end(); ++iter )
	{
		if( iter->first == LOBBY_ID )
		{
			continue;
		}

		ZeroMemory( &lobbyPacket, sizeof( lobbyPacket ) );

		lobbyPacket.packetType = TYPE_LobbyStart;
		lobbyPacket.data.lobbyStart.m_gameID = iter->first;

		SendMessageToClient( lobbyPacket, clientToSendTo );
	}
}


//-----------------------------------------------------------------------------------------------
void Server::SendMessageToClient( const CS6Packet& messageAsPacket, ConnectedClient& clientToSendTo )
{
	Network& theNetwork = Network::GetInstance();

	CS6Packet packetToSend = messageAsPacket;

	if( messageAsPacket.IsReliablePacket() )
	{
		++clientToSendTo.numReliableMessagesSent;
		packetToSend.packetNumber = clientToSendTo.numReliableMessagesSent;

		clientToSendTo.m_reliablePacketsAwaitingAckBack[ packetToSend.packetNumber ] = packetToSend; 
	}
	else
	{
		++clientToSendTo.numUnreliableMessagesSent;
		packetToSend.packetNumber = clientToSendTo.numUnreliableMessagesSent;
	}

	packetToSend.timestamp = Time::GetCurrentTimeInSeconds();

	theNetwork.SetIPAddressAsStringForConnection( m_listenConnectionID, clientToSendTo.ipAddressAsString );
	theNetwork.SetPortAsStringForConnection( m_listenConnectionID, clientToSendTo.portAsString );

	theNetwork.SendUDPMessage( ( char* )&packetToSend, sizeof( packetToSend ), m_listenConnectionID );
}


//-----------------------------------------------------------------------------------------------
void Server::AddOrUpdateConnectedClient( const CS6Packet& packet )
{
	std::string ipAddressOfClient = "";
	std::string portAsString = "";

	Network& theNetwork = Network::GetInstance();

	ipAddressOfClient = theNetwork.GetIPAddressAsStringFromConnection( m_listenConnectionID );
	portAsString = theNetwork.GetPortAsStringFromConnection( m_listenConnectionID );

	if( ipAddressOfClient == "" || portAsString == "" )
	{
		return;
	}

	std::string idOfClient = portAsString + ipAddressOfClient;

	auto iter = m_connectedAndActiveClients.begin();

	for( ; iter != m_connectedAndActiveClients.end(); ++iter )
	{
		if( iter->second == nullptr )
		{
			continue;
		}
		if( idOfClient == iter->second->clientID )
		{
			iter->second->timeSinceLastReceivedMessage = 0.f;

			if( packet.packetType == TYPE_Update )
			{
				iter->second->mostRecentUpdateInfo = packet.data.updated;
			}

			break;
		}
	}

	if( packet.packetType != TYPE_Acknowledge || packet.data.acknowledged.packetType != TYPE_Acknowledge )
	{
		return;
	}

	if( iter == m_connectedAndActiveClients.end() )
	{
		ConnectedClient* newConnectedClient = new ConnectedClient( ipAddressOfClient, portAsString );
		newConnectedClient->playerIDAsRGB = Color( uchar( m_connectedAndActiveClients.size() ) );

		++ConnectedClient::s_currentConnectedID;
		m_connectedAndActiveClients[ ConnectedClient::s_currentConnectedID ] = newConnectedClient;
		newConnectedClient->connectionID = ConnectedClient::s_currentConnectedID;

		PutNewClientInLobbyAndSendListOfCurrentGames( *newConnectedClient );

		//SendAGameStartPacketToNewClient( *newConnectedClient );
	}

	//Loop thru active clients and update the client whose id matches, else add new client to list

}


//-----------------------------------------------------------------------------------------------
void Server::RemoveClientFromRoom( GameID roomID, ConnectedClient& clientToSendRemove )
{
	if( m_gamesAndTheirClients.find( roomID ) == m_gamesAndTheirClients.end() )
	{
		return;
	}

	for( int i = 0; i < m_gamesAndTheirClients[ roomID ].size(); ++i )
	{
		if( m_gamesAndTheirClients[ roomID ][ i ] == clientToSendRemove.connectionID )
		{
			if( i == 0 && roomID != LOBBY_ID )
			{
				//remove all peoples and send them to lobby

				while( m_gamesAndTheirClients[ roomID ].size() > 1 )
				{
					auto foundIter = m_connectedAndActiveClients.find( m_gamesAndTheirClients[ roomID ].back() );

					if( foundIter != m_connectedAndActiveClients.end() )
					{
						PutNewClientInLobbyAndSendListOfCurrentGames( *foundIter->second );
					}

					m_gamesAndTheirClients[ roomID ].pop_back();
				}

				m_gamesAndTheirClients[ roomID ].pop_back();

				RemoveRoom( roomID );
				break;
			}
			else
			{
				//remove from room
				if( roomID != LOBBY_ID )
				{				
					auto foundIter = m_connectedAndActiveClients.find( m_gamesAndTheirClients[ roomID ][ i ] );

					if( foundIter != m_connectedAndActiveClients.end() )
					{
						PutNewClientInLobbyAndSendListOfCurrentGames( *foundIter->second );
					}
				}

				if( i < static_cast< int >( m_connectedAndActiveClients.size() ) -1 )
				{
					m_gamesAndTheirClients[ roomID ][ i ] = m_gamesAndTheirClients[ roomID ].back();
					--i;
				}

				m_gamesAndTheirClients[ roomID ].pop_back();
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Server::RemoveRoom( GameID roomID )
{
	m_gamesAndTheirClients.erase( roomID );

	CS6Packet lobbyPacket;

	for( auto iter = m_connectedAndActiveClients.begin(); iter != m_connectedAndActiveClients.end(); ++iter ) 
	{
		if( iter->second == nullptr )
		{
			continue;
		}

		ZeroMemory( &lobbyPacket, sizeof( lobbyPacket ) );
		lobbyPacket.packetType = TYPE_Acknowledge;
		lobbyPacket.data.acknowledged.packetType = TYPE_LobbyStart;

		SendMessageToClient( lobbyPacket, *( iter->second ) );

		auto clientIter = m_gamesAndTheirClients.begin();

		for( ; clientIter != m_gamesAndTheirClients.end(); ++clientIter )
		{
			if( clientIter->first == LOBBY_ID )
			{
				continue;
			}

			lobbyPacket.packetType = TYPE_LobbyStart;
			lobbyPacket.data.lobbyStart.m_gameID = clientIter->first;

			SendMessageToClient( lobbyPacket, *( iter->second ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Server::RenderConnectedClients() const
{
	//do this later

	const float fontSize = 24.f;
	const Vector3f offsetVector( 0.f, fontSize, 0.f );
	const Vector3f initialPosition( 0.f, 1000.f, 0.f ); //LAZY

	BitmapFont* font = BitmapFont::CreateOrGetFont( "Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml" );
	std::ostringstream outputStringStream;
	std::string stringToRender;

	Vector3f textPosition = initialPosition;

	outputStringStream.str( "" );
	outputStringStream << "Connected Clients:";
	stringToRender = outputStringStream.str();

	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition );
	textPosition -= offsetVector;

	auto iter = m_connectedAndActiveClients.begin();

	int actualNumClients = 0;

	for( ; iter != m_connectedAndActiveClients.end(); ++iter )
	{
		if( iter->second == nullptr )
		{
			continue;
		}

		++actualNumClients;
		outputStringStream.str( "" );
		outputStringStream << "Client Address: " << iter->second->ipAddressAsString << ", Port: " << iter->second->portAsString;
		stringToRender = outputStringStream.str();

		OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition );
		textPosition -= offsetVector;
	}

	if( m_connectedAndActiveClients.size() == 0 || actualNumClients == 0 )
	{
		outputStringStream.str( "" );
		outputStringStream << "None.";
		stringToRender = outputStringStream.str();

		OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition );
	}
}


//-----------------------------------------------------------------------------------------------
void Server::SetServerIpFromParameters( NamedProperties& parameters )
{
	std::string ipAddressAsString;

	parameters.Get( "param1", ipAddressAsString );

	FATAL_ASSERTION( ipAddressAsString != "", "Command: ip did not receive the correct parameters.\nip expects a non empty string." );

	m_currentServerIPAddressAsString = ipAddressAsString;
}


//-----------------------------------------------------------------------------------------------
void Server::SetPortToBindToFromParameters( NamedProperties& parameters )
{
	std::string portAsString;

	parameters.Get( "param1", portAsString );

	FATAL_ASSERTION( portAsString != "", "Command: port did not receive the correct parameters.\nport expects a non empty string." );

	m_currentServerPort = u_short( atoi( portAsString.c_str() ) );
}