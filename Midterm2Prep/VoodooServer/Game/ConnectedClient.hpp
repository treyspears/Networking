#ifndef CONNECTED_CLIENT_HPP
#define CONNECTED_CLIENT_HPP

#include "CS6Packet.hpp"
#include "Engine/Primitives/Color.hpp"

typedef Color PlayerID;

class ConnectedClient
{

public:

	ConnectedClient();
	ConnectedClient( const std::string& ipAddress, const std::string& port );
	ConnectedClient( const std::string& ipAddress, const std::string& port, const UpdatePacket& incomingPacket );

	std::string  clientID;
	std::string  ipAddressAsString;
	std::string  portAsString;

	UpdatePacket mostRecentUpdateInfo;
	PlayerID	 playerIDAsRGB;

	float		 timeSinceLastReceivedMessage;
	int			 numMessagesSent;
};

#endif