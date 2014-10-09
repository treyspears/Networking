#ifndef CONNECTED_CLIENT_HPP
#define CONNECTED_CLIENT_HPP

#include <map>

#include "FinalPacket.hpp"
#include "Engine/Primitives/Color.hpp"

typedef unsigned int GameID;
typedef unsigned int ConnectionID;
typedef Color PlayerID;

class ConnectedClient
{

public:

	ConnectedClient();
	ConnectedClient( const std::string& ipAddress, const std::string& port );
	ConnectedClient( const std::string& ipAddress, const std::string& port, const GameUpdatePacket& incomingPacket );

	std::string  clientID;
	std::string  ipAddressAsString;
	std::string  portAsString;

	GameUpdatePacket mostRecentUpdateInfo;
	PlayerID	 playerIDAsRGB;

	float		 timeSinceLastReceivedMessage;
	int			 numUnreliableMessagesSent;
	int			 numReliableMessagesSent;
	ConnectionID connectionID;
	GameID		 gameID;

	std::map< uint, FinalPacket > m_reliablePacketsAwaitingAckBack;

	static ConnectionID s_currentConnectedID;
};

#endif