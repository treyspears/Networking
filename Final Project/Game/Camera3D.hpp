#ifndef CAMERA3D
#define CAMERA3D

#include <vector>
#include <string>

#include "Engine/Primitives/Color.hpp"


#include "CameraControllerInterface.hpp"
#include "GameCommon.hpp"
#include "CameraBlueprint.hpp"

//Forward Declarations
//-----------------------------------------------------------------------------------------------
class ClientPlayer;
class Clock;
class NamedProperties;

//-----------------------------------------------------------------------------------------------
class Camera3D
{
public:

	Camera3D();

	void	Initialize();
	void	Update();

	inline void				 SetAttachedTo( ClientPlayer* attatchTo );
	inline ClientPlayer*	 GetAttachedTo() const;

	inline void				 SetController( CameraController* controller );
	inline CameraController* GetController() const;

	inline const std::string GetName() const;

	inline void		SetPose( const Vector3f& newPosition, const EulerAnglesf& newOrientation );
	inline void		SetPose( const Pose3D& newPose );
	inline void		SetPosition( const Vector3f& newPosition );
	inline void		SetOrientation( const EulerAnglesf& newOrientation );

	inline void		SetPositionInfluence( float influence );
	inline void		SetOrientationInfluence( float influence );

	inline const Pose3D			GetPose() const;
	inline const Vector3f		GetPosition() const;
	inline const EulerAnglesf	GetOrientation() const;

	inline const float			GetPositionInfluence() const;
	inline const float			GetOrientationInfluence() const;

	inline const Vector3f		GetViewDirectionVector() const;
	void						SetViewDirectionVector();

	void						StartNewInterpolatePositionInfluenceOverTime( float interpolationLength, float fromValue, float toValue );
	void						StartNewInterpolateOrientationInfluenceOverTime( float interpolationLength, float fromValue, float toValue );

	static Camera3D* CreateCameraFromBlueprintAndAttachTo( const std::string& blueprintName, ClientPlayer* attachedTo = nullptr
		, float initialPositionInfluence = 0.f, float initialOrientationInfluence = 0.f );

	static Vector3f GetBlendedPositionFromVectorOfCameras( const std::vector< Camera3D* > camerasToBlend );
	static EulerAnglesf GetBlendedOrientationFromVectorOfCameras( const std::vector< Camera3D* > camerasToBlend, const Vector3f& blendedPosition );
	static void DebugRenderVectorOfCameras( const std::vector< Camera3D* > camerasToRender, float fontSize, Vector3f& initialtextPosition );
	static void DebugRenderCamera( const Camera3D* cameraToRender, float fontSize, const Color& textColor, Vector3f& textPosition );

private:

	void			UpdateController( const Pose3D& currentPoseFromAttached );
	const Pose3D	GetPoseFromAttachedIfExists() const;
	void			SetPositionFromControllerIfExists();
	void			SetOrientationFromControllerIfExists();
	void			AdjustInfluences();
	void			UpdateOrientationFromBlendedPosition( const Vector3f& blendedPosition );
	void			OnPositionInterpolationEnd( NamedProperties& parameters );
	void			OnOrientationInterpolationEnd( NamedProperties& parameters );

	CameraBlueprint* m_blueprint;

	Pose3D m_pose;

	float m_positionInfluence;
	float m_orientationInfluence;

	Vector3f m_viewDirectionVector;

	ClientPlayer*			m_attachedTo;
	CameraController*		m_controller;
	Clock*					m_interpolationClock;

	

	struct InterpolationInfo
	{
		InterpolationInfo( float fromValue, float toValue )
			: eventOnFinishName("")
			, isInterpolating( false )
			, from( fromValue )
			, to( toValue )
		{}

		std::string eventOnFinishName;
		bool isInterpolating;
		float from;
		float to;
	};
	
	InterpolationInfo m_currentPositionInterpolationInfo;
	InterpolationInfo m_currentOrientationInterpolationInfo;
};

//-----------------------------------------------------------------------------------------------
inline void	Camera3D::SetAttachedTo( ClientPlayer* attatchedTo )
{
	m_attachedTo = attatchedTo;
}

//-----------------------------------------------------------------------------------------------
inline ClientPlayer* Camera3D::GetAttachedTo() const
{
	return m_attachedTo;
}

//-----------------------------------------------------------------------------------------------
inline void Camera3D::SetController( CameraController* controller )
{
	m_controller = controller;
}

//-----------------------------------------------------------------------------------------------
inline CameraController* Camera3D::GetController() const
{
	return m_controller;
}

//-----------------------------------------------------------------------------------------------
inline const std::string Camera3D::GetName() const
{
	std::string name = "None";
	
	if( m_blueprint )
	{
		name = m_blueprint->m_name;
	}

	return name;
}

//-----------------------------------------------------------------------------------------------
inline void	Camera3D::SetPose( const Vector3f& newPosition, const EulerAnglesf& newOrientation )
{
	m_pose.m_position = newPosition;
	m_pose.m_orientation = newOrientation;
}

//-----------------------------------------------------------------------------------------------
inline void Camera3D::SetPose( const Pose3D& newPose )
{
	m_pose = newPose;
}

//-----------------------------------------------------------------------------------------------
inline void	Camera3D::SetPosition( const Vector3f& newPosition )
{
	m_pose.m_position = newPosition;
}

//-----------------------------------------------------------------------------------------------
inline void	Camera3D::SetOrientation( const EulerAnglesf& newOrientation )
{
	m_pose.m_orientation = newOrientation;
}

//-----------------------------------------------------------------------------------------------
inline const Pose3D	Camera3D::GetPose() const
{
	return m_pose;
}

//-----------------------------------------------------------------------------------------------
inline const Vector3f Camera3D::GetPosition() const
{
	return m_pose.m_position;
}

//-----------------------------------------------------------------------------------------------
inline const EulerAnglesf Camera3D::GetOrientation() const
{
	return m_pose.m_orientation;
}

//-----------------------------------------------------------------------------------------------
inline const Vector3f Camera3D::GetViewDirectionVector() const
{
	return m_viewDirectionVector;
}

//-----------------------------------------------------------------------------------------------
inline void	Camera3D::SetPositionInfluence( float influence )
{
	m_positionInfluence = influence;
}

//-----------------------------------------------------------------------------------------------
inline void	Camera3D::SetOrientationInfluence( float influence )
{
	m_orientationInfluence = influence;
}

//-----------------------------------------------------------------------------------------------
inline const float Camera3D::GetPositionInfluence() const
{
	return m_positionInfluence;
}

//-----------------------------------------------------------------------------------------------
inline const float Camera3D::GetOrientationInfluence() const
{
	return m_orientationInfluence;
}

#endif