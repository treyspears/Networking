#include "CameraBlueprint.hpp"

#include "Engine/Utilities/ErrorWarningAssert.hpp"

#define STATIC

std::map< std::string, CameraBlueprint* > CameraBlueprint::s_cameraBlueprints;

//-----------------------------------------------------------------------------------------------
CameraBlueprint::CameraBlueprint( const pugi::xml_node& blueprintNode )
	: m_name( "" )
	, m_controllerName( "" )
	, m_springTension( 0.f )
	, m_springDampingCoefficient( 0.f )
	, m_verticalOscillationFrequency( 0.f )
	, m_horizontalOscillationFrequency( 0.f )
	, m_verticalOscillationMagnitude( 0.f )
	, m_horizontalOscillationMagnitude( 0.f )
	, m_onStopForceDirection( 0.f, 0.f, 0.f )
	, m_onStopForceMagnitude( 0.f )
	, m_maxRollDegreesOnDeltaOrientation( 0.f )
	, m_senseRadius( 0.f )
	, m_stimuliStrength( 0.f )
	, m_stimuliCameraIsInterestedIn( Stimuli::STIMULI_NONE )
{
	bool isBlueprintValid = true;

	isBlueprintValid = ValidateCameraBlueprint( blueprintNode );

	FATAL_ASSERTION( isBlueprintValid == true, "Camera blueprint definition wasn't valid." );

	std::map< std::string, CameraBlueprint* >::iterator foundIter;

	foundIter = s_cameraBlueprints.find( m_name );
	if( foundIter != s_cameraBlueprints.end() )
	{
		std::string errorMessage = m_name;
		errorMessage += " Camera blueprint definition was already found.";
		FATAL_ERROR( errorMessage );
	}

	s_cameraBlueprints[ m_name ] = this;
}

//-----------------------------------------------------------------------------------------------
STATIC void CameraBlueprint::CreateAndStoreCameraBlueprints( const pugi::xml_node& rootNode )
{
	for( pugi::xml_node blueprintNode = rootNode.child( "CameraDefinition" ); blueprintNode; blueprintNode = blueprintNode.next_sibling( "CameraDefinition" ) )
	{
		new CameraBlueprint( blueprintNode );
	}
}

//-----------------------------------------------------------------------------------------------
STATIC CameraBlueprint* CameraBlueprint::GetCameraBlueprint( const std::string& bluePrintName )
{
	CameraBlueprint* result = nullptr;
	auto foundIter = s_cameraBlueprints.find( bluePrintName );

	if( foundIter != s_cameraBlueprints.end() )
	{
		result = foundIter->second;
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
STATIC void CameraBlueprint::ReloadCameraBlueprints(  const pugi::xml_node& rootNode )
{
	for( pugi::xml_node blueprintNode = rootNode.child( "CameraDefinition" ); blueprintNode; blueprintNode = blueprintNode.next_sibling( "CameraDefinition" ) )
	{
		ReloadSingleCameraBlueprint( blueprintNode );
	}
}

//-----------------------------------------------------------------------------------------------
void CameraBlueprint::ReloadSingleCameraBlueprint( const pugi::xml_node& blueprintNode )
{
	bool isBlueprintValid = true;

	std::string blueprintName;

	XMLDocumentParser& parser = XMLDocumentParser::GetInstance();

	parser.ValidateXMLAttributes( blueprintNode, "name", "controllerType" );
	blueprintName = parser.GetStringXMLAttribute( blueprintNode, "name", "" );

	std::map< std::string, CameraBlueprint* >::iterator foundIter;

	foundIter = s_cameraBlueprints.find( blueprintName );
	if( foundIter == s_cameraBlueprints.end() )
	{
		std::string errorMessage = blueprintName;
		errorMessage += " Camera blueprint definition wasn't found.";
		FATAL_ERROR( errorMessage );
	}

	CameraBlueprint* blueprintToReload = foundIter->second;

	isBlueprintValid = blueprintToReload->ValidateCameraBlueprint( blueprintNode );

	FATAL_ASSERTION( isBlueprintValid == true, "Camera blueprint definition wasn't valid." );
}

//-----------------------------------------------------------------------------------------------
bool CameraBlueprint::ValidateCameraBlueprint( const pugi::xml_node& blueprintNode )
{
	bool result = true;

	XMLDocumentParser& parser = XMLDocumentParser::GetInstance();

	parser.ValidateXMLAttributes( blueprintNode, "name", "controllerType" );
	parser.ValidateXMLChildElements( blueprintNode, "", "Physics, LookAt" );

	m_name = parser.GetStringXMLAttribute( blueprintNode, "name", "" );
	if( m_name == "" )
		result = false;

	m_controllerName = parser.GetStringXMLAttribute( blueprintNode, "controllerType", "none" );


	result &= ValidateCameraPhysics( blueprintNode.child( "Physics" ) );
	result &= ValidateCameraLookAt( blueprintNode.child( "LookAt" ) );

	return result;
}

//-----------------------------------------------------------------------------------------------
bool CameraBlueprint::ValidateCameraPhysics( const pugi::xml_node& blueprintNode )
{
	bool result = true;

	if( blueprintNode.empty() )
	{
		return result;
	}

	XMLDocumentParser& parser = XMLDocumentParser::GetInstance();

	parser.ValidateXMLAttributes( blueprintNode, "", "springTension, springDampingCoefficient" );
	parser.ValidateXMLChildElements( blueprintNode, "", "Movement" );

	m_springTension = parser.GetFloatXMLAttribute( blueprintNode, "springTension", 0.f );
	m_springDampingCoefficient = parser.GetFloatXMLAttribute( blueprintNode, "springDampingCoefficient", 0.f );

	pugi::xml_node childNode = blueprintNode.child( "Movement" );

	if( childNode.empty() )
	{
		return result;
	}

	std::string optionalMovementParameters = "verticalOscillationFrequency, horizontalOscillationFrequency";
	optionalMovementParameters += ", verticalOscillationMagnitude, horizontalOscillationMagnitude";
	optionalMovementParameters += ", onStopForceDirection, onStopForceMagnitude, maxRollDegreesOnDeltaOrientation";

	parser.ValidateXMLAttributes( childNode, "", optionalMovementParameters );

	m_verticalOscillationFrequency = parser.GetFloatXMLAttribute( childNode, "verticalOscillationFrequency", 0.f );
	m_horizontalOscillationFrequency = parser.GetFloatXMLAttribute( childNode, "horizontalOscillationFrequency", 0.f );
	m_verticalOscillationMagnitude = parser.GetFloatXMLAttribute( childNode, "verticalOscillationMagnitude", 0.f );
	m_horizontalOscillationMagnitude = parser.GetFloatXMLAttribute( childNode, "horizontalOscillationMagnitude", 0.f );
	m_onStopForceDirection = parser.GetVector3XMLAttribute( childNode, "onStopForceDirection", Vector3f::Zero() );
	m_onStopForceMagnitude = parser.GetFloatXMLAttribute( childNode, "onStopForceMagnitude", 0.f );
	m_maxRollDegreesOnDeltaOrientation = parser.GetFloatXMLAttribute( childNode, "maxRollDegreesOnDeltaOrientation", 0.f );

	return result;
}

//-----------------------------------------------------------------------------------------------
bool CameraBlueprint::ValidateCameraLookAt( const pugi::xml_node& blueprintNode )
{
	bool result = true;

	if( blueprintNode.empty() )
	{
		return result;
	}

	XMLDocumentParser& parser = XMLDocumentParser::GetInstance();

	parser.ValidateXMLAttributes( blueprintNode, "", "stimuliType, radius, strength" );

	std::string stimuliTypeAsString = parser.GetStringXMLAttribute( blueprintNode, "stimuliType", "none" );
	ToLowerCaseString( stimuliTypeAsString );
	m_stimuliCameraIsInterestedIn = Stimuli::GetTypeForStimuliName( stimuliTypeAsString );

	m_senseRadius = parser.GetFloatXMLAttribute( blueprintNode, "radius", 0.f );
	m_stimuliStrength = parser.GetFloatXMLAttribute( blueprintNode, "strength", 0.f );

	return result;
}