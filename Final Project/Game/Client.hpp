#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <set>

#include "Engine/Networking/Network.hpp"
#include "Engine/Utilities/EventSystem.hpp"

#include "FinalPacket.hpp"
#include "ClientPlayer.hpp"
#include "Camera3D.hpp"

enum ClientState
{
	CLIENT_UNCONNECTED,
	CLIENT_IN_LOBBY,
	CLIENT_AWAITING_RESET,
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
	void	RenderPlayers() const;
	void	Render2DSprites() const;

	void	ChangeHost( const std::string& ipAddressAsString, const std::string& portAsString );

	void SetGameTargetToPrevious();
	void SetGameTargetToNext();
	void CreateGame();
	void JoinTargetedGame();

	inline void	RegisterForEvents();
	inline ClientState	GetClientState() const;

	
	ClientPlayer* GetLocalPlayer();
	std::vector< ClientPlayer >& GetOtherClientsPlayers();

	void Shoot();

	void UpdateLocalPlayerMovementMagnitude( float magnitude );

	const Camera3D& GetCameraToRenderFrom() const;

private:

	void		LoadCameraXMLDefinitions();
	void		ReceiveMessagesFromHostIfAny();

	void		PotentiallySendJoinLobbyPacketToServer( float& elapsedSendTime, float sendToTime );
	void		PotentiallySendJoinRoomPacketToServer( RoomID roomToJoin, float& elapsedSendTime, float sendToTime );
	void		PotentiallySendKeepAlivePacketToServer( float& elapsedSendTime, float sendToTime );
	void		PotentiallySendUpdatePacketToServer( float& elapsedSendTime, float sendToTime );
	void		ResendReliablePacket( FinalPacket& packetToSend );
	void		PotentiallyResendReliablePacketsThatHaventBeenAckedBack();
	void		SendFirePacket();
	void		SendMessageToHost( FinalPacket& packetToSend );

	void		AckBackSuccessfulReliablePacketReceive( const FinalPacket& packet );

	void		UpdatePlayers();
	void		UpdateCameras();

	void		RenderInGameStateInfo() const;
	void		RenderListedRooms() const;
	void		RenderConnecting() const;
	void		RenderPlayer( const ClientPlayer& playerToRender ) const;

	void		ProcessPacket( const FinalPacket& packet );
	void		ProcessAnyQueuedReliablePackets();

	void		OnReceiveReturnToLobbyPacket( const FinalPacket& returnToLobbyPacket );
	void		OnReceiveLobbyUpdatePacket( const FinalPacket& lobbyUpdatePacket );
	void		OnReceiveGameUpdatePacket( const FinalPacket& gameUpdatePacket );
	void		OnReceiveGameResetPacket( const FinalPacket& gameResetPacket );
	void		OnReceiveFirePacket( const FinalPacket& firePacket );
	void		OnReceiveHitPacket( const FinalPacket& hitPacket );
	void		OnReceiveRespawnPacket( const FinalPacket& respawnPacket );
	void		OnReceiveAckPacket( const FinalPacket& ackPacket );
	void		OnReceiveAckJoinRoomPacket( const FinalPacket& ackPacket );
	void		OnReceiveAckCreateRoomPacket( const FinalPacket& createRoomAckPacket );
	void		OnReceiveNackPacket( const FinalPacket& nackPacket );

	void		RemoveReliablePacketFromQueue( const FinalPacket& packetToRemove );

	FinalPacket	GetJoinRoomPacket( RoomID roomToConnectTo ) const;
	FinalPacket GetCreateGamePacket( RoomID roomToConnectTo ) const;
	FinalPacket GetUpdatePacket() const;

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
	uint						m_mostRecentlyProcessedReliablePacketNum;

	uint						m_mostRecentUnreliablePacketSentNum;
	uint						m_mostRecentReliablePacketSentNum;

	std::set< FinalPacket >		m_queueOfReliablePacketsToParse;
	std::vector< FinalPacket >	m_queueOfReliablePacketsSentToServer;
	
	char				m_numPlayersInRoom[ NUM_ROOMS ];
	char				m_selectedRoomID;
	RoomID				m_currentJoinRoomRequestNum;
	RoomID				m_currentRoomID;

	FinalPacket			m_mostRecentResetInfo;

	float						m_localPlayerMovementMagnitude;
	ClientPlayer*				m_localPlayer;
	ClientPlayer*				m_localPlayerStatsToSendToServer;
	std::vector< ClientPlayer >	m_otherClientsPlayers;

	bool						m_shouldFire;

	std::vector< Camera3D* > m_cameras;
	Camera3D*				 m_cameraToRenderFrom;
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