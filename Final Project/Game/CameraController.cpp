#include "CameraControllerInterface.hpp"

#include <string>

#include "Engine/Utilities/CommonUtilities.hpp"

#define UNUSED( x ) ( void )( x )
#define STATIC 

//-----------------------------------------------------------------------------------------------
CameraController::CameraController()
	: m_pose( Vector3f::Zero(), EulerAnglesf::Zero() )
{

}

//-----------------------------------------------------------------------------------------------
void CameraController::UpdateOrientationFromBlendedPosition( const Vector3f& blendedPosition )
{
	UNUSED( blendedPosition );
}

//-----------------------------------------------------------------------------------------------
STATIC CameraController* CameraController::CreateControllerFromBlueprint( CameraBlueprint* blueprint )
{
	CameraController* result = nullptr;

	if( blueprint == nullptr )
		return result;


	std::string controllerTypeAsString = blueprint->m_controllerName;
	ToLowerCaseString( controllerTypeAsString );

	if( controllerTypeAsString == "firstperson" )
	{
		result = new FirstPersonControllerStrategy( 10.f, 1.f );
		result->m_blueprint = blueprint;
	}
	else if( controllerTypeAsString == "physics" )
	{
		result = new PhysicsControllerStrategy();
		result->m_blueprint = blueprint;
	}

	return result;
}