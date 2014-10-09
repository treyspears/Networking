#include "Stimuli.hpp"

#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"

#define STATIC

//-----------------------------------------------------------------------------------------------
Stimuli::Stimuli( TypeOfStimuli stimuliType, const Vector3f& position, const Color& debugRenderColor )
	: m_type( stimuliType )
	, m_position( position )
	, m_debugRenderColor( debugRenderColor )
{

}

//-----------------------------------------------------------------------------------------------
void Stimuli::Render() const
{
	OpenGLRenderer::RenderDebugPoint( m_position, m_debugRenderColor, OpenGLRenderer::NO_DEPTH_TEST );
}

//-----------------------------------------------------------------------------------------------
void Stimuli::Update()
{

}

//-----------------------------------------------------------------------------------------------
STATIC Stimuli::TypeOfStimuli Stimuli::GetTypeForStimuliName( const std::string& stimuliName )
{
	if( stimuliName == "zebra" )		return STIMULI_ZEBRA;
	else if( stimuliName == "lion" )	return STIMULI_LION;
	else if( stimuliName == "sight" )	return STIMULI_SIGHT;
	else if( stimuliName == "sound" )	return STIMULI_SOUND;
	else if( stimuliName == "goal" )	return STIMULI_GOAL;
	else if( stimuliName == "all" )		return STIMULI_ALL;
	else								return STIMULI_NONE;
}