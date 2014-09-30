#ifndef CLIENT_PLAYER_HPP
#define CLIENT_PLAYER_HPP

#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Primitives/Color.hpp"

typedef Color PlayerID;

class ClientPlayer
{

public:

	ClientPlayer();

	void SetCurrentVelocityAndPositionFromMovementVector( const Vector2f& movementDirection );
	void SeekTarget();
	void Reset();

	PlayerID id;

	Vector2f currentPos;
	Vector2f currentVelocity;

	Vector2f desiredPos;
	Vector2f desiredVelocity;

	float	 orientationAsDegrees;
};



#endif