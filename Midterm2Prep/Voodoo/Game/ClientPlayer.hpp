#ifndef CLIENT_PLAYER_HPP
#define CLIENT_PLAYER_HPP

#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Primitives/Color.hpp"

typedef Color PlayerID;

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

	PlayerID id;

	Vector2f currentPos;
	Vector2f currentVelocity;

	Vector2f desiredPos;
	Vector2f desiredVelocity;

	float	 orientationAsDegrees;

	std::vector< Vector2f > m_shipVertexArray;
};



#endif