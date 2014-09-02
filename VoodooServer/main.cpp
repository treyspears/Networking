#define WIN32_LEAN_AND_MEAN
#define UNUSED(x) (void)(x);

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>

#include "Time.hpp"

#pragma comment( lib, "WS2_32" ) // Link in the WS2_32.lib static library

//-----------------------------------------------------------------------------------------------
const int BUFFER_LIMIT = 512;
const float MAX_SECONDS_OF_INACTIVITY = 5.f;
const float SEND_DELAY = 0.1f;

float currentSendElapsedTime = 0.f;

struct packet
{
	packet()
		: id( -1 )
		, r( 0 )
		, g( 0 )
		, b( 0 )
		, x( -1.f )
		, y( -1.f )
	{

	}

	char id;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	float x;
	float y;
};

struct uniqueClient
{
	uniqueClient()
		: mostRecentInfo()
		, ID( nullptr )
		, ipAddressAsString( nullptr )
		, portAsString( nullptr )
		, timeSinceLastRecieve( 0.f )
	{

	}

	uniqueClient( const std::string& ipAddress, const std::string& port )
		: mostRecentInfo()
		, ID( port + ipAddress )
		, ipAddressAsString( ipAddress )
		, portAsString( port )
		, timeSinceLastRecieve( 0.f )
	{

	}

	uniqueClient( const std::string& ipAddress, const std::string& port, const packet& incomingPacket )
		: mostRecentInfo( incomingPacket )
		, ID( port + ipAddress )
		, ipAddressAsString( ipAddress )
		, portAsString( port )
		, timeSinceLastRecieve( 0.f )
	{

	}

	std::string ID;
	std::string ipAddressAsString;
	std::string portAsString;
	packet mostRecentInfo;
	float timeSinceLastRecieve;
};

//-----------------------------------------------------------------------------------------------
void ToUpperCaseString( std::string& outString );

void PromptUserForIPAndPortInfo( std::string& ipAddress_out, std::string& port_out );
bool InitializeWinSocket();
bool CreateAddressInfoForUDP( const std::string& ipAddress, const std::string& udpPort, sockaddr_in& udpAddressInfo );
bool CreateUDPSocket( sockaddr_in& addressInfo, SOCKET& newSocket );
bool BindSocket( SOCKET& socketToBind, addrinfo*& addressInfo );
bool UDPSendMessage( const SOCKET& socketToSendThru, const char* message, int messageLength );
bool UDPServerRecieveMessage( const SOCKET& serverSocket );

bool SetupUDPServer( const std::string& ipaddress, const std::string& udpPort, sockaddr_in& serverAddressInfo_out, SOCKET& udpServerSocket_out );
bool RunUDPServer( SOCKET& udpServerSocket );
void AddClientToListOfConnectedClients( const sockaddr_in& senderInfo, const char* receiveBuffer );
packet CreatePacketFromBuffer( const char* buffer );
void UDPServerRemoveInactiveClients();
void UDPServerDisplayConnectedClients();
void UDPServerBroadcastPackets( SOCKET& udpSocket );
void UDPServerUpdateClientTimes();

sockaddr_in UDPServerAddressInfo;
std::string IPAddressAsStringForServer;
std::string portAsStringForServer;

u_long BLOCKING = 0;
u_long NON_BLOCKING = 1;

std::vector< uniqueClient > g_relevantClients;

//-----------------------------------------------------------------------------------------------
int __cdecl main(int argc, char **argv)
{
	UNUSED( argc );
	UNUSED( argv );

	Time::InitializeTime();
	PromptUserForIPAndPortInfo( IPAddressAsStringForServer, portAsStringForServer );

	bool serverRunResult = true;

	SOCKET UDPServerSocket = INVALID_SOCKET;

	serverRunResult = SetupUDPServer( IPAddressAsStringForServer, portAsStringForServer, UDPServerAddressInfo, UDPServerSocket );

	ioctlsocket( UDPServerSocket, FIONBIO, &NON_BLOCKING );


	if( serverRunResult )
	{
		serverRunResult = RunUDPServer( UDPServerSocket );
	}

	closesocket( UDPServerSocket );

	WSACleanup();

	system( "pause" );
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
	std::printf( "\nEnter IP Address for the server: " );
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
bool UDPSendMessage( const SOCKET& socketToSendThru, const char* message, int messageLength, sockaddr_in& udpAddressInfo )
{
	int sendResultAsInt;

	sendResultAsInt = sendto( socketToSendThru, message, messageLength, 0, (struct sockaddr *)&udpAddressInfo, sizeof( udpAddressInfo ) );

	if ( sendResultAsInt == SOCKET_ERROR ) 
	{
		printf( "send failed with error: %d\n", WSAGetLastError() );
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------------------------
bool UDPServerRecieveMessage( const SOCKET& serverSocket )
{
	char receiveBuffer[ BUFFER_LIMIT ];
	memset( receiveBuffer, '\0', BUFFER_LIMIT );

	int bytesReceived = 0;

	sockaddr_in senderInfo;
	int senderInfoLength = sizeof( senderInfo );

	bytesReceived = recvfrom( serverSocket, receiveBuffer, BUFFER_LIMIT, 0, (struct sockaddr *)&senderInfo, &senderInfoLength );

	if( bytesReceived == sizeof( packet ) )
	{
		AddClientToListOfConnectedClients( senderInfo, receiveBuffer );
		return true;
	}
	else if( bytesReceived > 0 )
	{
		std::printf( "Server received message: %s\n\n", receiveBuffer );
		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------------------------
bool SetupUDPServer( const std::string& ipaddress, const std::string& udpPort, sockaddr_in& serverAddressInfo_out, SOCKET& udpServerSocket_out )
{
	bool wasAllInitializationSuccessful = true;

	wasAllInitializationSuccessful &= InitializeWinSocket();
	wasAllInitializationSuccessful &= CreateAddressInfoForUDP( ipaddress, udpPort, serverAddressInfo_out );

	if( !wasAllInitializationSuccessful )
	{
		return false;
	}

	wasAllInitializationSuccessful &= CreateUDPSocket( serverAddressInfo_out, udpServerSocket_out );

	if( !wasAllInitializationSuccessful )
	{
		return false;
	}

	wasAllInitializationSuccessful &= BindSocket( udpServerSocket_out, serverAddressInfo_out );

	if( !wasAllInitializationSuccessful )
	{
		return false;
	}

	return wasAllInitializationSuccessful;
}


//-----------------------------------------------------------------------------------------------
bool RunUDPServer( SOCKET& udpServerSocket )
{
	bool runUntilAppClosed = true;

	while( runUntilAppClosed )
	{
		UDPServerRecieveMessage( udpServerSocket );
		UDPServerRemoveInactiveClients();
		UDPServerDisplayConnectedClients();
		UDPServerBroadcastPackets( udpServerSocket );
		UDPServerUpdateClientTimes();
	}

	return runUntilAppClosed;
}

//-----------------------------------------------------------------------------------------------
void AddClientToListOfConnectedClients( const sockaddr_in& senderInfo, const char* receiveBuffer )
{
	char buffer[ 100 ];

	std::string senderIP = inet_ntoa( senderInfo.sin_addr );
	std::string senderPort = _itoa( static_cast< int >( senderInfo.sin_port ), buffer, 10 );

	std::string senderID = senderPort + senderIP;

	packet newPacket = CreatePacketFromBuffer( receiveBuffer );

	auto iter = g_relevantClients.begin();

	for( ; iter != g_relevantClients.end(); ++iter )
	{
		if( iter->ID == senderID )
		{
			iter->timeSinceLastRecieve = 0.f;
			iter->mostRecentInfo = newPacket;

			break;
		}
	}	

	if( iter == g_relevantClients.end() )
	{
		uniqueClient newClient( senderIP, senderPort, newPacket );
		g_relevantClients.push_back( newClient );
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
			char* message = ( char* )&g_relevantClients[ i ].mostRecentInfo;
			UDPSendMessage( udpSocket, message, sizeof( packet ), clientInfo );
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