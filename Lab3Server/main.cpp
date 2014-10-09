#define WIN32_LEAN_AND_MEAN
#define UNUSED(x) (void)(x);

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <map>
#include <time.h>

#include "Time.hpp"
#include "CS6Packet.hpp"

#pragma comment( lib, "WS2_32" ) // Link in the WS2_32.lib static library

//-----------------------------------------------------------------------------------------------

struct uniqueClient
{
	uniqueClient()
		: mostRecentInfo()
		, ID( nullptr )
		, ipAddressAsString( nullptr )
		, portAsString( nullptr )
		, timeSinceLastRecieve( 0.f )
		, sentMessagesCount( 0 )
	{

	}

	uniqueClient( const std::string& ipAddress, const std::string& port )
		: mostRecentInfo()
		, ID( port + ipAddress )
		, ipAddressAsString( ipAddress )
		, portAsString( port )
		, timeSinceLastRecieve( 0.f )
		, sentMessagesCount( 0 )
	{

	}

	uniqueClient( const std::string& ipAddress, const std::string& port, const CS6Packet& incomingPacket )
		: mostRecentInfo( incomingPacket )
		, ID( port + ipAddress )
		, ipAddressAsString( ipAddress )
		, portAsString( port )
		, timeSinceLastRecieve( 0.f )
		, sentMessagesCount( 0 )
	{

	}

	std::string ID;
	std::string ipAddressAsString;
	std::string portAsString;
	CS6Packet mostRecentInfo;
	float timeSinceLastRecieve;

	unsigned int sentMessagesCount;

	std::map< unsigned int, CS6Packet > m_packetsThatPotentiallyFailedToBeReceived;
};

const int BUFFER_LIMIT = 512;
const float MAX_SECONDS_OF_INACTIVITY = 5.f;
const float SEND_DELAY = 0.15f;
const float RAND_MAX_INVERSE = 1.f / (float)RAND_MAX;

const float RESEND_GUARANTEED_MAX_DELAY = 0.25f;

float currentSendElapsedTime = 0.f;

u_long BLOCKING = 0;
u_long NON_BLOCKING = 1;

float g_flagX = 0.f;
float g_flagY = 0.f;

//-----------------------------------------------------------------------------------------------
void ToUpperCaseString( std::string& outString );

void PromptUserForIPAndPortInfo( std::string& ipAddress_out, std::string& port_out );
bool InitializeWinSocket();
bool CreateAddressInfoForUDP( const std::string& ipAddress, const std::string& udpPort, sockaddr_in& udpAddressInfo );
bool CreateUDPSocket( SOCKET& newSocket, sockaddr_in& addressInfo );
bool BindSocket( SOCKET& socketToBind, sockaddr_in& addressInfo );

bool UDPSendMessageServer( const SOCKET& socketToSendThru, const sockaddr_in& sendToAddrInfo, const char* message, int messageLength );
bool UDPReceiveMessageServer( const SOCKET& socketToRecieveMessageThru );
bool RunUDPServer( SOCKET& udpServerSocket );

void AddClientToListOfConnectedClients( const sockaddr_in& senderInfo, const char* receiveBuffer );
CS6Packet CreatePacketFromBuffer( const char* buffer );
void UDPServerRemoveInactiveClients();
void UDPServerDisplayConnectedClients();
void UDPServerBroadcastPackets( SOCKET& udpSocket );
void UDPServerUpdateClientTimes();

void ProcessMessageFromClient( const char* buffer, const sockaddr_in& senderInfo );
void OnReceiveUpdate( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo );
void OnReceiveAck( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo );
void OnReceiveVictory( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo );
void OnAckAcknowledge( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo );
void OnAckReset( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo );
void OnAckVictory( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo );

void SetRandomFlagPosition();
void ResendAnyGuaranteedPacketsIfThresholdIsMet();
void CreateColorsForPlayers();
void GetColorForPlayer( int playerIndex, unsigned char *rgb );

std::string ipAddressAsString;
std::string portAsString;

sockaddr_in UDPServerAddrInfo;
std::vector< uniqueClient > g_relevantClients;

SOCKET UDPServerSocket = INVALID_SOCKET;

//-----------------------------------------------------------------------------------------------
int __cdecl main(int argc, char **argv)
{
	UNUSED( argc );
	UNUSED( argv );

	srand( ( unsigned int )time ( NULL ) );

	CreateColorsForPlayers();

	SetRandomFlagPosition();
	

	Time::InitializeTime();
	PromptUserForIPAndPortInfo( ipAddressAsString, portAsString );

	bool serverRunResult = true;

	

	serverRunResult = InitializeWinSocket();
	serverRunResult &= CreateAddressInfoForUDP( ipAddressAsString, portAsString, UDPServerAddrInfo );
	serverRunResult &= CreateUDPSocket( UDPServerSocket, UDPServerAddrInfo );
	ioctlsocket( UDPServerSocket, FIONBIO, &NON_BLOCKING );
	serverRunResult &= BindSocket( UDPServerSocket, UDPServerAddrInfo );

	if( serverRunResult )
	{
		RunUDPServer( UDPServerSocket );

		shutdown( UDPServerSocket, SD_BOTH );
		closesocket( UDPServerSocket );
		WSACleanup();
	}

	system("pause");
	if( !serverRunResult )
	{
		return 1;
	}
	return 0;
}


//-----------------------------------------------------------------------------------------------
void ToUpperCaseString( std::string& outString )
{
	for(int i = 0; i < (int)outString.length(); ++ i)
	{
		outString[i] = static_cast<char>(toupper(static_cast<int>(outString[i])));
	}
}


//-----------------------------------------------------------------------------------------------
void PromptUserForIPAndPortInfo( std::string& ipAddress_out, std::string& port_out )
{
	std::printf( "Enter IP Address for the server: " );
	std::cin >> ipAddress_out;

	std::printf( "IP Address of the server is %s\n\n", ipAddress_out.c_str() );

	std::printf( "Enter Port for server: " );
	std::cin >> port_out;

	std::printf( "Port of the server is %s\n\n", port_out.c_str() );
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

	UNUSED( ipAddress );

	ZeroMemory( &udpAddressInfo, sizeof( udpAddressInfo ) );
	udpAddressInfo.sin_family = AF_INET;
	udpAddressInfo.sin_addr.S_un.S_addr = INADDR_ANY;
	udpAddressInfo.sin_port = htons( u_short( atoi( udpPort.c_str() ) ) );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool CreateUDPSocket( SOCKET& newSocket, sockaddr_in& addressInfo )
{
	char optval;

	newSocket = socket( addressInfo.sin_family, SOCK_DGRAM, IPPROTO_UDP );
	setsockopt( newSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof( optval ) );

	if( newSocket == INVALID_SOCKET )
	{
		printf( "socket failed with error: %ld\n", WSAGetLastError() );

		shutdown( newSocket, SD_BOTH );
		closesocket( newSocket );
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

		shutdown( socketToBind, SD_BOTH );
		closesocket( socketToBind );
		WSACleanup();

		return false;
	}	

	return true;
}


//-----------------------------------------------------------------------------------------------
bool UDPSendMessageServer( const SOCKET& socketToSendThru, const sockaddr_in& sendToAddrInfo, const char* message, int messageLength )
{
	int sendResultAsInt;

	sendResultAsInt = sendto( socketToSendThru, message, messageLength, 0, (struct sockaddr *)&sendToAddrInfo, sizeof( sockaddr_in ) );

	if ( sendResultAsInt == SOCKET_ERROR ) 
	{
		printf( "send failed with error: %d\n", WSAGetLastError() );
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------------------------
bool RunUDPServer( SOCKET& udpServerSocket )
{
	bool shouldQuit = false;

	while( !shouldQuit )
	{
		shouldQuit &= UDPReceiveMessageServer( udpServerSocket );
		UDPServerRemoveInactiveClients();
		UDPServerDisplayConnectedClients();
		UDPServerBroadcastPackets( udpServerSocket );
		UDPServerUpdateClientTimes();
	}

	return shouldQuit;
}

//-----------------------------------------------------------------------------------------------
CS6Packet CreatePacketFromBuffer( const char* buffer )
{
	CS6Packet* result = nullptr;

	result = ( CS6Packet* )( buffer );

	return *result;
}

//-----------------------------------------------------------------------------------------------
void UDPServerRemoveInactiveClients()
{
	for( int i = 0; i < static_cast< int >( g_relevantClients.size() ); ++i )
	{
		if( g_relevantClients[ i ].timeSinceLastRecieve >= MAX_SECONDS_OF_INACTIVITY )
		{
			uniqueClient temp = g_relevantClients.back();

			if( i < static_cast< int >( g_relevantClients.size() ) - 1 )
			{
				g_relevantClients[ i ] = temp;
			}

			g_relevantClients.pop_back();
			--i;
		}
	}
}

//-----------------------------------------------------------------------------------------------
void UDPServerDisplayConnectedClients()
{
	system( "cls" );
	std::printf( "Connected Users: ");

	auto iter = g_relevantClients.begin();

	for( ; iter != g_relevantClients.end(); ++iter )
	{
		std::printf( "\nUser: ID -> %s, IP -> %s, Port -> %s", iter->ID.c_str(), iter->ipAddressAsString.c_str(), iter->portAsString.c_str() );
	}	
}

//-----------------------------------------------------------------------------------------------
void UDPServerBroadcastPackets( SOCKET& udpSocket )
{
	if( currentSendElapsedTime < SEND_DELAY )
	{
		return;
	}

	int sizeOfPacket = sizeof( CS6Packet );
	auto iter = g_relevantClients.begin();

	sockaddr_in clientInfo;
	int clientInfoLength = sizeof( clientInfo );

	for( ; iter != g_relevantClients.end(); ++iter )
	{
		memset( (char *) &clientInfo, 0, clientInfoLength );

		clientInfo.sin_family = AF_INET;
		clientInfo.sin_port = htons( u_short( atoi( iter->portAsString.c_str() ) ) );
		clientInfo.sin_addr.S_un.S_addr = inet_addr( iter->ipAddressAsString.c_str() );

		for( int i = 0; i < static_cast< int >( g_relevantClients.size() ); ++i )
		{
			++iter->sentMessagesCount;
			++iter->mostRecentInfo.packetNumber;

			char* message = ( char* )&g_relevantClients[ i ].mostRecentInfo;
			
			UDPSendMessageServer( udpSocket, clientInfo, message, sizeOfPacket );
		}
	}
	currentSendElapsedTime = 0.f;
}

//-----------------------------------------------------------------------------------------------
void UDPServerUpdateClientTimes()
{
	static double timeAtLastUpdate = Time::GetCurrentTimeInSeconds();
	double timeNow = Time::GetCurrentTimeInSeconds();

	float deltaSeconds = static_cast< float >( timeNow - timeAtLastUpdate );

	auto iter = g_relevantClients.begin();

	for( ; iter != g_relevantClients.end(); ++iter )
	{
		iter->timeSinceLastRecieve += deltaSeconds;
	}

	currentSendElapsedTime += deltaSeconds;

	timeAtLastUpdate = timeNow;
}

//-----------------------------------------------------------------------------------------------
bool UDPReceiveMessageServer( const SOCKET& socketToRecieveMessageThru )
{
	bool shouldQuitApp = false;

	char receiveBuffer[ BUFFER_LIMIT ];
	memset( receiveBuffer, '\0', BUFFER_LIMIT );	

	sockaddr_in clientAddrInfo;
	int addrInfoLength = sizeof( sockaddr_in );
	int bytesReceived = 0;

	do 
	{
		ZeroMemory( &clientAddrInfo, sizeof( clientAddrInfo ) );
		memset( receiveBuffer, '\0', BUFFER_LIMIT );
		bytesReceived = 0;

		bytesReceived = recvfrom( socketToRecieveMessageThru, receiveBuffer, BUFFER_LIMIT, 0, (struct sockaddr *)&clientAddrInfo, &addrInfoLength );

		if( bytesReceived == sizeof( CS6Packet ) )
		{
			ProcessMessageFromClient( receiveBuffer, clientAddrInfo );
		}
	} 
	while ( bytesReceived > 0 );

	return shouldQuitApp;
}

//-----------------------------------------------------------------------------------------------
void ProcessMessageFromClient( const char* buffer, const sockaddr_in& senderInfo )
{
	CS6Packet bufferAsPacket = CreatePacketFromBuffer( buffer );

	PacketType typeOfPacket = bufferAsPacket.packetType;

	if( typeOfPacket == TYPE_Update )
	{
		OnReceiveUpdate( bufferAsPacket, senderInfo );
	}
	else if( typeOfPacket == TYPE_Acknowledge )
	{
		OnReceiveAck( bufferAsPacket, senderInfo );
	}
	else if( typeOfPacket == TYPE_Victory )
	{
		OnReceiveVictory( bufferAsPacket, senderInfo );
	}
}


void OnReceiveUpdate( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo )
{
	char buffer[ 100 ];

	std::string senderIP = inet_ntoa( senderInfo.sin_addr );
	std::string senderPort = _itoa( static_cast< int >( ntohs( senderInfo.sin_port ) ), buffer, 10 );

	std::string senderID = senderPort + senderIP;

	auto iter = g_relevantClients.begin();

	for( ; iter != g_relevantClients.end(); ++iter )
	{
		if( iter->ID == senderID )
		{
			iter->timeSinceLastRecieve = 0.f;

			iter->mostRecentInfo.data.updated.xPosition = bufferAsPacket.data.updated.xPosition;
			iter->mostRecentInfo.data.updated.yPosition = bufferAsPacket.data.updated.yPosition;
			iter->mostRecentInfo.data.updated.xVelocity = bufferAsPacket.data.updated.xVelocity;
			iter->mostRecentInfo.data.updated.yVelocity = bufferAsPacket.data.updated.yVelocity;
			iter->mostRecentInfo.data.updated.yawDegrees = bufferAsPacket.data.updated.yawDegrees;

			break;
		}
	}	
}


void OnReceiveAck( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo )
{
	PacketType typeOfAck = bufferAsPacket.data.acknowledged.packetType;

	if( typeOfAck == TYPE_Acknowledge )
	{
		OnAckAcknowledge( bufferAsPacket, senderInfo );
	}
	else if( typeOfAck == TYPE_Reset )
	{
		OnAckReset( bufferAsPacket, senderInfo );
	}
	else if( typeOfAck == TYPE_Victory )
	{
		OnAckVictory( bufferAsPacket, senderInfo );
	}
}


void OnReceiveVictory( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo )
{
	SetRandomFlagPosition();

	char buffer[ 100 ];

	std::string senderIP = inet_ntoa( senderInfo.sin_addr );
	std::string senderPort = _itoa( static_cast< int >( ntohs( senderInfo.sin_port ) ), buffer, 10 );

	std::string senderID = senderPort + senderIP;

	auto iter = g_relevantClients.begin();

	for( ; iter != g_relevantClients.end(); ++iter )
	{
		if( iter->ID == senderID )
		{
			iter->timeSinceLastRecieve = 0.f;

			++iter->sentMessagesCount;
			++iter->mostRecentInfo.packetNumber;

			CS6Packet ackBack;

			ackBack.packetType = TYPE_Acknowledge;
			ackBack.packetNumber = iter->sentMessagesCount;
			ackBack.data.acknowledged.packetType = TYPE_Victory;

			UDPSendMessageServer( UDPServerSocket, senderInfo, ( char* )&ackBack, sizeof( CS6Packet ) );

			break;
		}
	}

	if( iter == g_relevantClients.end() )
	{
		return;
	}

	CS6Packet victoryPacket;
	victoryPacket.packetType = TYPE_Victory;
	victoryPacket.data.victorious.playerColorAndID[ 0 ] = iter->mostRecentInfo.playerColorAndID[ 0 ];
	victoryPacket.data.victorious.playerColorAndID[ 1 ] = iter->mostRecentInfo.playerColorAndID[ 1 ];
	victoryPacket.data.victorious.playerColorAndID[ 2 ] = iter->mostRecentInfo.playerColorAndID[ 2 ];

	iter = g_relevantClients.begin();

	for( ; iter != g_relevantClients.end(); ++iter )
	{
		++( *iter ).sentMessagesCount;
		++( *iter ).mostRecentInfo.packetNumber;

		sockaddr_in clientInfo;

		clientInfo.sin_family = AF_INET;
		clientInfo.sin_port = htons( u_short( atoi( iter->portAsString.c_str() ) ) );
		clientInfo.sin_addr.S_un.S_addr = inet_addr( iter->ipAddressAsString.c_str() );

		victoryPacket.packetNumber = iter->sentMessagesCount;
		iter->m_packetsThatPotentiallyFailedToBeReceived[ iter->sentMessagesCount ] = victoryPacket;

		UDPSendMessageServer( UDPServerSocket, clientInfo, ( char* )&victoryPacket, sizeof( CS6Packet ) );
	}
}


void OnAckAcknowledge( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo )
{
	char buffer[ 100 ];

	std::string senderIP = inet_ntoa( senderInfo.sin_addr );
	std::string senderPort = _itoa( static_cast< int >( ntohs( senderInfo.sin_port ) ), buffer, 10 );

	std::string senderID = senderPort + senderIP;


	auto iter = g_relevantClients.begin();

	for( ; iter != g_relevantClients.end(); ++iter )
	{
		if( iter->ID == senderID )
		{
			iter->timeSinceLastRecieve = 0.f;

			break;
		}
	}	

	if( iter == g_relevantClients.end() )
	{
		uniqueClient newClient( senderIP, senderPort, bufferAsPacket );
		g_relevantClients.push_back( newClient );

		++g_relevantClients.back().sentMessagesCount;
		++g_relevantClients.back().mostRecentInfo.packetNumber;

		CS6Packet resetPacket;

		resetPacket.packetType = TYPE_Reset;
		resetPacket.packetNumber = g_relevantClients.back().sentMessagesCount;

		resetPacket.playerColorAndID[ 0 ] = rand() % 255;
		resetPacket.playerColorAndID[ 1 ] = rand() % 255;
		resetPacket.playerColorAndID[ 2 ] = rand() % 255;

		resetPacket.data.reset.playerColorAndID[ 0 ] = resetPacket.playerColorAndID[ 0 ];
		resetPacket.data.reset.playerColorAndID[ 1 ] = resetPacket.playerColorAndID[ 1 ];
		resetPacket.data.reset.playerColorAndID[ 2 ] = resetPacket.playerColorAndID[ 2 ];

		//GetColorForPlayer( g_relevantClients.size() - 1, resetPacket.playerColorAndID );
		//memcpy( &resetPacket.data.reset.playerColorAndID, &resetPacket.playerColorAndID, sizeof( resetPacket.playerColorAndID ) );

		resetPacket.timestamp = Time::GetCurrentTimeInSeconds();
		

		float randomX = 500.f * static_cast<float>( rand() ) * RAND_MAX_INVERSE;
		float randomY = 500.f * static_cast<float>( rand() ) * RAND_MAX_INVERSE;

		resetPacket.data.reset.playerXPosition = randomX;
		resetPacket.data.reset.playerYPosition = randomY;
		resetPacket.data.reset.flagXPosition = g_flagX;
		resetPacket.data.reset.flagYPosition = g_flagY;

		g_relevantClients.back().m_packetsThatPotentiallyFailedToBeReceived[ g_relevantClients.back().sentMessagesCount ] = resetPacket;

		UDPSendMessageServer( UDPServerSocket, senderInfo, ( char* )&resetPacket, sizeof( resetPacket ) );
	}
}


void OnAckReset( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo )
{
	char buffer[ 100 ];

	std::string senderIP = inet_ntoa( senderInfo.sin_addr );
	std::string senderPort = _itoa( static_cast< int >( ntohs( senderInfo.sin_port ) ), buffer, 10 );

	std::string senderID = senderPort + senderIP;

	auto iter = g_relevantClients.begin();
	
	while( iter != g_relevantClients.end() )
	{
		if( iter->ID == senderID )
		{
			iter->timeSinceLastRecieve = 0.f;
			iter->m_packetsThatPotentiallyFailedToBeReceived.erase( bufferAsPacket.data.acknowledged.packetNumber );

			break;
		}

		++iter;
	}
}

//-----------------------------------------------------------------------------------------------
void OnAckVictory( const CS6Packet& bufferAsPacket, const sockaddr_in& senderInfo )
{
	char buffer[ 100 ];

	std::string senderIP = inet_ntoa( senderInfo.sin_addr );
	std::string senderPort = _itoa( static_cast< int >( ntohs( senderInfo.sin_port ) ), buffer, 10 );

	std::string senderID = senderPort + senderIP;

	auto iter = g_relevantClients.begin();

	int playerIndex = 0;

	while( iter != g_relevantClients.end() )
	{
		if( iter->ID == senderID )
		{
			iter->timeSinceLastRecieve = 0.f;
			iter->m_packetsThatPotentiallyFailedToBeReceived.erase( bufferAsPacket.data.acknowledged.packetNumber );

			++iter->sentMessagesCount;
			++iter->mostRecentInfo.packetNumber;

			CS6Packet resetPacket;

			resetPacket.packetType = TYPE_Reset;
			resetPacket.packetNumber = iter->sentMessagesCount;
			//GetColorForPlayer( playerIndex, resetPacket.playerColorAndID );
			//memcpy( &resetPacket.data.reset.playerColorAndID, &resetPacket.playerColorAndID, sizeof( resetPacket.playerColorAndID ) );

			resetPacket.playerColorAndID[ 0 ] = rand() % 255;
			resetPacket.playerColorAndID[ 1 ] = rand() % 255;
			resetPacket.playerColorAndID[ 2 ] = rand() % 255;

			resetPacket.data.reset.playerColorAndID[ 0 ] = resetPacket.playerColorAndID[ 0 ];
			resetPacket.data.reset.playerColorAndID[ 1 ] = resetPacket.playerColorAndID[ 1 ];
			resetPacket.data.reset.playerColorAndID[ 2 ] = resetPacket.playerColorAndID[ 2 ];

			resetPacket.timestamp = Time::GetCurrentTimeInSeconds();

			float randomX = 500.f * static_cast<float>( rand() ) * RAND_MAX_INVERSE;
			float randomY = 500.f * static_cast<float>( rand() ) * RAND_MAX_INVERSE;

			resetPacket.data.reset.playerXPosition = randomX;
			resetPacket.data.reset.playerYPosition = randomY;
			resetPacket.data.reset.flagXPosition = g_flagX;
			resetPacket.data.reset.flagYPosition = g_flagY;

			iter->m_packetsThatPotentiallyFailedToBeReceived[ iter->sentMessagesCount ] = resetPacket;

			UDPSendMessageServer( UDPServerSocket, senderInfo, ( char* )&resetPacket, sizeof( resetPacket ) );

			break;
		}

		++playerIndex;
		++iter;
	}
}


//-----------------------------------------------------------------------------------------------
void SetRandomFlagPosition()
{
	g_flagX = 500.f * static_cast<float>( rand() ) * RAND_MAX_INVERSE;
	g_flagY = 500.f * static_cast<float>( rand() ) * RAND_MAX_INVERSE;
}

//-----------------------------------------------------------------------------------------------
void ResendAnyGuaranteedPacketsIfThresholdIsMet()
{
	double currentTime = Time::GetCurrentTimeInSeconds();

	auto iter = g_relevantClients.begin();

	for( ; iter != g_relevantClients.end(); ++iter )
	{
		auto mapIter = ( *iter ).m_packetsThatPotentiallyFailedToBeReceived.begin();

		for( ; mapIter != ( *iter ).m_packetsThatPotentiallyFailedToBeReceived.end(); ++mapIter )
		{
			double timeDifference = currentTime - mapIter->second.timestamp;

			if( timeDifference > RESEND_GUARANTEED_MAX_DELAY )
			{
				mapIter->second.timestamp = currentTime;
				++mapIter->second.packetNumber;
				++iter->sentMessagesCount;
				
				sockaddr_in clientAddrInfo;

				clientAddrInfo.sin_family = AF_INET;
				clientAddrInfo.sin_addr.S_un.S_addr = inet_addr( iter->ipAddressAsString.c_str() );
				clientAddrInfo.sin_port = htons( u_short( atoi( iter->portAsString.c_str() ) ) );
				UDPSendMessageServer( UDPServerSocket, clientAddrInfo, ( char* )&mapIter->second, sizeof( CS6Packet ) );
			}
			
		}
	}
}


void CreateColorsForPlayers()
{
	unsigned char color[ 3 ];

	color[ 0 ] = 255;
	color[ 1 ] = 0;
	color[ 2 ] = 0;

	colors.push_back( color );

	color[ 0 ] = 0;
	color[ 1 ] = 255;
	color[ 2 ] = 0;

	colors.push_back( color );

	color[ 0 ] = 0;
	color[ 1 ] = 0;
	color[ 2 ] = 255;

	colors.push_back( color );


	color[ 0 ] = 255;
	color[ 1 ] = 255;
	color[ 2 ] = 0;

	colors.push_back( color );

	color[ 0 ] = 0;
	color[ 1 ] = 255;
	color[ 2 ] = 255;

	colors.push_back( color );

	color[ 0 ] = 255;
	color[ 1 ] = 0;
	color[ 2 ] = 255;

	colors.push_back( color );

	color[ 0 ] = 255;
	color[ 1 ] = 127;
	color[ 2 ] = 127;

	colors.push_back( color );

	color[ 0 ] = 127;
	color[ 1 ] = 255;
	color[ 2 ] = 127;

	colors.push_back( color );

	color[ 0 ] = 127;
	color[ 1 ] = 127;
	color[ 2 ] = 255;

	colors.push_back( color );

	color[ 0 ] = 127;
	color[ 1 ] = 127;
	color[ 2 ] = 127;

	colors.push_back( color );
}

void GetColorForPlayer( int playerIndex, unsigned char *rgb )
{

	//if( playerIndex < maxColors )
	//{
	//	memcpy( &rgb, &colors[ playerIndex ], sizeof( rgb ) );
	//}
	//else
	//{
		rgb[ 0 ] = rand() % 255;
		rgb[ 1 ] = rand() % 255;
		rgb[ 2 ] = rand() % 255;
	//}

}