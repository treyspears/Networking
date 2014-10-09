#ifndef CLIENT_PLAYER_HPP
#define CLIENT_PLAYER_HPP

#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Primitives/Color.hpp"

typedef unsigned char PlayerID;
const PlayerID BAD_PLAYER_ID = 255;

class ClientPlayer
{

public:

	enum RotationDirection { LEFT, RIGHT };

	ClientPlayer();

	void SetCurrentVelocityAndPositionFromMagnitude( float magnitude );
	void SeekTarget();
	void Reset();

	void CreateShipVerts();
	void RotateForTimeStep( RotationDirection direction );
	void RenderShip() const;

	inline void		SetID( PlayerID newID );
	inline PlayerID GetID() const;
	inline Color	GetShipColor() const;

	Vector3f currentPos;
	Vector3f currentVelocity;

	Vector3f desiredPos;
	Vector3f desiredVelocity;

	Vector3f currentAcceleration;

	float	 orientationAsDegrees;
	//bool	 isIt;

	std::vector< Vector2f > m_shipVertexArray;

	unsigned char health;
	unsigned char score;

private:

	PlayerID id;
	Color    m_shipColor;
};


//-----------------------------------------------------------------------------------------------
inline void	ClientPlayer::SetID( PlayerID newID )
{
	id = newID;
	m_shipColor = Color( id );
}


//-----------------------------------------------------------------------------------------------
inline PlayerID ClientPlayer::GetID() const
{
	return id;
}


//-----------------------------------------------------------------------------------------------
inline Color ClientPlayer::GetShipColor() const
{
	return m_shipColor;
}

#endif