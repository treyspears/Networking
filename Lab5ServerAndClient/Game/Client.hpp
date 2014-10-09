#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <set>

#include "Engine/Networking/Network.hpp"
#include "Engine/Utilities/EventSystem.hpp"

#include "CS6Packet.hpp"
#include "ClientPlayer.hpp"

class PacketComparator
{
public:

	bool operator () ( const CS6Packet& lhs, const CS6Packet& rhs ) const
	{
		return lhs.packetNumber < rhs.packetNumber;
	}
};


enum ClientState
{
	CLIENT_UNCONNECTED,
	CLIENT_IN_LOBBY,
	CLIENT_IN_GAME
};

class Client
{

public:

	Client();
	~Client();

	void	StartUp();
	void	ShutDown();

	void	UpdateLocalPlayerMovementMagnitude( float magnitude );
	void	Update();

	void	Render() const;

	void	ChangeHost( const std::string& ipAddressAsString, const std::string& portAsString );
	
	ClientPlayer&					GetLocalPlayer();
	std::vector< ClientPlayer >&	GetOtherClientsPlayers();

	void SetGameTargetToPrevious();
	void SetGameTargetToNext();
	void CreateGame();
	void JoinTargetedGame();

	inline void	RegisterForEvents();
	inline ClientState	GetClientState() const;

private:

	void		ReceiveMessagesFromHostIfAny();
	void		UpdatePlayers();
	bool		CheckForCollision( float& elapsedSendTime, float sendToTime );
	void		PotentiallySendAckAckPacketToServer( float& elapsedSendTime, float sendToTime );
	void		PotentiallySendUpdatePacketToServer( float& elapsedSendTime, float sendToTime );
	void		PotentiallySendVictoryPacketToServer( float& elapsedSendTime, float sendToTime );
	void		AckBackSuccessfulReliablePacketReceive( const CS6Packet& packet );

	void		RenderPlayers() const;
	void		RenderPlayer( const ClientPlayer& playerToRender ) const;
	void		RenderFlag() const;
	void		RenderListedGames() const;
	void		RenderConnecting() const;

	void		ProcessPacket( const CS6Packet& packet );
	void		ProcessAnyQueuedReliablePackets();

	void		OnReceiveUpdatePacket( const CS6Packet& updatePacket );
	void		OnReceiveLobbyStartPacket( const CS6Packet& lobbyStartPacket );
	void		OnReceiveGameStartPacket( const CS6Packet& gameStartPacket );
	void		OnReceiveResetPacket( const CS6Packet& resetPacket );
	void		OnReceiveVictoryPacket( const CS6Packet& victoryPacket );
	void		OnReceiveAckPacket( const CS6Packet& ackPacket );
	void		OnReceiveAckLobbyStartPacket( const CS6Packet& ackPacket );

	CS6Packet	GetGameStartAckPacket();
	CS6Packet	GetAckAckPacket();
	CS6Packet	GetHostGamePacket();
	CS6Packet	GetJoinGamePacket();
	CS6Packet	GetUpdatePacketFromPlayer( const ClientPlayer& player );
	CS6Packet	GetVictoryPacketFromPlayer( const ClientPlayer& player );

	void		SetPlayerParameters( NamedProperties& parameters );
	void		SetServerIpFromParameters( NamedProperties& parameters );
	void		SetServerPortFromParameters( NamedProperties& parameters );

	ClientState					m_currentState;
	Vector2f					m_flagPosition;
	//PlayerID					m_currentItPlayerID;

	int							m_connectionToHostID;

	std::string					m_currentHostIPAddressAsString;
	u_short						m_currentHostPort;

	float						m_localPlayerMovementMagnitude;
	ClientPlayer				m_localPlayer;
	std::vector< ClientPlayer >	m_otherClientsPlayers;

	float						m_sendPacketsToHostFrequency;

	uint						m_mostRecentlyProcessedUnreliablePacketNum;
	uint						m_nextExpectedReliablePacketNumToProcess;

	std::set< CS6Packet, PacketComparator > m_queueOfReliablePacketsToParse;
	
	std::set< unsigned int >	m_gameIDsFromLobby;
	int							m_selectedGameID;
	unsigned int				m_currentGameID;
};

//-----------------------------------------------------------------------------------------------
inline void Client::RegisterForEvents()
{
	EventSystem& eventSystem = EventSystem::GetInstance();

	eventSystem.RegisterEventWithCallbackAndObject( "playerColor", &Client::SetPlayerParameters, this );
	eventSystem.RegisterEventWithCallbackAndObject( "ip", &Client::SetServerIpFromParameters, this );
	eventSystem.RegisterEventWithCallbackAndObject( "port", &Client::SetServerPortFromParameters, this );
}


//-----------------------------------------------------------------------------------------------
inline ClientState Client::GetClientState() const
{
	return m_currentState;
}




#endif