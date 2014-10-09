#include "ConnectedClient.hpp"

//-----------------------------------------------------------------------------------------------
ConnectedClient::ConnectedClient()
	: clientID( "" )
	, ipAddressAsString( "" )
	, portAsString( "" )
	, mostRecentUpdateInfo()
	, timeSinceLastReceivedMessage( 0.f )
	, numMessagesSent( 0 )
{
	memset( &mostRecentUpdateInfo, 0, sizeof( mostRecentUpdateInfo ) );
	memset( &playerIDAsRGB, 0, sizeof( playerIDAsRGB ) );
}


//-----------------------------------------------------------------------------------------------
ConnectedClient::ConnectedClient( const std::string& ipAddress, const std::string& port )
	: clientID( port + ipAddress )
	, ipAddressAsString( ipAddress )
	, portAsString( port )
	, mostRecentUpdateInfo()
	, timeSinceLastReceivedMessage( 0.f )
	, numMessagesSent( 0 )
{
	memset( &mostRecentUpdateInfo, 0, sizeof( mostRecentUpdateInfo ) );
	memset( &playerIDAsRGB, 0, sizeof( playerIDAsRGB ) );
}


//-----------------------------------------------------------------------------------------------
ConnectedClient::ConnectedClient( const std::string& ipAddress, const std::string& port, const UpdatePacket& incomingPacket )
	: clientID( port + ipAddress )
	, ipAddressAsString( ipAddress )
	, portAsString( port )
	, mostRecentUpdateInfo( incomingPacket )
	, timeSinceLastReceivedMessage( 0.f )
	, numMessagesSent( 0 )
{
	memset( &playerIDAsRGB, 0, sizeof( playerIDAsRGB ) );
}