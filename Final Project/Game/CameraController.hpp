#ifndef CAMERA_CONTROLLER
#define CAMERA_CONTROLLER

#include "GameCommon.hpp"
#include "CameraBlueprint.hpp"

//-----------------------------------------------------------------------------------------------
class CameraController
{
public:

	CameraController();

	virtual void				Initialize() = 0;
	virtual void				Update( Camera3D* currentAttachedTo, const Pose3D& currentAttachedToPose ) = 0;
	virtual void				UpdateOrientationFromBlendedPosition( const Vector3f& blendedPosition );
	
	inline const Pose3D			GetPose() const;
	inline const Vector3f		GetPosition() const;
	inline const EulerAnglesf	GetOrientation() const;

	static CameraController*	CreateControllerFromBlueprint( CameraBlueprint* blueprint );

protected:

	CameraBlueprint* m_blueprint;
	Pose3D m_pose;
};

//-----------------------------------------------------------------------------------------------
inline const Pose3D CameraController::GetPose() const
{
	return m_pose;
}

//-----------------------------------------------------------------------------------------------
inline const Vector3f CameraController::GetPosition() const
{
	return m_pose.m_position;
}

//-----------------------------------------------------------------------------------------------
inline const EulerAnglesf CameraController::GetOrientation() const
{
	return m_pose.m_orientation;
}

#endif