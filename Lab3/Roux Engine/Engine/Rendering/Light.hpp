#ifndef LIGHT
#define LIGHT

#include <vector>

#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Primitives/Vector4.hpp"
#include "Engine/Primitives/Vector3.hpp"

class Color;

const Vector3f DEFAULT_LIGHT_POSITION(0.f,0.f,0.f);
const Vector4f DEFAULT_LIGHT_COLOR(0.f,0.f,0.f,0.f);
const Vector3f DEFAULT_LIGHT_DIRECTION(0.f,0.f,0.f);
const float DEFAULT_LIGHT_INNER_RADIUS = 999999.f - 1.f;
const float DEFAULT_LIGHT_OUTER_RADIUS = 999999.f;
const float DEFAULT_LIGHT_INNER_APERTURE_DOT = -2.f;
const float DEFAULT_LIGHT_OUTER_APERTURE_DOT = -3.f;
const float DEFAULT_LIGHT_FRACTION_AMBIENT = 0.f;
const float DEFAULT_LIGHT_IGNORE_SPECULAR = 0.f;
const float DEFAULT_LIGHT_IGNORE_POSITION = 0.f;


class Light
{
public:
	Light(const Vector3f& position = DEFAULT_LIGHT_POSITION
		, const Vector4f& colorAndBrightness = DEFAULT_LIGHT_COLOR
		, const Vector3f& direction = DEFAULT_LIGHT_DIRECTION
		, float innerRadius = DEFAULT_LIGHT_INNER_RADIUS
		, float outerRadius = DEFAULT_LIGHT_OUTER_RADIUS
		, float innerApertureDot = DEFAULT_LIGHT_INNER_APERTURE_DOT
		, float outerApertureDot = DEFAULT_LIGHT_OUTER_APERTURE_DOT
		, float fractionAmbient = DEFAULT_LIGHT_FRACTION_AMBIENT
		, float ignoreSpecular = DEFAULT_LIGHT_IGNORE_SPECULAR
		, float ignorePosition = DEFAULT_LIGHT_IGNORE_POSITION);

	Vector3f m_position;
	Vector4f m_colorAndBrightness;
	Vector3f m_direction;

	friend class LightSystem;

private:

	float m_innerRadius;
	float m_outerRadius;
	float m_innerApertureDot;
	float m_outerApertureDot;
	float m_fractionAmbient;
	float m_ignoreSpecular;
	float m_ignorePosition;
};

struct LightMovementInitialPosition;

//-----------------------------------------------------------------------------------------------
const int MAX_LIGHTS = 16;

class LightSystem
{
public:
	enum LightMovement
	{
		STATIONARY,
		FIGURE_EIGHT
	};

	static void Initialize(int shaderProgramID);

	static void PushGlobalPointLight(const Vector3f& position, const Color& color, float brightness, LightMovement movement
									, float fractionAmbient = 0.f);
	static void PushLocalPointLight(const Vector3f& position, const Color& color, float brightness, float innerRadius, float outerRadius, LightMovement movement
									, float fractionAmbient = 0.f);

	static void PushGlobalSpotLight(const Vector3f& position, const Color& color, const Vector3f& direction, float brightness, float innerApertureDot,
									float outerApertureDot, LightMovement movement, float fractionAmbient = 0.f);

	static void PushLocalSpotLight(const Vector3f& position, const Color& color, const Vector3f& direction, float brightness, float innerRadius, float outerRadius,
								   float innerApertureDot, float outerApertureDot, LightMovement movement, float fractionAmbient = 0.f);

	static void PushGlobalAmbientLight(const Color& color, float brightness, LightMovement movement);
	static void PushLocalAmbientLight(const Vector3f& position, const Color& color, float brightness, float innerRadius, float outerRadius, LightMovement movement);
	static void PushGlobalDirectionalLight(const Color& color, const Vector3f& direction, float brightness, LightMovement movement, float fractionAmbient = 0.f);

	static void PopLight();
	static void PopAllLights();

	static void Update(float deltaSeconds, float movementRadius = 1.f, float movementSpeed = 0.5f);
	static void TranslateTopLight(const Vector3f& translation);
	static void SetTopLightDirection( const Vector3f& direction );
	static void SetTopLightBrightness( float brightness );
	static void Enable();

private:

	static int m_positionUniformLocation[MAX_LIGHTS];
	static int m_colorAndBrightnessUniformLocation[MAX_LIGHTS];
	static int m_directionuniformLocation[MAX_LIGHTS];
	static int m_innerRadiusUniformLocation[MAX_LIGHTS];
	static int m_outerRadiusUniformLocation[MAX_LIGHTS];
	static int m_innerApertureDotUniformLocation[MAX_LIGHTS];
	static int m_outerApertureDotUniformLocation[MAX_LIGHTS];

	static int m_fractionAmbientLocation[MAX_LIGHTS];
	static int m_ignoreSpecularLocation[MAX_LIGHTS];
	static int m_ignorePositionLocation[MAX_LIGHTS];
	
	static std::vector<LightMovementInitialPosition> m_lights;
	static int m_numberOfActivatedLights;
};

#endif