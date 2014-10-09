#include "Network.hpp"

//-----------------------------------------------------------------------------------------------
Network::Network()
	: m_currentNumConnectionIDs( 0 )
	, m_hostInformation( nullptr )
	, m_hasBeenInitialized( false )
{
	ZeroMemory( &m_winSockAddrData, sizeof( m_winSockAddrData ) );
}


//-----------------------------------------------------------------------------------------------
Network::~Network()
{
	ShutDown();
}


//-----------------------------------------------------------------------------------------------
void Network::Startup()
{
	int result = 0;

	result = WSAStartup( MAKEWORD( 2, 2 ), &m_winSockAddrData );

	if( result != 0 )
	{
		printf( "Failed, Error Code: %d", WSAGetLastError() );
		exit( EXIT_FAILURE );
	}
}


//-----------------------------------------------------------------------------------------------
void Network::ShutDown()
{
	WSACleanup();
}


//-----------------------------------------------------------------------------------------------
int Network::CreateUDPSocketFromDomainNameAndPort( const char* domainAsString, u_short port )
{
	Connection* newConnection = new Connection();

	int newConnectionID = m_currentNumConnectionIDs;
	++m_currentNumConnectionIDs;

	newConnection->m_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if( newConnection->m_socket == SOCKET_ERROR )
	{
		printf( "socket() failed, Error Code: %d", WSAGetLastError() );
		exit( EXIT_FAILURE );
	}

	m_hostInformation = gethostbyname( domainAsString );

	memset( ( char* )&newConnection->m_socketAddressInfo, 0, newConnection->m_socketAddrInfoLength );

	newConnection->m_socketAddressInfo.sin_family = AF_INET;
	newConnection->m_socketAddressInfo.sin_port = htons( port );
	//grabs first ip from host
	newConnection->m_socketAddressInfo.sin_addr.s_addr = inet_addr( inet_ntoa( *(struct in_addr* )*m_hostInformation->h_addr_list ) );

	u_long nonBlockingIO = 1;
	ioctlsocket( newConnection->m_socket, FIONBIO, &nonBlockingIO );


	std::pair< int, Connection* > connectionPair( newConnectionID, newConnection );
	m_connections.insert( connectionPair );

	return newConnectionID;
}


//-----------------------------------------------------------------------------------------------
int Network::CreateUDPSocketFromIPAndPort( const char* ipAddressAsString, u_short port )
{
	Connection* newConnection = new Connection();

	int newConnectionID = m_currentNumConnectionIDs;
	++m_currentNumConnectionIDs;

	newConnection->m_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	if( newConnection->m_socket == SOCKET_ERROR )
	{
		printf( "socket() failed, Error Code: %d", WSAGetLastError() );
		exit( EXIT_FAILURE );
	}

	memset( ( char* )&newConnection->m_socketAddressInfo, 0, newConnection->m_socketAddrInfoLength );

	newConnection->m_socketAddressInfo.sin_family = AF_INET;
	newConnection->m_socketAddressInfo.sin_port = htons( port );
	newConnection->m_socketAddressInfo.sin_addr.S_un.S_addr = inet_addr( ipAddressAsString );

	u_long nonBlockingIO = 1;
	ioctlsocket( newConnection->m_socket, FIONBIO, &nonBlockingIO );


	std::pair< int, Connection* > connectionPair( newConnectionID, newConnection );
	m_connections.insert( connectionPair );

	return newConnectionID;
}


//-----------------------------------------------------------------------------------------------
void Network::BindSocket( int connectionID )
{
	std::unordered_map< int, Connection* >::const_iterator connectionIter = m_connections.find( connectionID );

	int bindResultAsInt = SOCKET_ERROR;

	if( connectionIter != m_connections.end() )
	{
		Connection* connection = connectionIter->second;

		bindResultAsInt = bind( connection->m_socket, ( struct sockaddr * )&connection->m_socketAddressInfo, connection->m_socketAddrInfoLength );
	}
}

//-----------------------------------------------------------------------------------------------
int Network::SendUDPMessage( void* message, int messageSize, int connectionID )
{
	int bytesSent = -1;

	std::unordered_map< int, Connection* >::const_iterator connectionIter = m_connections.find( connectionID );
	
	if( connectionIter != m_connections.end() )
	{
		Connection* connection = connectionIter->second;
		bytesSent = sendto( connection->m_socket, ( char*)message, messageSize, 0, ( struct sockaddr* )&connection->m_socketAddressInfo, connection->m_socketAddrInfoLength );
	}

	if( bytesSent == SOCKET_ERROR )
	{
		printf( "Failed to send, Error Code: %d", WSAGetLastError() );
		exit( EXIT_FAILURE );
	}

	return bytesSent;
}


//-----------------------------------------------------------------------------------------------
int Network::ReceiveUDPMessage( void* buffer, int bufferSize, int connectionID )
{
	int bytesReceived = -1;
	
	std::unordered_map< int, Connection* >::const_iterator connectionIter = m_connections.find( connectionID );
	
	if( connectionIter != m_connections.end() )
	{
		Connection* connection = connectionIter->second;
		bytesReceived = recvfrom( connection->m_socket, ( char* )buffer, bufferSize, 0, ( struct sockaddr* )&connection->m_socketAddressInfo, &connection->m_socketAddrInfoLength );
	}

	return bytesReceived;
}


//-----------------------------------------------------------------------------------------------
void Network::CloseUDPSocket( int connectionID )
{
	//FUTURE EDIT: Need to clean up socket from memory

	std::unordered_map< int, Connection* >::const_iterator connectionIter = m_connections.find( connectionID );
	Connection* connection = connectionIter->second;

	closesocket( connection->m_socket );
}


//-----------------------------------------------------------------------------------------------
std::string Network::GetIPAddressAsStringFromConnection( int connectionID )
{
	std::string result = "";

	std::unordered_map< int, Connection* >::const_iterator connectionIter = m_connections.find( connectionID );
	
	if( connectionIter != m_connections.end() )
	{
		Connection* connection = connectionIter->second;

		result = inet_ntoa( connection->m_socketAddressInfo.sin_addr );
	}
	
	return result;
}


//-----------------------------------------------------------------------------------------------
std::string Network::GetPortAsStringFromConnection( int connectionID )
{
	std::string result = "";

	std::unordered_map< int, Connection* >::const_iterator connectionIter = m_connections.find( connectionID );

	if( connectionIter != m_connections.end() )
	{
		char buffer[ 100 ];

		Connection* connection = connectionIter->second;

		result = _itoa( static_cast< int >( ntohs( connection->m_socketAddressInfo.sin_port ) ), buffer, 10 );
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void Network::SetIPAddressAsStringForConnection( int connectionID, const std::string& ipAddressAsString )
{
	std::unordered_map< int, Connection* >::const_iterator connectionIter = m_connections.find( connectionID );

	if( connectionIter != m_connections.end() )
	{
		Connection* connection = connectionIter->second;

		connection->m_socketAddressInfo.sin_addr.S_un.S_addr = inet_addr( ipAddressAsString.c_str() );
	}
}


//-----------------------------------------------------------------------------------------------
void Network::SetPortAsStringForConnection( int connectionID, const std::string& portAsString )
{
	std::unordered_map< int, Connection* >::const_iterator connectionIter = m_connections.find( connectionID );

	if( connectionIter != m_connections.end() )
	{
		Connection* connection = connectionIter->second;

		connection->m_socketAddressInfo.sin_port = htons( u_short( atoi( portAsString.c_str() ) ) );
	}
}