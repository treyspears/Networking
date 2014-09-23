#define UNUSED(x) (void)(x);

#include "Light.hpp"

#include <string>
#include <sstream>

#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Primitives/Color.hpp"
#include "Engine/Utilities/Time.hpp"


struct LightMovementInitialPosition
{
	Light light;
	LightSystem::LightMovement movement;
	Vector3f initialPosition;
};

std::vector<LightMovementInitialPosition> LightSystem::m_lights;


int LightSystem::m_numberOfActivatedLights;
int LightSystem::m_positionUniformLocation[MAX_LIGHTS];
int LightSystem::m_colorAndBrightnessUniformLocation[MAX_LIGHTS];
int LightSystem::m_directionuniformLocation[MAX_LIGHTS];
int LightSystem::m_innerRadiusUniformLocation[MAX_LIGHTS];
int LightSystem::m_outerRadiusUniformLocation[MAX_LIGHTS];
int LightSystem::m_innerApertureDotUniformLocation[MAX_LIGHTS];
int LightSystem::m_outerApertureDotUniformLocation[MAX_LIGHTS];

int LightSystem::m_fractionAmbientLocation[MAX_LIGHTS];
int LightSystem::m_ignoreSpecularLocation[MAX_LIGHTS];
int LightSystem::m_ignorePositionLocation[MAX_LIGHTS];


//-----------------------------------------------------------------------------------------------
Light::Light(const Vector3f& position
	, const Vector4f& colorAndBrightness
	, const Vector3f& direction
	, float innerRadius
	, float outerRadius
	, float innerApertureDot
	, float outerApertureDot
	, float fractionAmbient
	, float ignoreSpecular
	, float ignorePosition)
	
	: m_position(position)
	, m_colorAndBrightness(colorAndBrightness)
	, m_direction(direction)
	, m_innerRadius(innerRadius)
	, m_outerRadius(outerRadius)
	, m_innerApertureDot(innerApertureDot)
	, m_outerApertureDot(outerApertureDot)
	, m_fractionAmbient(fractionAmbient)
	, m_ignoreSpecular(ignoreSpecular)
	, m_ignorePosition(ignorePosition)
{

}
//-----------------------------------------------------------------------------------------------

void LightSystem::Initialize(int shaderProgramID)
{
	LightMovementInitialPosition nullLight;

	for(int i = 0; i < MAX_LIGHTS; ++i)
	{
		m_lights.push_back(nullLight);
	}

	m_numberOfActivatedLights = 0;

	for(int i = 0; i < MAX_LIGHTS; ++i)
	{
		std::stringstream stream;
		stream << "lightList[";
		stream << i;
		stream << "]";
		std::string namePlusindex;
		stream >> namePlusindex;

		std::string tempString = namePlusindex;
		tempString += std::string(".position");
		m_positionUniformLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());

		tempString = namePlusindex;
		tempString += std::string(".colorAndBrightness");
		m_colorAndBrightnessUniformLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());

		tempString = namePlusindex;
		tempString += std::string(".direction");
		m_directionuniformLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());

		tempString = namePlusindex;
		tempString += std::string(".innerRadius");
		m_innerRadiusUniformLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());

		tempString = namePlusindex;
		tempString += std::string(".outerRadius");
		m_outerRadiusUniformLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());

		tempString = namePlusindex;
		tempString += std::string(".innerApertureDot");
		m_innerApertureDotUniformLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());

		tempString = namePlusindex;
		tempString += std::string(".outerApertureDot");
		m_outerApertureDotUniformLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());

		tempString = namePlusindex;
		tempString += std::string(".fractionAmbient");
		m_fractionAmbientLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());

		tempString = namePlusindex;
		tempString += std::string(".ignoreSpecular");
		m_ignoreSpecularLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());

		tempString = namePlusindex;
		tempString += std::string(".ignorePosition");
		m_ignorePositionLocation[i] = OpenGLRenderer::GetUniformLocation(shaderProgramID,tempString.c_str());
	}
}

//-----------------------------------------------------------------------------------------------
void LightSystem::PushGlobalPointLight(const Vector3f& position, const Color& color, float brightness, LightMovement movement
										, float fractionAmbient)
{
	if(m_numberOfActivatedLights == MAX_LIGHTS)
		return;

	Vector3f colorNormalized = color.ToVector3fNormalizedAndFullyOpaque();
	Vector4f colorAndBrightness(colorNormalized.x, colorNormalized.y, colorNormalized.z, brightness);

	LightMovementInitialPosition newLight;

	newLight.light = Light(position,colorAndBrightness, DEFAULT_LIGHT_DIRECTION, DEFAULT_LIGHT_INNER_RADIUS, DEFAULT_LIGHT_OUTER_RADIUS
		, DEFAULT_LIGHT_INNER_APERTURE_DOT, DEFAULT_LIGHT_OUTER_APERTURE_DOT, fractionAmbient, DEFAULT_LIGHT_IGNORE_SPECULAR
		, DEFAULT_LIGHT_IGNORE_POSITION);
	
	newLight.movement = movement;
	newLight.initialPosition = position;


	m_lights[m_numberOfActivatedLights] = newLight;

	++m_numberOfActivatedLights;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::PushLocalPointLight(const Vector3f& position, const Color& color, float brightness, float innerRadius, float outerRadius, LightMovement movement
									, float fractionAmbient)
{
	if(m_numberOfActivatedLights == MAX_LIGHTS)
		return;

	Vector3f colorNormalized = color.ToVector3fNormalizedAndFullyOpaque();
	Vector4f colorAndBrightness(colorNormalized.x, colorNormalized.y, colorNormalized.z, brightness);

	LightMovementInitialPosition newLight;

	newLight.light = Light(position,colorAndBrightness,DEFAULT_LIGHT_DIRECTION,innerRadius,outerRadius, DEFAULT_LIGHT_INNER_APERTURE_DOT
		, DEFAULT_LIGHT_OUTER_APERTURE_DOT, fractionAmbient, DEFAULT_LIGHT_IGNORE_SPECULAR, DEFAULT_LIGHT_IGNORE_POSITION);

	newLight.movement = movement;
	newLight.initialPosition = position;


	m_lights[m_numberOfActivatedLights] = newLight;

	++m_numberOfActivatedLights;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::PushGlobalSpotLight(const Vector3f& position, const Color& color, const Vector3f& direction, float brightness, float innerApertureDot,
	float outerApertureDot, LightMovement movement, float fractionAmbient)
{
	if(m_numberOfActivatedLights == MAX_LIGHTS)
		return;

	Vector3f colorNormalized = color.ToVector3fNormalizedAndFullyOpaque();
	Vector4f colorAndBrightness(colorNormalized.x, colorNormalized.y, colorNormalized.z, brightness);

	LightMovementInitialPosition newLight;

	newLight.light = Light(position, colorAndBrightness, direction, DEFAULT_LIGHT_INNER_RADIUS, DEFAULT_LIGHT_OUTER_RADIUS, innerApertureDot
		, outerApertureDot, fractionAmbient, DEFAULT_LIGHT_IGNORE_SPECULAR, DEFAULT_LIGHT_IGNORE_POSITION);

	newLight.movement = movement;
	newLight.initialPosition = position;


	m_lights[m_numberOfActivatedLights] = newLight;

	++m_numberOfActivatedLights;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::PushLocalSpotLight(const Vector3f& position, const Color& color, const Vector3f& direction, float brightness, float innerRadius, float outerRadius,
	float innerApertureDot, float outerApertureDot, LightMovement movement, float fractionAmbient)
{
	if(m_numberOfActivatedLights == MAX_LIGHTS)
		return;

	Vector3f colorNormalized = color.ToVector3fNormalizedAndFullyOpaque();
	Vector4f colorAndBrightness(colorNormalized.x, colorNormalized.y, colorNormalized.z, brightness);

	LightMovementInitialPosition newLight;

	newLight.light = Light(position, colorAndBrightness, direction, innerRadius, outerRadius, innerApertureDot, outerApertureDot, fractionAmbient
		, DEFAULT_LIGHT_IGNORE_SPECULAR, DEFAULT_LIGHT_IGNORE_POSITION);

	newLight.movement = movement;
	newLight.initialPosition = position;


	m_lights[m_numberOfActivatedLights] = newLight;

	++m_numberOfActivatedLights;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::PushGlobalAmbientLight(const Color& color, float brightness, LightMovement movement)
{
	if(m_numberOfActivatedLights == MAX_LIGHTS)
		return;

	Vector3f colorNormalized = color.ToVector3fNormalizedAndFullyOpaque();
	Vector4f colorAndBrightness(colorNormalized.x, colorNormalized.y, colorNormalized.z, brightness);

	LightMovementInitialPosition newLight;

	newLight.light = Light(DEFAULT_LIGHT_POSITION, colorAndBrightness, DEFAULT_LIGHT_DIRECTION, DEFAULT_LIGHT_INNER_RADIUS, DEFAULT_LIGHT_OUTER_RADIUS
		, DEFAULT_LIGHT_INNER_APERTURE_DOT, DEFAULT_LIGHT_OUTER_APERTURE_DOT, 1.f, 1.f, 1.f);

	newLight.movement = movement;
	newLight.initialPosition = DEFAULT_LIGHT_POSITION;


	m_lights[m_numberOfActivatedLights] = newLight;

	++m_numberOfActivatedLights;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::PushLocalAmbientLight(const Vector3f& position, const Color& color, float brightness, float innerRadius, float outerRadius, LightMovement movement)
{
	if(m_numberOfActivatedLights == MAX_LIGHTS)
		return;

	Vector3f colorNormalized = color.ToVector3fNormalizedAndFullyOpaque();
	Vector4f colorAndBrightness(colorNormalized.x, colorNormalized.y, colorNormalized.z, brightness);

	LightMovementInitialPosition newLight;

	newLight.light = Light(position,colorAndBrightness,DEFAULT_LIGHT_DIRECTION, innerRadius, outerRadius, DEFAULT_LIGHT_INNER_APERTURE_DOT
		, DEFAULT_LIGHT_OUTER_APERTURE_DOT, 1.f, DEFAULT_LIGHT_IGNORE_SPECULAR, DEFAULT_LIGHT_IGNORE_POSITION);

	newLight.movement = movement;
	newLight.initialPosition = position;


	m_lights[m_numberOfActivatedLights] = newLight;

	++m_numberOfActivatedLights;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::PushGlobalDirectionalLight(const Color& color, const Vector3f& direction, float brightness, LightMovement movement, float fractionAmbient)
{
	if(m_numberOfActivatedLights == MAX_LIGHTS)
		return;

	Vector3f colorNormalized = color.ToVector3fNormalizedAndFullyOpaque();
	Vector4f colorAndBrightness(colorNormalized.x, colorNormalized.y, colorNormalized.z, brightness);

	LightMovementInitialPosition newLight;

	newLight.light = Light(DEFAULT_LIGHT_POSITION,colorAndBrightness,direction,DEFAULT_LIGHT_INNER_RADIUS, DEFAULT_LIGHT_OUTER_RADIUS
		, DEFAULT_LIGHT_INNER_APERTURE_DOT, DEFAULT_LIGHT_OUTER_APERTURE_DOT, fractionAmbient, DEFAULT_LIGHT_IGNORE_SPECULAR, 1.f);

	newLight.movement = movement;
	newLight.initialPosition = DEFAULT_LIGHT_POSITION;

	m_lights[m_numberOfActivatedLights] = newLight;

	++m_numberOfActivatedLights;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::PopLight()
{
	if(m_numberOfActivatedLights == 0)
		return;

	--m_numberOfActivatedLights;

	m_lights[m_numberOfActivatedLights].light.m_colorAndBrightness = Vector4f(0.f,0.f,0.f,0.f);	
}

//-----------------------------------------------------------------------------------------------
void LightSystem::PopAllLights()
{
	while(m_numberOfActivatedLights > 0)
	{
		--m_numberOfActivatedLights;

		m_lights[m_numberOfActivatedLights].light.m_colorAndBrightness = Vector4f(0.f,0.f,0.f,0.f);			
	}
}

//-----------------------------------------------------------------------------------------------
void LightSystem::Update(float deltaSeconds, float movementRadius, float movementSpeed)
{
	UNUSED(deltaSeconds);

	Vector3f initialPosition;
	Vector3f lightRGB;
	Color color;

	float currentElapsedTime = (float)Time::GetCurrentTimeInSeconds();

	for(int i = 0; i < m_numberOfActivatedLights; ++i)
	{
		initialPosition = m_lights[i].initialPosition;

		if(m_lights[i].movement == FIGURE_EIGHT)
		{
			m_lights[i].light.m_position = Vector3f
				(
				initialPosition.x + movementRadius * sin(i + movementSpeed * currentElapsedTime),
				initialPosition.y + movementRadius * sin(i + 2.f * movementSpeed * currentElapsedTime),
				initialPosition.z
				);
		}
		else
		{
			m_lights[i].light.m_position = initialPosition;
		}

		lightRGB.x = m_lights[i].light.m_colorAndBrightness.x;
		lightRGB.y = m_lights[i].light.m_colorAndBrightness.y;
		lightRGB.z = m_lights[i].light.m_colorAndBrightness.z;
	
		color.FromVector3fNormalizedAndFullyOpaque(lightRGB);

		initialPosition = m_lights[i].light.m_position;

		if(!m_lights[i].light.m_ignorePosition)
			OpenGLRenderer::RenderDebugPoint(initialPosition, color);
	}
}

//-----------------------------------------------------------------------------------------------
void LightSystem::TranslateTopLight(const Vector3f& translation)
{
	if(m_numberOfActivatedLights <= 0)
		return;

	m_lights[m_numberOfActivatedLights - 1].initialPosition += translation;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::SetTopLightDirection( const Vector3f& direction )
{
	if(m_numberOfActivatedLights <= 0)
		return;

	m_lights[m_numberOfActivatedLights - 1].light.m_direction = direction;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::SetTopLightBrightness(  float brightness )
{
	if(m_numberOfActivatedLights <= 0)
		return;
	m_lights[m_numberOfActivatedLights - 1].light.m_colorAndBrightness.w = brightness;
}

//-----------------------------------------------------------------------------------------------
void LightSystem::Enable()
{
	for(int i = 0; i < MAX_LIGHTS; ++i)
	{
		OpenGLRenderer::Uniform3fv(m_positionUniformLocation[i], 1, &m_lights[i].light.m_position.x);
		OpenGLRenderer::Uniform4fv(m_colorAndBrightnessUniformLocation[i], 1, &m_lights[i].light.m_colorAndBrightness.x);
		OpenGLRenderer::Uniform3fv(m_directionuniformLocation[i], 1, &m_lights[i].light.m_direction.x);
		OpenGLRenderer::Uniform1f(m_innerRadiusUniformLocation[i], m_lights[i].light.m_innerRadius);
		OpenGLRenderer::Uniform1f(m_outerRadiusUniformLocation[i], m_lights[i].light.m_outerRadius);
		OpenGLRenderer::Uniform1f(m_innerApertureDotUniformLocation[i], m_lights[i].light.m_innerApertureDot);
		OpenGLRenderer::Uniform1f(m_outerApertureDotUniformLocation[i], m_lights[i].light.m_outerApertureDot);
		OpenGLRenderer::Uniform1f(m_fractionAmbientLocation[i], m_lights[i].light.m_fractionAmbient);
		OpenGLRenderer::Uniform1f(m_ignoreSpecularLocation[i], m_lights[i].light.m_ignoreSpecular);
		OpenGLRenderer::Uniform1f(m_ignorePositionLocation[i], m_lights[i].light.m_ignorePosition);
	}
}