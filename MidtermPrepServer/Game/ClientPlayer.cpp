#include "ClientPlayer.hpp"

#include "Engine/Utilities/Clock.hpp"

const float PLAYER_SPEED = 3000.f;


//-----------------------------------------------------------------------------------------------
ClientPlayer::ClientPlayer()
	: id()
	, currentPos( Vector2f::Zero() )
	, currentVelocity( Vector2f::Zero() )
	, desiredPos( Vector2f::Zero() )
	, desiredVelocity( Vector2f::Zero() )
	, orientationAsDegrees( 0.f )
{

}


//-----------------------------------------------------------------------------------------------
void ClientPlayer::SetCurrentVelocityAndPositionFromMovementVector( const Vector2f& movementDirection )
{
	Clock& clock = Clock::GetMasterClock();

	currentVelocity = movementDirection * PLAYER_SPEED * static_cast< float >( clock.m_currentDeltaSeconds );

	currentPos += currentVelocity * static_cast< float >( clock.m_currentDeltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void ClientPlayer::SeekTarget()
{
	Vector2f currentToDesiredVector = desiredPos - currentPos;
	Vector2f currentVelocityDirection = currentVelocity;

	currentToDesiredVector.Normalize();
	currentVelocityDirection.Normalize();

	Vector2f newVelocityDirection = currentToDesiredVector;// - currentVelocityDirection;

	newVelocityDirection.Normalize();

	Clock& clock = Clock::GetMasterClock();

	if( Vector2f::Distance( currentPos, desiredPos ) > 1.f )
	{
		currentVelocity = newVelocityDirection * PLAYER_SPEED * static_cast< float >( clock.m_currentDeltaSeconds );
		currentPos += currentVelocity * static_cast< float >( clock.m_currentDeltaSeconds );
	}
	else
	{
		currentVelocity = Vector2f::Zero();
	}
}


//-----------------------------------------------------------------------------------------------
void ClientPlayer::Reset()
{
	currentPos = Vector2f::Zero();
	currentVelocity = Vector2f::Zero();
	desiredPos = Vector2f::Zero();
	desiredVelocity = Vector2f::Zero();
	orientationAsDegrees = 0.f;
}
