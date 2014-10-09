#include "FirstPersonControllerStrategy.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"

#include <sstream>
#include <windows.h>
#define WIN32_LEAN_AND_MEAN

#define UNUSED( x ) ( void )( x )

//-----------------------------------------------------------------------------------------------
FirstPersonControllerStrategy::FirstPersonControllerStrategy( float distanceInFrontToLookAt, float focusAheadNormalizedAmount )
	: m_lookAtdistance( distanceInFrontToLookAt )
	, m_focusedOnPointAmount( focusAheadNormalizedAmount )
{

}

//-----------------------------------------------------------------------------------------------
void FirstPersonControllerStrategy::Initialize()
{

}

//-----------------------------------------------------------------------------------------------
void FirstPersonControllerStrategy::Update( Camera3D* currentAttachedTo, const Pose3D& currentAttachedToPose )
{
	UNUSED( currentAttachedTo );

	m_pose = currentAttachedToPose;
}

//-----------------------------------------------------------------------------------------------
Vector3f FirstPersonControllerStrategy::GetNonOffsetForwardViewVector()
{
	Vector3f forwardViewVector;

	float yawInRadians		= ConvertDegreesToRadians( m_pose.m_orientation.yawDegreesAboutZ );
	float pitchInRadians	= ConvertDegreesToRadians( m_pose.m_orientation.pitchDegreesAboutY );
	float cosinePitch		= cos( pitchInRadians );
	float sinPitch			= sin( pitchInRadians );

	Vector3f forward( cos( yawInRadians ), sin( yawInRadians ), 0.f );

	forwardViewVector.x = forward.x * cosinePitch;
	forwardViewVector.y = forward.y * cosinePitch;
	forwardViewVector.z = -sinPitch;

	return forwardViewVector;
}

//-----------------------------------------------------------------------------------------------
void FirstPersonControllerStrategy::UpdateOrientationFromBlendedPosition( const Vector3f& blendedPosition )
{
	Vector3f pointToLookAt = ( m_lookAtdistance * GetNonOffsetForwardViewVector() ) - ( m_pose.m_position - blendedPosition );

	float rotationZ = 0.f;
	float rotationY = 0.f;

	rotationZ = atan2( pointToLookAt.y, pointToLookAt.x );

	if( m_pose.m_orientation.yawDegreesAboutZ < 0.f && rotationZ > 0.f )
	{
		rotationZ = rotationZ - TWO_PI;
	}
	else if( m_pose.m_orientation.yawDegreesAboutZ > 0.f && rotationZ < 0.f )
	{
		rotationZ = TWO_PI + rotationZ;
	}

	if( pointToLookAt.x >= 0 )
	{
		rotationY = -atan2( pointToLookAt.z * cos( rotationZ ), pointToLookAt.x  );
	}
	else
	{
		rotationY = atan2( pointToLookAt.z * cos( rotationZ ), -pointToLookAt.x  );
	}

	rotationY = ConvertRadiansToDegrees( rotationY );
	rotationZ = ConvertRadiansToDegrees( rotationZ );

	EulerAnglesf offsetOrientation( EulerAnglesf::Zero() );

	offsetOrientation.pitchDegreesAboutY = rotationY;
	offsetOrientation.yawDegreesAboutZ   = rotationZ;

	offsetOrientation = m_pose.m_orientation - offsetOrientation;
	offsetOrientation = offsetOrientation * m_focusedOnPointAmount;

	m_pose.m_orientation += offsetOrientation;
}

//-----------------------------------------------------------------------------------------------
void FirstPersonControllerStrategy::SetFocusAheadStrength( float newFocusStrength )
{
	m_focusedOnPointAmount = newFocusStrength;
}

//-----------------------------------------------------------------------------------------------
float FirstPersonControllerStrategy::GetFocusAheadStrength() const
{
	return m_focusedOnPointAmount;
}