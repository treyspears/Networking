#ifndef PHYSICS_CONTROLLER_STRATEGY
#define PHYSICS_CONTROLLER_STRATEGY

#include "CameraController.hpp"

//-----------------------------------------------------------------------------------------------
class PhysicsControllerStrategy : public CameraController
{

public:
	PhysicsControllerStrategy();

	virtual void Initialize();
	virtual void Update( Camera3D* currentAttachedTo, const Pose3D& currentAttachedToPose );

	void ApplyInstantaneosVelocity( const Vector3f& direction, float speed );
	void ApplyStopForce();

private:

	void			UpdateOnInput();
	Vector3f		GetOscillatingForce( const Pose3D& currentAttachedToPose, const Pose3D& deltaAttachedToPose );
	Vector3f		ComputeOscillatingForce( const EulerAnglesf& currentAttachedToOrientation );
	EulerAnglesf    GetOscillatingOrientation( const Pose3D& deltaAttachedToPose );
	EulerAnglesf	ComputeOscillatingOrientation();
	float			GetOscillatingForceScale( const Vector3f& deltaPosition );
	Vector3f		ComputeAndGetSpringForce( const Vector3f& differenceVector );

	Vector3f m_velocity;
};

#endif