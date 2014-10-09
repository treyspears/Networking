#ifndef CAMERA_BLUEPRINT
#define CAMERA_BLUEPRINT

#include <string>
#include <map>

#include "Engine/Utilities/XMLUtilities.hpp"

#include "GameCommon.hpp"
#include "Stimuli.hpp"

//-----------------------------------------------------------------------------------------------
class CameraBlueprint
{
public:

	CameraBlueprint( const pugi::xml_node& blueprintNode );

	static void				ReloadCameraBlueprints( const pugi::xml_node& rootNode );
	static void				CreateAndStoreCameraBlueprints( const pugi::xml_node& rootNode );
	static CameraBlueprint* GetCameraBlueprint( const std::string& entityName );


	friend class Camera3D;
	friend class CameraController;
	friend class PhysicsControllerStrategy;
	friend class FirstPersonControllerStrategy;

private:

	static void	ReloadSingleCameraBlueprint( const pugi::xml_node& blueprintNode );

	bool		ValidateCameraBlueprint( const pugi::xml_node& blueprintNode );
	bool		ValidateCameraPhysics( const pugi::xml_node& blueprintNode );
	bool		ValidateCameraLookAt( const pugi::xml_node& blueprintNode );

	std::string m_name;
	std::string m_controllerName;

	float	 m_springTension;
	float	 m_springDampingCoefficient;

	float	 m_verticalOscillationFrequency;
	float	 m_horizontalOscillationFrequency;
	float	 m_verticalOscillationMagnitude;
	float	 m_horizontalOscillationMagnitude;

	Vector3f m_onStopForceDirection;
	float	 m_onStopForceMagnitude;

	float	 m_maxRollDegreesOnDeltaOrientation;
	float	 m_senseRadius;
	float	 m_stimuliStrength;
	Stimuli::TypeOfStimuli  m_stimuliCameraIsInterestedIn;


	static   std::map< std::string, CameraBlueprint* > s_cameraBlueprints;

};
#endif