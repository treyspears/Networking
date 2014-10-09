#ifndef NETWORK_HPP
#define NETWORK_HPP

#pragma once

#include <WinSock2.h>
#include "Connection.hpp"
#include <unordered_map>

#pragma comment( lib, "ws2_32.lib" )

class Network
{
public:

	static Network& GetInstance()
	{
		static Network instance; 

		if( !instance.m_hasBeenInitialized )
		{
			instance.Startup();
			instance.m_hasBeenInitialized = true;
		}

		return instance;
	}

	void Startup();
	void ShutDown();

	int  CreateUDPSocketFromDomainNameAndPort( const char* domainAsString, u_short port );
	int  CreateUDPSocketFromIPAndPort( const char* ipAddressAsString, u_short port );
	void BindSocket( int connectionID );

	int SendUDPMessage( void* message, int messageSize, int connectionID );
	int ReceiveUDPMessage( void* buffer, int bufferSize, int connectionID );

	void CloseUDPSocket( int connectionID );

	std::string GetIPAddressAsStringFromConnection( int connectionID );
	std::string GetPortAsStringFromConnection( int connectionID );

	void SetIPAddressAsStringForConnection( int connectionID, const std::string& ipAddressAsString );
	void SetPortAsStringForConnection( int connectionID, const std::string& portAsString );

private:

	Network(); 
	
	Network( Network const& );              
	void operator=( Network const& ); 
	~Network();

	int				m_currentNumConnectionIDs;
	struct hostent* m_hostInformation;
	WSADATA			m_winSockAddrData;

	bool			m_hasBeenInitialized;

	std::unordered_map< int, Connection* > m_connections;
};

#endif