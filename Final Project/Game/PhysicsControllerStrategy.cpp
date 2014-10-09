#include "PhysicsControllerStrategy.hpp"

#include <sstream>

#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Utilities/Clock.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"

#include "Game.hpp"

#define UNUSED( x ) ( void )( x )

//-----------------------------------------------------------------------------------------------
PhysicsControllerStrategy::PhysicsControllerStrategy()
	: m_velocity( Vector3f::Zero() )
{

}

//-----------------------------------------------------------------------------------------------
void PhysicsControllerStrategy::Initialize()
{

}

//-----------------------------------------------------------------------------------------------
void PhysicsControllerStrategy::Update( Camera3D* currentAttachedTo, const Pose3D& currentAttachedToPose )
{
	UNUSED( currentAttachedTo );

	static Pose3D previousAttachedToPose = currentAttachedToPose;
	static Pose3D controllerPose = currentAttachedToPose;

	const Pose3D deltaPose = currentAttachedToPose - previousAttachedToPose;
	const Clock& appClock = *Game::s_appClock;

	Vector3f acceleration = Vector3f::Zero();
	EulerAnglesf orientationFromOscillation = EulerAnglesf::Zero();
	EulerAnglesf torque = EulerAnglesf::Zero();

	float deltaSeconds = static_cast< float >( appClock.m_currentDeltaSeconds );

	//we only want the spring force to affect x and y movements from oscillation, not general movement.
	controllerPose.m_position.x = currentAttachedToPose.m_position.x;
	controllerPose.m_position.y = currentAttachedToPose.m_position.y;
	controllerPose.m_orientation.pitchDegreesAboutY = currentAttachedToPose.m_orientation.pitchDegreesAboutY;
	controllerPose.m_orientation.yawDegreesAboutZ = currentAttachedToPose.m_orientation.yawDegreesAboutZ;

	acceleration += GetOscillatingForce( currentAttachedToPose, deltaPose );
	orientationFromOscillation = orientationFromOscillation + GetOscillatingOrientation( deltaPose );	

	Vector3f controllerToAttached = controllerPose.m_position - currentAttachedToPose.m_position;
	acceleration += ComputeAndGetSpringForce( controllerToAttached );

	m_velocity += acceleration * deltaSeconds;
	controllerPose.m_position += m_velocity * deltaSeconds;

	controllerPose.m_orientation.rollDegreesAboutX = orientationFromOscillation.rollDegreesAboutX;

	m_pose = controllerPose;
	previousAttachedToPose = currentAttachedToPose;
}

//-----------------------------------------------------------------------------------------------
void PhysicsControllerStrategy::ApplyInstantaneosVelocity( const Vector3f& direction, float speed )
{
	m_velocity += direction * speed;
}

//-----------------------------------------------------------------------------------------------
void PhysicsControllerStrategy::ApplyStopForce()
{
	m_velocity += m_blueprint->m_onStopForceDirection * m_blueprint->m_onStopForceMagnitude;
}

//-----------------------------------------------------------------------------------------------
void PhysicsControllerStrategy::UpdateOnInput()
{

}

//-----------------------------------------------------------------------------------------------
Vector3f PhysicsControllerStrategy::GetOscillatingForce( const Pose3D& currentAttachedToPose, const Pose3D& deltaAttachedToPose )
{
	Vector3f acceleration = Vector3f::Zero();
	
	float oscillatingForceScale = GetOscillatingForceScale( deltaAttachedToPose.m_position );

	acceleration = ComputeOscillatingForce( currentAttachedToPose.m_orientation ) * oscillatingForceScale;

	return acceleration;
}

//-----------------------------------------------------------------------------------------------
Vector3f PhysicsControllerStrategy::ComputeOscillatingForce( const EulerAnglesf& currentAttachedToOrientation )
{
	const Clock& appClock = *Game::s_appClock;

	Vector3f sumForce = Vector3f::Zero();

	float yawInRadians = ConvertDegreesToRadians( currentAttachedToOrientation.yawDegreesAboutZ );

	const Vector3f UP = Vector3f( 0.f, 0.f, 1.f );
	Vector3f left( -sin( yawInRadians ), cos( yawInRadians ), 0.f );

	float verticalFrequency = TWO_PI * m_blueprint->m_verticalOscillationFrequency;
	float horizontalFrequency = TWO_PI * m_blueprint->m_horizontalOscillationFrequency;
	float verticalMagnitude = m_blueprint->m_verticalOscillationMagnitude;
	float horizontalMagnitude = m_blueprint->m_horizontalOscillationMagnitude;

	double verticalOscillatingValue = sin( verticalFrequency * appClock.m_currentElapsedSeconds );
	double horizontalOscillatingValue = sin( horizontalFrequency * appClock.m_currentElapsedSeconds );

	sumForce += UP * verticalMagnitude * static_cast< float >( verticalOscillatingValue );
	sumForce += left * horizontalMagnitude * static_cast< float >( horizontalOscillatingValue );

	return sumForce;
}

//-----------------------------------------------------------------------------------------------
EulerAnglesf PhysicsControllerStrategy::GetOscillatingOrientation( const Pose3D& deltaAttachedToPose )
{
	EulerAnglesf orientationAcceleration = EulerAnglesf::Zero();

	float oscillatingForceScale = GetOscillatingForceScale( deltaAttachedToPose.m_position );
	orientationAcceleration = ComputeOscillatingOrientation() * oscillatingForceScale;

	return orientationAcceleration;
}

//-----------------------------------------------------------------------------------------------
EulerAnglesf PhysicsControllerStrategy::ComputeOscillatingOrientation()
{
	EulerAnglesf oscillatedOrientation = EulerAnglesf::Zero();
	
	const Clock& appClock = *Game::s_appClock;

	float horizontalFrequency = TWO_PI * m_blueprint->m_horizontalOscillationFrequency;
	float maxRollDegrees = m_blueprint->m_maxRollDegreesOnDeltaOrientation;

	double newRollDegrees = sin( horizontalFrequency * appClock.m_currentElapsedSeconds );
	newRollDegrees *= maxRollDegrees;

	oscillatedOrientation.rollDegreesAboutX = static_cast< float >( newRollDegrees );

	return oscillatedOrientation;
}

//-----------------------------------------------------------------------------------------------
float PhysicsControllerStrategy::GetOscillatingForceScale( const Vector3f& deltaPosition )
{
	float result = 0.f;

	Vector3f deltaHorizontalMovement( deltaPosition.x, deltaPosition.y, 0.f );

	result = Vector3f::MagnitudeSquared( deltaHorizontalMovement );

	if( result > 0.f )
	{
		result = sqrt( result );
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
Vector3f PhysicsControllerStrategy::ComputeAndGetSpringForce( const Vector3f& differenceVector )
{
	Vector3f sumForce = Vector3f::Zero();

	float springTension = m_blueprint->m_springTension;
	float springDampingCoefficient = m_blueprint->m_springDampingCoefficient;

	sumForce = ( -( springTension * differenceVector ) - ( springDampingCoefficient * m_velocity ) );

	return sumForce;
}