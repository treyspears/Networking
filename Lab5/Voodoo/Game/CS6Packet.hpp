#pragma once
#ifndef INCLUDED_CS6_PACKET_HPP
#define INCLUDED_CS6_PACKET_HPP

//Communication Protocol:
//   Client->Server: Ack
//   Server->Client: Reset
//   Client->Server: Ack

//   ------Update Loop------
//		Client->Server: Update
//		Server->ALL Clients: Update
//		ALL Clients->Server: Ack
//   ----End Update Loop----

//   Client->Server: Victory
//   Server->Client: Ack
//   Server->ALL Clients: Victory
//   ALL Clients->Server: Ack
//   Server->ALL Clients: Reset

#include <vector>

//-----------------------------------------------------------------------------------------------
typedef unsigned char PacketType;
static const PacketType TYPE_Acknowledge = 10;
static const PacketType TYPE_Victory = 11;
static const PacketType TYPE_Update = 12;
static const PacketType TYPE_GameStart = 13;
static const PacketType TYPE_Reset = 14;
static const PacketType TYPE_LobbyStart = 15;
static const PacketType TYPE_JoinGame = 16;
static const PacketType TYPE_HostGame = 17;

//-----------------------------------------------------------------------------------------------
struct AckPacket
{
	PacketType packetType;
	unsigned int packetNumber;
};

//-----------------------------------------------------------------------------------------------
struct GameStartPacket
{
	float playerXPosition;
	float playerYPosition;
	float flagXPosition;
	float flagYPosition;
	unsigned int gameID;
	//Player orientation should always start at 0 (east)
	unsigned char playerColorAndID[ 3 ];
};

struct ResetPacket
{
	float playerXPosition;
	float playerYPosition;
	float flagXPosition;
	float flagYPosition;
	//Player orientation should always start at 0 (east)
};

struct LobbyStart
{
	unsigned int m_gameID;
};


//-----------------------------------------------------------------------------------------------
struct UpdatePacket
{
	//unsigned int m_gameID;
	float xPosition;
	float yPosition;
	float xVelocity;
	float yVelocity;
	float yawDegrees;

	//0 = east
	//+ = counterclockwise
	//range 0-359
};

//-----------------------------------------------------------------------------------------------
struct VictoryPacket
{
	unsigned char playerColorAndID[ 3 ];
};


//-----------------------------------------------------------------------------------------------
struct JoinGamePacket
{
	unsigned int gameID;
};


//-----------------------------------------------------------------------------------------------
struct CS6Packet
{
	PacketType packetType;
	unsigned char playerColorAndID[ 3 ];
	unsigned int packetNumber;
	double timestamp;
	union PacketData
	{
		AckPacket acknowledged;
		GameStartPacket gameStart;
		LobbyStart lobbyStart;
		JoinGamePacket joinGame;
		ResetPacket reset;
		UpdatePacket updated;
		VictoryPacket victorious;
	} data;

	inline bool IsReliablePacket() const;
};


//-----------------------------------------------------------------------------------------------
inline bool CS6Packet::IsReliablePacket() const
{
	bool result = packetType == TYPE_GameStart 
		|| packetType == TYPE_Reset 
		|| packetType == TYPE_LobbyStart 
		|| packetType == TYPE_HostGame
		|| packetType == TYPE_JoinGame
		|| ( packetType == TYPE_Acknowledge && data.acknowledged.packetType == TYPE_LobbyStart );

	return result;
}

#endif //INCLUDED_CS6_PACKET_HPP