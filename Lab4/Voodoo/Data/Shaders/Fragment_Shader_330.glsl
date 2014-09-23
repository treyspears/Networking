#version 330
//Fragment shader

// VARIABLES
const float MAX_GLOSSINESS_POWER = 32.0;
const int MAX_LIGHTS = 16;
const float MAX_FLOAT_VALUE = 1000000000000000000.0;
const float MAX_FRAGMENT_HEIGHT = 0.03;

struct Light
{
	vec3 position;
	vec4 colorAndBrightness;
	vec3 direction;
	float innerRadius;
	float outerRadius;
	float innerApertureDot;
	float outerApertureDot;
	float fractionAmbient;
	float ignoreSpecular;
	float ignorePosition;
};

//UNIFORMS

uniform Light lightList[MAX_LIGHTS];

uniform sampler2D u_diffuseTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_specularTexture;
uniform sampler2D u_emissiveTexture;
uniform sampler2D u_bumpTexture;

uniform int u_useDiffuse;
uniform int u_useTextures;
uniform int u_useNormalMap;
uniform int u_shouldCalculateSpecularLight;
uniform int u_useSpecularMap;
uniform int u_useBumpMap;
uniform int u_useEmissiveMap;
uniform vec3 u_cameraPosition;

uniform vec4 u_fogColor;
uniform float u_fogStartDistance;
uniform float u_fogEndDistance;

// INPUTS
in vec4 v_screenPosition;
in vec4 v_worldPosition;
in vec4 v_surfaceColor;
in vec2 v_textureCoordinates;

in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;

// OUTPUTS
out vec4 fragColor;

//-----------------------------------------------------------------------------------------------
vec4 ClampVec4(vec4 value, float minValue, float maxValue)
{
	value.r = min(maxValue,max(minValue, value.r));
	value.g = min(maxValue,max(minValue, value.g));
	value.b = min(maxValue,max(minValue, value.b));
	value.a = min(maxValue,max(minValue, value.a));

	return value;
}

//-----------------------------------------------------------------------------------------------
vec3 ClampVec3(vec3 value)
{
	value.r = min(1.0,max(0.0, value.r));
	value.g = min(1.0,max(0.0, value.g));
	value.b = min(1.0,max(0.0, value.b));

	return value;
}

//-----------------------------------------------------------------------------------------------
float Clampf(float value)
{
	value = min(1.0,max(0.0,value));

	return value;
}

//-----------------------------------------------------------------------------------------------
float SmoothStep(float x)
{
	return (3.0 * (x * x) - 2.0 * (x * x * x));
}

//-----------------------------------------------------------------------------------------------
float CalcLuminance (vec4 color)
{
	vec3 luminanceCoefficients = vec3( 0.22, 0.71, 0.07 );
	float luminance = dot( luminanceCoefficients, color.rgb );

	return luminance;
}

//-----------------------------------------------------------------------------------------------
vec3 GetActualPosition(vec3 actualPosition, vec3 direction, float ignorePosition)
{
	actualPosition = ((1.0 - ignorePosition) * actualPosition) + (MAX_FLOAT_VALUE * ignorePosition * direction);
	
	return actualPosition;  
} 

//-----------------------------------------------------------------------------------------------
vec2 GetAdjustedTextureCoordsAndNormals(vec3 cameraToPixelVector, inout vec3 adjustedNormal)
{
	vec2 textureCoords = v_textureCoordinates;

	mat3 TBNSpaceBasisMatrix = mat3(v_tangent,v_bitangent,v_normal); 
	mat3 TBNSpaceBasisMatrixInverse = transpose(TBNSpaceBasisMatrix);

	vec3 pixelToCameraNormalizedInTangentSpace = -normalize(cameraToPixelVector);
	pixelToCameraNormalizedInTangentSpace = TBNSpaceBasisMatrixInverse * pixelToCameraNormalizedInTangentSpace;

	float fragmentHeight = MAX_FRAGMENT_HEIGHT * CalcLuminance( texture2D(u_bumpTexture, textureCoords ) );

	textureCoords += u_useBumpMap * fragmentHeight * vec2( pixelToCameraNormalizedInTangentSpace.x, - pixelToCameraNormalizedInTangentSpace.y);
	
	vec4 normalTexel = texture2D(u_normalTexture, textureCoords );
	float dontUseNormalMap = (1.0 - u_useNormalMap);

	adjustedNormal = dontUseNormalMap * adjustedNormal + u_useNormalMap * normalTexel.xyz;
	adjustedNormal *= dontUseNormalMap + u_useNormalMap * 2.0;
	adjustedNormal -= u_useNormalMap;

	adjustedNormal = TBNSpaceBasisMatrix * adjustedNormal;

	return textureCoords;
}

//-----------------------------------------------------------------------------------------------
vec3 CalculateDiffuseForLight(inout float lightBrightness, vec3 normalRGBValues, int i)
{
	vec3 pixelToLightVector;
	vec3 pixelToLightVectorNormalized;
	vec3 lightToPixelVectorNormalized;
	float diffuseBrightness;
	float distanceBetweenPixelAndLight;
	float closenessToCenter;

	float brightnessFraction;

	pixelToLightVector = GetActualPosition(lightList[i].position, lightList[i].direction, lightList[i].ignorePosition) - v_worldPosition.xyz;
	distanceBetweenPixelAndLight = sqrt(dot(pixelToLightVector,pixelToLightVector));
	
	pixelToLightVectorNormalized = normalize(pixelToLightVector);	
	lightToPixelVectorNormalized = -pixelToLightVectorNormalized;

	closenessToCenter = dot(pixelToLightVectorNormalized, lightList[i].direction);
	closenessToCenter = (lightList[i].innerApertureDot - closenessToCenter) / (lightList[i].innerApertureDot - lightList[i].outerApertureDot);
	closenessToCenter = Clampf(closenessToCenter);
	closenessToCenter = 1.0 - closenessToCenter; 
	closenessToCenter = SmoothStep(closenessToCenter);

	brightnessFraction = 1.0 - ( 1.0 - lightList[i].ignorePosition ) * Clampf((distanceBetweenPixelAndLight - lightList[i].innerRadius) / (lightList[i].outerRadius - lightList[i].innerRadius));
	brightnessFraction = SmoothStep(brightnessFraction);
	lightBrightness = lightList[i].colorAndBrightness.a * brightnessFraction * closenessToCenter;

	diffuseBrightness = dot(pixelToLightVectorNormalized, normalRGBValues);
	diffuseBrightness *= (1.0 - lightList[i].fractionAmbient);
	diffuseBrightness = Clampf(lightList[i].fractionAmbient + diffuseBrightness);
		
	return diffuseBrightness * lightList[i].colorAndBrightness.rgb * lightBrightness;
}

//-----------------------------------------------------------------------------------------------
vec3 CalculateSpecularForLight(vec3 idealPointToLightVectorNormalized, vec3 specularMapColor, float specularIntensity, float lightBrightness, float glossiness, int i)
{
	vec3 actualPointToLightVectorNormalized = normalize(GetActualPosition(lightList[i].position, lightList[i].direction, lightList[i].ignorePosition) - v_worldPosition.xyz);
	float halfwayDotProduct = dot(idealPointToLightVectorNormalized, actualPointToLightVectorNormalized);
	halfwayDotProduct = Clampf(halfwayDotProduct);

	return (1.0 - lightList[i].ignoreSpecular) * specularMapColor * specularIntensity * lightList[i].colorAndBrightness.rgb * lightBrightness * pow(halfwayDotProduct, 1.0 + (MAX_GLOSSINESS_POWER * glossiness));
}

//-----------------------------------------------------------------------------------------------
vec4 GetEmissiveColor(vec2 textureCoords)
{
	return u_useEmissiveMap * texture2D(u_emissiveTexture, textureCoords);
}

//-----------------------------------------------------------------------------------------------
void AddFog(inout vec4 fragColor, vec3 cameraToPixelVector)
{
	float distanceBetweenPixelAndCamera = sqrt(dot(cameraToPixelVector, cameraToPixelVector));

	float fogFraction = Clampf((distanceBetweenPixelAndCamera - u_fogStartDistance) / (u_fogEndDistance - u_fogStartDistance));
	float fogStrength = fogFraction * u_fogColor.a;

	fragColor = ClampVec4(fragColor * (1.0 - fogStrength), 0.0, 1.0) + ClampVec4(u_fogColor * fogStrength, 0.0, 1.0);
}

//-----------------------------------------------------------------------------------------------
void main()
{	
	vec3 normalRGBValues = vec3(0.0,0.0,1.0);
	vec4 sumColor = vec4(0.0,0.0,0.0,1.0);
	float lightBrightness;
	
	vec3 cameraToPixelVector = v_worldPosition.xyz - u_cameraPosition;
	vec2 textureCoords = GetAdjustedTextureCoordsAndNormals(cameraToPixelVector, normalRGBValues);

	//-----------------------------------------------------------------------------------------------
	vec4 specularLight = vec4(0.0,0.0,0.0,0.0);

	vec3 idealPointToLightVectorNormalized;
	vec4 specularTexel = vec4(0.0,0.0,0.0,0.0);
	vec3 specularMapColor = vec3(1.0, 1.0, 1.0);
	float specularIntensity = 1.0;
	float glossiness = 1.0;

	//TEST
	//-----------------------------------------------------------------------------------------------
	//float testPixel = 1.0;
	//vec4 testColor = vec4(1.0,0.0,0.0,1.0);
	//-----------------------------------------------------------------------------------------------

	idealPointToLightVectorNormalized = normalize(reflect(cameraToPixelVector, normalRGBValues));
		
	specularTexel = texture2D(u_specularTexture, textureCoords );
	float dontUseSpecularMap = 1.0 - u_useSpecularMap;

	specularMapColor *= dontUseSpecularMap + u_useSpecularMap * vec3( 1.0, 1.0, 1.0 );
	specularIntensity = dontUseSpecularMap + u_useSpecularMap * specularTexel.r;
	glossiness = dontUseSpecularMap + u_useSpecularMap * specularTexel.b;
		
	//-----------------------------------------------------------------------------------------------

	for(int i = 0; i < MAX_LIGHTS; ++i)
	{
		sumColor.xyz += CalculateDiffuseForLight(lightBrightness, normalRGBValues, i);
		specularLight.xyz += CalculateSpecularForLight(idealPointToLightVectorNormalized, specularMapColor, specularIntensity, lightBrightness, glossiness, i);

		//if( (lightList[0].outerRadius - lightList[0].innerRadius) == 1.0 )
		//{
		//	testPixel *= 0.0;
		//}
	}

	sumColor = ClampVec4(sumColor, 0.0, 1.0);
	specularLight *= u_shouldCalculateSpecularLight;
	specularLight.w = 1.0;

	vec4 surfaceTimesLight = v_surfaceColor * sumColor;
	surfaceTimesLight.w = 1.0;
	
	if(u_useTextures == 1)
	{
		vec4 diffuseTexel = texture2D( u_diffuseTexture, textureCoords );

		fragColor = diffuseTexel * surfaceTimesLight;
	}
	else
	{
		fragColor = surfaceTimesLight;	
	}

	fragColor = fragColor * u_useDiffuse + specularLight;
	fragColor += GetEmissiveColor(textureCoords);
	AddFog(fragColor, cameraToPixelVector);

	//EDIT THERE IS A DIVISION BY ZERO
	//fragColor += testColor * ( 1.0 - testPixel );
}