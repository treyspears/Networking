#ifndef FIRST_PERSON_CONTROLLER_STRATEGY
#define FIRST_PERSON_CONTROLLER_STRATEGY

#include "CameraController.hpp"

//-----------------------------------------------------------------------------------------------
class FirstPersonControllerStrategy : public CameraController
{
public:

	FirstPersonControllerStrategy( float distanceInFrontToLookAt, float focusAheadNormalizedAmount = 1.f );

	virtual void Initialize();
	virtual void Update( Camera3D* currentAttachedTo, const Pose3D& currentAttachedToPose );
	virtual void UpdateOrientationFromBlendedPosition( const Vector3f& blendedPosition );

	void	SetFocusAheadStrength( float newFocusStrength );
	float	GetFocusAheadStrength() const;

private:

	Vector3f GetNonOffsetForwardViewVector( );

	//this could/should be in the blueprint.
	float m_lookAtdistance;
	float m_focusedOnPointAmount;
};

#endif