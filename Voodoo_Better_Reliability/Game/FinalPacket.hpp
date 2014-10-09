#pragma once
#ifndef INCLUDED_FINAL_PACKET_HPP
#define INCLUDED_FINAL_PACKET_HPP

#pragma region Change Log
/*
	Change Log:
	v1.0: (VK) - Original version
	v1.1: (VK) - Added a none player index. 
				 Changed id variable in header to clientID to be more clear about whose ID is in the packet.
	v1.2: (PR) - Made operator < and IsGuaranteed() inline to fix potential linker issues
	v1.3: (VK) - Made ErrorCode 0 indicate success, and 255 be unknown.
				 This way, functions can use ErrorCode to indicate success or failure.
				 Prettied up the change log...because reasons.
*/
#pragma endregion //Change Log

#pragma region Game Rules
//-----------------------------------------------------------------------------------------------
//Game Specifications:
//	Arena Dimensions: 500x500
//	Kills to Win: 10


//-----------------------------------------------------------------------------------------------
//Tank Specifications:
//	Max Health: 1
//	Collider: Circle, Radius 10
//	Velocity: 30 units/second
//	Orientation: Instant Facing (Infinite deg/second)
//		Turret orientation = tank forward orientation
//	On reset, tanks get 2 seconds of invulnerability


//-----------------------------------------------------------------------------------------------
//Weapon Specifications:
//	Laser shots have infinite penetration (hit all targets on that line)
//	Laser shots do 1 damage
#pragma endregion //Game Rules

#pragma region Network Protocol
//-----------------------------------------------------------------------------------------------
//PROTOCOL START
//	Client->Server: Join( ROOM_Lobby )
//	Server->Client: Ack
//	Server->Client: LobbyUpdate
//	GOTO LOBBY LOOP


//LOBBY LOOP
//	Until client chooses an option:
//		Client->Server: KeepAlive
//		Server->Client: LobbyUpdate

//	Client->Server: CreateRoom( # )
//		If room # is empty:
//			Server->Client: Ack
//			Server->Client: GameReset (implicit respawn)
//			GOTO GAME LOOP
//		If room # is full:
//			Server->Client: Nack( ERROR_RoomFull )
//			GOTO LOBBY LOOP

//	Client->Server: JoinRoom( # )
//		If room # is empty:
//			Server->Client: Nack( ERROR_RoomEmpty )
//			GOTO LOBBY LOOP
//		If room # is full:
//			Server->Client: Ack
//			Server->Client: GameReset (implicit respawn)
//			GOTO GAME LOOP


//GAME LOOP
//	Client->Server: Update, Hit, Fire
//	Server->Client: Update, Respawn

//	When end score is reached OR host exits the game:
//		Server->ALL Clients: ReturnToLobby
//		Client->Server: Ack
//		GOTO LOBBY LOOP
#pragma endregion //Network Protocol

#pragma region Packet Type Definitions
//-----------------------------------------------------------------------------------------------
typedef unsigned char ClientID;
static const ClientID ID_None = 0;

typedef unsigned char RoomID;
static const RoomID ROOM_Lobby = 0;
static const RoomID ROOM_None = 255;

typedef unsigned int PacketNumber;

//-----------------------------------------------------------------------------------------------
typedef unsigned char PacketType;
static const PacketType TYPE_None = 0;
static const PacketType TYPE_Ack = 1;
static const PacketType TYPE_Nack = 2;
static const PacketType TYPE_KeepAlive = 3;
static const PacketType TYPE_CreateRoom = 4;
static const PacketType TYPE_JoinRoom = 5;
static const PacketType TYPE_LobbyUpdate = 6;
static const PacketType TYPE_GameUpdate = 7;
static const PacketType TYPE_GameReset = 8;
static const PacketType TYPE_Respawn = 9;
static const PacketType TYPE_Hit = 10;
static const PacketType TYPE_Fire = 11;
static const PacketType TYPE_ReturnToLobby = 12;

//-----------------------------------------------------------------------------------------------
typedef unsigned char ErrorCode;
static const ErrorCode ERROR_None = 0;
static const ErrorCode ERROR_RoomEmpty = 1;
static const ErrorCode ERROR_RoomFull = 2;
static const ErrorCode ERROR_BadRoomID = 3;
static const ErrorCode ERROR_Unknown = 255;
#pragma endregion //Packet Type Definitions



#pragma region Packet Structure Definitions
//-----------------------------------------------------------------------------------------------
struct AckPacket
{
	PacketType type;
	PacketNumber number;
};

//-----------------------------------------------------------------------------------------------
struct NackPacket
{
	PacketType type;
	PacketNumber number;
	ErrorCode errorCode;
};

//-----------------------------------------------------------------------------------------------
struct KeepAlivePacket
{

};

//-----------------------------------------------------------------------------------------------
struct CreateRoomPacket
{
	// 1-8 creates room at i-1
	// 0, 9-255 is an error (ERROR_BadRoomID)
	RoomID room;
};

//-----------------------------------------------------------------------------------------------
struct JoinRoomPacket
{
	// 0 joins lobby
	// 1-8 joins room at i-1
	// 9-255 is an error (ERROR_BadRoomID)
	RoomID room;
};

//-----------------------------------------------------------------------------------------------
struct LobbyUpdatePacket
{
	char playersInRoomNumber[ 8 ];
};

//-----------------------------------------------------------------------------------------------
struct GameUpdatePacket
{
	float xPosition;
	float yPosition;
	float xVelocity;
	float yVelocity;
	float xAcceleration;
	float yAcceleration;
	float orientationDegrees; //0-359.99, 0 = east
	unsigned char health;
	unsigned char score;
};

//-----------------------------------------------------------------------------------------------
struct GameResetPacket
{
	float xPosition;
	float yPosition;
	float orientationDegrees;
	ClientID id;
};

//-----------------------------------------------------------------------------------------------
struct RespawnPacket
{
	float xPosition;
	float yPosition;
	float orientationDegrees;
};

//-----------------------------------------------------------------------------------------------
struct HitPacket
{
	ClientID instigatorID;
	ClientID targetID;
	unsigned char damageDealt;
};

//-----------------------------------------------------------------------------------------------
struct GunfirePacket
{
	ClientID instigatorID;
};

//-----------------------------------------------------------------------------------------------
struct ReturnToLobbyPacket
{

};
#pragma endregion //Packet Structure Definitions



//-----------------------------------------------------------------------------------------------
struct FinalPacket
{
	//Header
	PacketType type;
	ClientID clientID;
	PacketNumber number;
	double timestamp;

	union PacketData
	{
		AckPacket acknowledged;
		NackPacket refused;
		KeepAlivePacket keptAlive;
		CreateRoomPacket creating;
		JoinRoomPacket joining;
		LobbyUpdatePacket updatedLobby;
		GameUpdatePacket updatedGame;
		GameResetPacket reset;
		RespawnPacket respawn;
		HitPacket hit;
		GunfirePacket gunfire;
		ReturnToLobbyPacket lobbyReturn;
	} data;


	//Functions
	bool operator<( const FinalPacket& other ) const;

	bool IsGuaranteed() const;
};


//-----------------------------------------------------------------------------------------------
inline bool FinalPacket::operator<( const FinalPacket& other ) const
{
	return this->number < other.number;
}

//-----------------------------------------------------------------------------------------------
inline bool FinalPacket::IsGuaranteed() const
{
	switch( type )
	{
	case TYPE_CreateRoom:
	case TYPE_JoinRoom:
	case TYPE_GameReset:
	case TYPE_Respawn:
	case TYPE_Hit:
	case TYPE_Fire:
	case TYPE_ReturnToLobby:
		return true;

	case TYPE_Ack:
	case TYPE_Nack:
	case TYPE_KeepAlive:
	case TYPE_LobbyUpdate:
	case TYPE_GameUpdate:
	case TYPE_None:
	default:
		break;
	}
	return false;
}



//-----------------------------------------------------------------------------------------------
class FinalPacketComparer
{
public:
	bool operator() (const FinalPacket& lhs, const FinalPacket& rhs) const
	{
		if( lhs.number < rhs.number )
			return true;
		return false;
	}
};

#endif //INCLUDED_FINAL_PACKET_HPP