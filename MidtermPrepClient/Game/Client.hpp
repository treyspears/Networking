#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

#include "Engine/Networking/Network.hpp"
#include "Engine/Utilities/EventSystem.hpp"

#include "CS6Packet.hpp"
#include "ClientPlayer.hpp"

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

	inline void	RegisterForEvents();

private:

	void		ReceiveMessagesFromHostIfAny();
	void		UpdatePlayers();
	void		PotentiallySendUpdatePacketToServer();

	void		RenderPlayers() const;
	void		RenderPlayer( const ClientPlayer& playerToRender ) const;

	void		ProcessPacket( const CS6Packet& packet );
	void		OnReceivedUpdatePacket( const CS6Packet& updatePacket );

	CS6Packet	GetUpdatePacketFromPlayer( const ClientPlayer& player );

	void		SetPlayerParameters( NamedProperties& parameters );
	void		SetServerIpFromParameters( NamedProperties& parameters );
	void		SetServerPortFromParameters( NamedProperties& parameters );


	int							m_connectionToHostID;

	std::string					m_currentHostIPAddressAsString;
	u_short						m_currentHostPort;

	float						m_localPlayerMovementMagnitude;
	ClientPlayer				m_localPlayer;
	std::vector< ClientPlayer >	m_otherClientsPlayers;

	float						m_sendPacketsToHostFrequency;
};

//-----------------------------------------------------------------------------------------------
inline void Client::RegisterForEvents()
{
	EventSystem& eventSystem = EventSystem::GetInstance();

	eventSystem.RegisterEventWithCallbackAndObject( "playerColor", &Client::SetPlayerParameters, this );
	eventSystem.RegisterEventWithCallbackAndObject( "ip", &Client::SetServerIpFromParameters, this );
	eventSystem.RegisterEventWithCallbackAndObject( "port", &Client::SetServerPortFromParameters, this );
}

#endif