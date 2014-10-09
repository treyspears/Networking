#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <set>

#include "Engine/Networking/Network.hpp"
#include "Engine/Utilities/EventSystem.hpp"

#include "FinalPacket.hpp"
#include "ClientPlayer.hpp"


enum ClientState
{
	CLIENT_UNCONNECTED,
	CLIENT_IN_LOBBY,
	CLIENT_IN_GAME
};

typedef uchar RoomID;
const RoomID NUM_ROOMS = 8;


class Client
{

public:

	Client();
	~Client();

	void	StartUp();
	void	ShutDown();

	void	Update();
	void	Render() const;

	void	ChangeHost( const std::string& ipAddressAsString, const std::string& portAsString );

	void SetGameTargetToPrevious();
	void SetGameTargetToNext();
	void CreateGame();
	void JoinTargetedGame();

	inline void	RegisterForEvents();
	inline ClientState	GetClientState() const;

private:

	void		ReceiveMessagesFromHostIfAny();

	void		PotentiallySendJoinLobbyPacketToServer( float& elapsedSendTime, float sendToTime );
	void		PotentiallySendJoinRoomPacketToServer( RoomID roomToJoin, float& elapsedSendTime, float sendToTime );
	void		PotentiallySendKeepAlivePacketToServer( float& elapsedSendTime, float sendToTime );
	void		ResendReliablePacket( FinalPacket& packetToSend );
	void		PotentiallyResendReliablePacketsThatHaventBeenAckedBack();
	void		SendMessageToHost( FinalPacket& packetToSend );

	void		AckBackSuccessfulReliablePacketReceive( const FinalPacket& packet );

	void		RenderImInARoom() const;
	void		RenderListedRooms() const;
	void		RenderConnecting() const;

	void		ProcessPacket( const FinalPacket& packet );
	void		ProcessAnyQueuedReliablePackets();

	void		OnReceiveReturnToLobbyPacket( const FinalPacket& returnToLobbyPacket );
	void		OnReceiveLobbyUpdatePacket( const FinalPacket& lobbyStartPacket );
	void		OnReceiveGameResetPacket( const FinalPacket& gameResetPacket );
	void		OnReceiveAckPacket( const FinalPacket& ackPacket );
	void		OnReceiveAckJoinRoomPacket( const FinalPacket& ackPacket );
	void		OnReceiveAckCreateRoomPacket( const FinalPacket& createRoomAckPacket );
	void		OnReceiveNackPacket( const FinalPacket& nackPacket );

	void		RemoveReliablePacketFromQueue( const FinalPacket& packetToRemove );

	FinalPacket	GetJoinRoomPacket( RoomID roomToConnectTo ) const;
	FinalPacket GetCreateGamePacket( RoomID roomToConnectTo ) const;

	void		SetPlayerParameters( NamedProperties& parameters );
	void		SetServerIpFromParameters( NamedProperties& parameters );
	void		SetServerPortFromParameters( NamedProperties& parameters );

	void		OutputReceivedPacket( const FinalPacket& packetToOutput ) const;
	std::string GetReceivedPacketTypeAsString( const PacketType& typeOfPacket ) const;
	std::string GetReceivedNackErrorCodeAsString( const ErrorCode& codeError ) const;

	int							m_connectionToHostID;
	std::string					m_currentHostIPAddressAsString;
	u_short						m_currentHostPort;

	ClientState					m_currentState;

	uint						m_mostRecentlyProcessedUnreliablePacketNum;
	uint						m_nextExpectedReliablePacketNumToProcess;

	uint						m_mostRecentUnreliablePacketSentNum;
	uint						m_mostRecentReliablePacketSentNum;

	std::set< FinalPacket >		m_queueOfReliablePacketsToParse;
	std::vector< FinalPacket >	m_queueOfReliablePacketsSentToServer;
	
	char				m_numPlayersInRoom[ NUM_ROOMS ];
	char				m_selectedRoomID;
	RoomID				m_currentJoinRoomRequestNum;
	RoomID				m_currentRoomID;

	FinalPacket			m_mostRecentResetInfo;
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