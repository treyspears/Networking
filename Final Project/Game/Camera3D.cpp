#include "Camera3D.hpp"

#include <sstream>

#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Utilities/Clock.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Utilities/NamedProperties.hpp"
#include "Engine/Utilities/EventSystem.hpp"

#include "Game.hpp"
#include "ClientPlayer.hpp"

#define UNUSED( x ) ( void )( x )
#define STATIC

//-----------------------------------------------------------------------------------------------
Camera3D::Camera3D()
	: m_blueprint( nullptr )
	, m_attachedTo( nullptr )
	, m_controller( nullptr )
	, m_pose( Vector3f::Zero(), EulerAnglesf::Zero() )
	, m_positionInfluence( 0.f )
	, m_orientationInfluence( 0.f )
	, m_viewDirectionVector( Vector3f::Zero() )
	, m_currentPositionInterpolationInfo( 0.f, 0.f )
	, m_currentOrientationInterpolationInfo( 0.f, 0.f )
{}

//-----------------------------------------------------------------------------------------------
void Camera3D::Initialize()
{ 
	EventSystem& eventSystem = EventSystem::GetInstance();

	m_interpolationClock = new Clock( *Game::s_appClock, 0.1, false, 1.f );
	
	std::string& positionInterpolationEventName = m_currentPositionInterpolationInfo.eventOnFinishName;

	positionInterpolationEventName = GetName();
	positionInterpolationEventName += " Position Interpolation Alarm";

	eventSystem.RegisterEventWithCallbackAndObject( positionInterpolationEventName, &Camera3D::OnPositionInterpolationEnd, this );

	std::string& orientationInterpolationEventName = m_currentOrientationInterpolationInfo.eventOnFinishName;

	orientationInterpolationEventName = GetName();
	orientationInterpolationEventName += " Orientation Interpolation Alarm";

	eventSystem.RegisterEventWithCallbackAndObject( orientationInterpolationEventName, &Camera3D::OnOrientationInterpolationEnd, this );

	if( m_controller )
	{
		m_controller->Initialize();
	}
}

//-----------------------------------------------------------------------------------------------
void Camera3D::Update()
{
	const Pose3D currentPoseFromAttached = GetPoseFromAttachedIfExists();

	UpdateController( currentPoseFromAttached );
	SetPositionFromControllerIfExists();
	SetOrientationFromControllerIfExists();
	SetViewDirectionVector();
	AdjustInfluences();
}

//-----------------------------------------------------------------------------------------------
void Camera3D::UpdateController( const Pose3D& currentPoseFromAttached )
{
	if( m_controller )
	{
		m_controller->Update( this, currentPoseFromAttached );
	}
}

//-----------------------------------------------------------------------------------------------
void Camera3D::SetViewDirectionVector()
{
	//const Pose3D& poseWithOffset = GetPoseWithOffset();

	float yawInDegrees = ConvertDegreesToRadians( m_pose.m_orientation.yawDegreesAboutZ );
	float pitchInDegrees = ConvertDegreesToRadians( m_pose.m_orientation.pitchDegreesAboutY );
	Vector3f forward( cos( yawInDegrees ), sin( yawInDegrees ), 0.f );

	Vector3f horiztonalMovementDirection = Vector3f::Zero();
	float cosinePitch = cos( pitchInDegrees );

	m_viewDirectionVector.x = forward.x * cosinePitch;
	m_viewDirectionVector.y = forward.y * cosinePitch;
	m_viewDirectionVector.z = -sin( pitchInDegrees );
}

//-----------------------------------------------------------------------------------------------
void Camera3D::StartNewInterpolatePositionInfluenceOverTime( float interpolationLength, float fromValue, float toValue )
{
	m_currentPositionInterpolationInfo.isInterpolating = true;
	m_currentPositionInterpolationInfo.from = fromValue;
	m_currentPositionInterpolationInfo.to = toValue;

	NamedProperties params = NamedProperties::EmptyProperties();
	m_interpolationClock->AddAlarmWithLengthAndParameters( m_currentPositionInterpolationInfo.eventOnFinishName, interpolationLength, params );
}

//-----------------------------------------------------------------------------------------------
void Camera3D::StartNewInterpolateOrientationInfluenceOverTime( float interpolationLength, float fromValue, float toValue )
{
	m_currentOrientationInterpolationInfo.isInterpolating = true;
	m_currentOrientationInterpolationInfo.from = fromValue;
	m_currentOrientationInterpolationInfo.to = toValue;

	NamedProperties params = NamedProperties::EmptyProperties();
	m_interpolationClock->AddAlarmWithLengthAndParameters( m_currentOrientationInterpolationInfo.eventOnFinishName, interpolationLength, params );
}

//-----------------------------------------------------------------------------------------------
const Pose3D Camera3D::GetPoseFromAttachedIfExists() const
{
	Pose3D resultantPose = m_pose;

	if( m_attachedTo )
	{
		resultantPose.m_position = m_attachedTo->currentPos;
		resultantPose.m_orientation = EulerAnglesf( m_attachedTo->orientationAsDegrees, 0.f, 0.f );
	}

	return resultantPose;
}

//-----------------------------------------------------------------------------------------------
void Camera3D::SetPositionFromControllerIfExists()
{
	if( m_controller )
	{
		m_pose.m_position = m_controller->GetPosition();
	}
}

//-----------------------------------------------------------------------------------------------
void Camera3D::SetOrientationFromControllerIfExists()
{
	if( m_controller )
	{
		m_pose.m_orientation = m_controller->GetOrientation();
	}
}


//-----------------------------------------------------------------------------------------------
void Camera3D::AdjustInfluences()
{
	float normalizedElapsedTime = 0.f;
	if( m_currentPositionInterpolationInfo.isInterpolating )
	{
		normalizedElapsedTime = m_interpolationClock->GetPercentageElapsedForAlarm( m_currentPositionInterpolationInfo.eventOnFinishName );
		m_positionInfluence = ( ( 1.f - normalizedElapsedTime ) * m_currentPositionInterpolationInfo.from ) + ( normalizedElapsedTime * m_currentPositionInterpolationInfo.to );
	}

	if( m_currentOrientationInterpolationInfo.isInterpolating )
	{
		normalizedElapsedTime = m_interpolationClock->GetPercentageElapsedForAlarm( m_currentOrientationInterpolationInfo.eventOnFinishName );
		m_orientationInfluence = ( ( 1.f - normalizedElapsedTime ) * m_currentOrientationInterpolationInfo.from ) + ( normalizedElapsedTime * m_currentOrientationInterpolationInfo.to );
	}
}

//-----------------------------------------------------------------------------------------------
void Camera3D::UpdateOrientationFromBlendedPosition( const Vector3f& blendedPosition )
{
	if( m_controller )
	{
		m_controller->UpdateOrientationFromBlendedPosition( blendedPosition );
		m_pose.m_orientation = m_controller->GetOrientation();
	}
	//NEEDS TO MAYBE DO STUFF HERE.
}

//-----------------------------------------------------------------------------------------------
void Camera3D::OnPositionInterpolationEnd( NamedProperties& parameters )
{
	UNUSED( parameters );

	m_currentPositionInterpolationInfo.isInterpolating = false;
	m_positionInfluence = m_currentPositionInterpolationInfo.to;
}

void Camera3D::OnOrientationInterpolationEnd( NamedProperties& parameters )
{
	UNUSED( parameters );

	m_currentOrientationInterpolationInfo.isInterpolating = false;
	m_orientationInfluence = m_currentOrientationInterpolationInfo.to;
}


//-----------------------------------------------------------------------------------------------
STATIC Camera3D* Camera3D::CreateCameraFromBlueprintAndAttachTo( const std::string& blueprintName, ClientPlayer* attachedTo
	, float initialPositionInfluence, float initialOrientationInfluence )
{
	Camera3D* newCamera = nullptr;
	CameraBlueprint* blueprint = CameraBlueprint::GetCameraBlueprint( blueprintName );

	if( !blueprint )
		return nullptr;

	CameraController* controller = CameraController::CreateControllerFromBlueprint( blueprint );

	if( !controller )
		return nullptr;

	newCamera = new Camera3D();

	newCamera->m_blueprint = blueprint;
	newCamera->m_attachedTo = attachedTo;
	newCamera->m_controller = controller;
	newCamera->m_positionInfluence = initialPositionInfluence;
	newCamera->m_orientationInfluence = initialOrientationInfluence;

	return newCamera;
}

//-----------------------------------------------------------------------------------------------
STATIC Vector3f Camera3D::GetBlendedPositionFromVectorOfCameras( const std::vector< Camera3D* > camerasToBlend )
{
	Vector3f blendedPosition = Vector3f::Zero();

	Camera3D* camToBlend = nullptr;
	float sumPositionInfluence = 0.f;
	
	std::vector< Camera3D* >::const_iterator cameraIter = camerasToBlend.begin();
	
	for( ; cameraIter != camerasToBlend.end(); ++cameraIter )
	{
		camToBlend = *cameraIter;
		sumPositionInfluence += camToBlend->GetPositionInfluence();
	}

	sumPositionInfluence = 1.f / sumPositionInfluence;

	float currentCameraPositionInfluence = 0.f;
	cameraIter = camerasToBlend.begin();

	for( ; cameraIter != camerasToBlend.end(); ++cameraIter )
	{
		camToBlend = *cameraIter;
		currentCameraPositionInfluence = camToBlend->GetPositionInfluence() * sumPositionInfluence;
		blendedPosition += camToBlend->GetPosition() * currentCameraPositionInfluence;
	}

	return blendedPosition;
}

//-----------------------------------------------------------------------------------------------
STATIC EulerAnglesf Camera3D::GetBlendedOrientationFromVectorOfCameras( const std::vector< Camera3D* > camerasToBlend, const Vector3f& blendedPosition )
{
	EulerAnglesf blendedOrientation = EulerAnglesf::Zero();

	Camera3D* camToBlend = nullptr;
	float sumOrientationInfluence = 0.f;

	std::vector< Camera3D* >::const_iterator cameraIter = camerasToBlend.begin();

	for( ; cameraIter != camerasToBlend.end(); ++cameraIter )
	{
		camToBlend = *cameraIter;
		sumOrientationInfluence += camToBlend->GetOrientationInfluence();
	}

	sumOrientationInfluence = 1.f / sumOrientationInfluence;

	float currentCameraOrientationInfluence = 0.f;
	cameraIter = camerasToBlend.begin();

	for( ; cameraIter != camerasToBlend.end(); ++cameraIter )
	{
		camToBlend = *cameraIter;

		camToBlend->UpdateOrientationFromBlendedPosition( blendedPosition );
		currentCameraOrientationInfluence = camToBlend->GetOrientationInfluence() * sumOrientationInfluence;
		blendedOrientation += camToBlend->GetOrientation() * currentCameraOrientationInfluence;
	}

	return blendedOrientation;
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
void RenderCameraPosition( const Vector3f& cameraPosition, float fontSize, const Color& textColor, Vector3f& textPosition, const std::string& prependedString )
{
	const Vector3f offsetVector( 0.f, fontSize, 0.f );

	BitmapFont* font = BitmapFont::CreateOrGetFont( "Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml" );
	std::ostringstream outputStringStream;
	std::string stringToRender;

	outputStringStream.str( "" );
	outputStringStream << prependedString;
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition, textColor.ToVector4fNormalized(), true );

	textPosition -= offsetVector;
	outputStringStream.str( "" );
	outputStringStream << "        X: " << cameraPosition.x;
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize * 0.75f, textPosition, textColor.ToVector4fNormalized(), true );

	textPosition -= offsetVector;
	outputStringStream.str( "" );
	outputStringStream << "        Y: " << cameraPosition.y;
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize * 0.75f, textPosition, textColor.ToVector4fNormalized(), true );

	textPosition -= offsetVector;
	outputStringStream.str( "" );
	outputStringStream << "        Z: " << cameraPosition.z;
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize * 0.75f, textPosition, textColor.ToVector4fNormalized(), true );
}

//-----------------------------------------------------------------------------------------------
void RenderCameraOrientation( const EulerAnglesf& cameraOrientation, float fontSize, const Color& textColor, Vector3f& textPosition, const std::string& prependedString )
{
	const Vector3f offsetVector( 0.f, fontSize, 0.f );

	BitmapFont* font = BitmapFont::CreateOrGetFont( "Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml" );
	std::ostringstream outputStringStream;
	std::string stringToRender;

	outputStringStream.str( "" );
	outputStringStream << prependedString;
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition, textColor.ToVector4fNormalized(), true );

	textPosition -= offsetVector;
	outputStringStream.str( "" );
	outputStringStream << "        Roll: " << cameraOrientation.rollDegreesAboutX;
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize * 0.75f, textPosition, textColor.ToVector4fNormalized(), true );

	textPosition -= offsetVector;
	outputStringStream.str( "" );
	outputStringStream << "        Pitch: " << cameraOrientation.pitchDegreesAboutY;
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize * 0.75f, textPosition, textColor.ToVector4fNormalized(), true );

	textPosition -= offsetVector;
	outputStringStream.str( "" );
	outputStringStream << "        Yaw: " << cameraOrientation.yawDegreesAboutZ;
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize * 0.75f, textPosition, textColor.ToVector4fNormalized(), true );
}

//-----------------------------------------------------------------------------------------------
void RenderFloatValue( float value, float fontSize, const Color& textColor, Vector3f& textPosition, const std::string& prependedString )
{
	const Vector3f offsetVector( 0.f, fontSize, 0.f );

	BitmapFont* font = BitmapFont::CreateOrGetFont( "Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml" );
	std::ostringstream outputStringStream;
	std::string stringToRender;

	outputStringStream.str( "" );
	outputStringStream << prependedString;
	outputStringStream << value;
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition, textColor.ToVector4fNormalized(), true );
}

//-----------------------------------------------------------------------------------------------
STATIC void Camera3D::DebugRenderVectorOfCameras( const std::vector< Camera3D* > camerasToRender, float fontSize, Vector3f& initialtextPosition )
{
	const float SPACING_OFFSET = 2.f;
	/*LAZY*/ const int CAMERAS_PER_LINE = 3;
	/*LAZY*/ const float OFFSET_X = 256.f;
	/*LAZY*/ const float STARTING_Y = 576.f;
	
	/*LAZY*/ int cameraCount = 0;
	uchar colorAsInt = 0;
	Color textColor = Color( Red );

	std::vector< Camera3D* >::const_iterator cameraIter = camerasToRender.begin();

	for( ; cameraIter != camerasToRender.end(); ++cameraIter )
	{
		textColor.FromID( colorAsInt );

		DebugRenderCamera( *cameraIter, fontSize, textColor, initialtextPosition );

		initialtextPosition -= Vector3f( 0.f, SPACING_OFFSET * fontSize, 0.f );
		++colorAsInt;

		++cameraCount;

		if( cameraCount == CAMERAS_PER_LINE )
		{
			initialtextPosition += Vector3f( OFFSET_X, 0.f, 0.f );
			initialtextPosition.y = STARTING_Y - fontSize;
			cameraCount = 0;
		}
	}
}

//-----------------------------------------------------------------------------------------------
STATIC void Camera3D::DebugRenderCamera( const Camera3D* cameraToRender, float fontSize, const Color& textColor, Vector3f& textPosition )
{
	const Vector3f offsetVector( 0.f, fontSize, 0.f );

	const Vector3f& cameraPosition = cameraToRender->GetPosition();
	const EulerAnglesf& cameraOrientation = cameraToRender->GetOrientation();

	float cameraPositionInfluence = cameraToRender->GetPositionInfluence();
	float cameraOrientationInfluence = cameraToRender->GetOrientationInfluence();

	BitmapFont* font = BitmapFont::CreateOrGetFont( "Data/Fonts/MainFont_EN_00.png", "Data/Fonts/MainFont_EN.FontDef.xml" );

	std::ostringstream outputStringStream;
	std::string stringToRender;

	outputStringStream << cameraToRender->GetName();
	outputStringStream << ": ";
	stringToRender = outputStringStream.str();
	OpenGLRenderer::RenderText( stringToRender, font, fontSize, textPosition, textColor.ToVector4fNormalized(), true );

	textPosition -= offsetVector;
	RenderCameraPosition( cameraPosition, fontSize, textColor, textPosition, "    Position: " );

	textPosition -= offsetVector;
	RenderFloatValue( cameraPositionInfluence, fontSize, textColor, textPosition, "    Position Influence: " );

	textPosition -= offsetVector;
	RenderCameraOrientation( cameraOrientation, fontSize, textColor, textPosition, "    Orientation: " );

	textPosition -= offsetVector;
	RenderFloatValue( cameraOrientationInfluence, fontSize, textColor, textPosition, "    Orientation Influence: " );
}
