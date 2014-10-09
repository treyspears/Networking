#include "ClientPlayer.hpp"

#include "Engine/Utilities/Clock.hpp"
#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"

const float PLAYER_SPEED = 3000.f;
const float ANGULAR_VELOCITY = 100.f;


//-----------------------------------------------------------------------------------------------
ClientPlayer::ClientPlayer()
	: id( BAD_PLAYER_ID )
	, currentPos( Vector3f::Zero() )
	, currentVelocity( Vector3f::Zero() )
	, currentAcceleration( Vector3f::Zero() )
	, desiredPos( Vector3f::Zero() )
	, desiredVelocity( Vector3f::Zero() )
	, orientationAsDegrees( 0.f )
	, health( 1 )
	, score( 0 )
	, m_shipColor()
{
	//m_shipColor = Color( id );
	CreateShipVerts();
}


//-----------------------------------------------------------------------------------------------
void ClientPlayer::SetCurrentVelocityAndPositionFromMagnitude( float magnitude )
{
	Clock& clock = Clock::GetMasterClock();
	float speed = PLAYER_SPEED;

	float orientationRadians = PI_OVER_180 * orientationAsDegrees;
	Vector3f deltaVelocity;

	deltaVelocity = Vector3f( cos( orientationRadians ), sin( orientationRadians ), 0.f );
	deltaVelocity *= speed * magnitude;

	currentVelocity = deltaVelocity * static_cast< float >( clock.m_currentDeltaSeconds );

	currentPos += currentVelocity * static_cast< float >( clock.m_currentDeltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void ClientPlayer::SeekTarget()
{
	Clock& appClock = Clock::GetMasterClock();
	float deltaSeconds = static_cast< float >( appClock.m_currentDeltaSeconds );
	float speed = PLAYER_SPEED;

	Vector3f extrapolatedPosition = desiredPos;
	Vector3f extrapolatedVelocityDirection = desiredVelocity;

	extrapolatedVelocityDirection.Normalize();
	extrapolatedPosition += extrapolatedVelocityDirection * PLAYER_SPEED * deltaSeconds;

	Vector3f currentToExtrapolatedDirVector = extrapolatedPosition - currentPos;
	currentToExtrapolatedDirVector.Normalize();

	Vector3f currentVelocityDirection = currentVelocity;
	currentVelocityDirection.Normalize();

	Vector3f newVelocityDirection = currentToExtrapolatedDirVector;

	currentVelocity = newVelocityDirection * speed * deltaSeconds * GetFloatMin( 1.f, Vector3f::Distance( extrapolatedPosition, currentPos ) * 0.5f );
	currentPos += currentVelocity * deltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void ClientPlayer::Reset()
{
	currentPos = Vector3f::Zero();
	currentVelocity = Vector3f::Zero();
	currentAcceleration = Vector3f::Zero();
	desiredPos = Vector3f::Zero();
	desiredVelocity = Vector3f::Zero();
	orientationAsDegrees = 0.f;
	health = 1;
	score = 0;
}


//-----------------------------------------------------------------------------------------------
void ClientPlayer::CreateShipVerts()
{
	m_shipVertexArray.push_back( Vector2f( 7.5f, 0.f )   );
	m_shipVertexArray.push_back( Vector2f( -7.5f, 5.f )  );
	m_shipVertexArray.push_back( Vector2f( -5.f, 2.f )   );
	m_shipVertexArray.push_back( Vector2f( -5.f, -2.f )  );
	m_shipVertexArray.push_back( Vector2f( -7.5f, -5.f ) );
}


//-----------------------------------------------------------------------------------------------
void ClientPlayer::RotateForTimeStep( RotationDirection direction )
{
	Clock& clock = Clock::GetMasterClock();

	float deltaSeconds = static_cast< float >( clock.m_currentDeltaSeconds );

	if( direction == ClientPlayer::LEFT )
	{
		orientationAsDegrees += ANGULAR_VELOCITY * deltaSeconds;
	}
	else
	{
		orientationAsDegrees += -ANGULAR_VELOCITY * deltaSeconds;
	}
}


//-----------------------------------------------------------------------------------------------
void ClientPlayer::RenderShip() const
{
	Vector3f colorAsVec3 = m_shipColor.ToVector3fNormalizedAndFullyOpaque();

	OpenGLRenderer::PushMatrix();
	{

		glTranslatef( currentPos.x, currentPos.y, currentPos.z );
		glRotatef( orientationAsDegrees, 0.f, 0.f, 1.f );

		OpenGLRenderer::PushMatrix();
		{
			glScalef( 10.f, 10.f, 10.f );
			OpenGLRenderer::RenderSolidCube( Vector3f::Zero(), 0.f, Vector3f::Zero(), colorAsVec3 );
		}
		OpenGLRenderer::PopMatrix();

		OpenGLRenderer::RenderSolidCube( Vector3f( 6.75f, 0.f, 0.f ), 0.f, Vector3f::Zero(), colorAsVec3 * 0.5f, 3.5f );
		OpenGLRenderer::RenderSolidCube( Vector3f( 8.5f, 0.f, 0.f ), 0.f, Vector3f::Zero(), colorAsVec3 * 0.5f, 3.5f );

	}
	OpenGLRenderer::PopMatrix();
}