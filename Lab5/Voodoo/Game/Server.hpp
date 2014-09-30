#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>

#include "Engine/Networking/Network.hpp"
#include "Engine/Utilities/EventSystem.hpp"

#include "CS6Packet.hpp"
#include "ConnectedClient.hpp"

//-----------------------------------------------------------------------------------------------
typedef unsigned int ConnectionID;

class Server
{

public:

	Server();
	~Server();

	void StartUp();
	void ShutDown();

	void Update();
	void Render() const;

	inline void RegisterForEvents();

private:

	void ReceiveMessagesFromClientsIfAny();
	void ProcessPacket( const CS6Packet& packet );

	void OnReceiveUpdatePacket( const CS6Packet& packet );
	void OnReceiveVictoryPacket( const CS6Packet& packet );
	void OnReceiveHostGamePacket( const CS6Packet& packet );
	void OnReceiveJoinGamePacket( const CS6Packet& packet ); 
	void OnReceiveAckPacket( const CS6Packet& packet );
	void OnAckAcknowledge( const CS6Packet& packet );
	void OnAckReliablePacket( const CS6Packet& packet );

	void RemoveInactiveClients();
	void ResendAnyGuaranteedPacketsThatHaveTimedOut();

	void SendUpdatePacketsToAllClients();
	void BroadCastMessageToAllClients( const CS6Packet& messageAsPacket, int messageLength );
	void BroadCastMessageToAllClientsInRoom( GameID room, const CS6Packet& messageAsPacket, int messageLength );
	void SendAGameStartPacketToNewClient( ConnectedClient& clientToSendTo );
	void PutNewClientInLobbyAndSendListOfCurrentGames( ConnectedClient& clientToSendTo );
	void SendMessageToClient( const CS6Packet& messageAsPacket, ConnectedClient& clientToSendTo );

	void AddOrUpdateConnectedClient( const CS6Packet& packet );
	void RemoveClientFromRoom( GameID roomID, ConnectedClient& clientToSendRemove );
	void RemoveRoom( GameID roomID );

	void RenderConnectedClients() const;

	void SetServerIpFromParameters( NamedProperties& parameters );
	void SetPortToBindToFromParameters( NamedProperties& parameters );

	int m_listenConnectionID;

	std::string m_currentServerIPAddressAsString;
	u_short		m_currentServerPort;

	//PlayerID	m_currentItPlayerID;

	std::map< ConnectionID, ConnectedClient* > m_connectedAndActiveClients;
	//std::vector< ConnectedClient > m_connectedAndActiveClients;
	std::map< GameID, std::vector< int > > m_gamesAndTheirClients;
	std::map< GameID, Vector2f > m_currentFlagPositions;

	static GameID s_currentMaxGameID;
};


//-----------------------------------------------------------------------------------------------
inline void Server::RegisterForEvents()
{
	EventSystem& eventSystem = EventSystem::GetInstance();

	eventSystem.RegisterEventWithCallbackAndObject( "ip", &Server::SetServerIpFromParameters, this );
	eventSystem.RegisterEventWithCallbackAndObject( "port", &Server::SetPortToBindToFromParameters, this );
}


#endif