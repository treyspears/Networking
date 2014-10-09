#version 110

//Fragment shader, GLSL 1.10
//
// INPUTS
//

//uniform float u_time;
uniform sampler2D u_diffuseTexture;
uniform int u_useTextures;

varying vec4 v_surfaceColor;	// RGBA color of the triangle at this point
varying vec2 v_textureCoordinates; //Texture coordinates of the triangle at this point

// OUTPUTS
// gl_FragColor - final color (RGBA) to report to the framebuffer (to be blended)

//-----------------------------------------------------------------------------------------------
void main()
{
	vec4 baseColor = vec4(1.0, 1.0, 1.0, 1.0);

	if(u_useTextures == 1)
	{
		vec4 diffuseTexel = texture2D( u_diffuseTexture, v_textureCoordinates );
		gl_FragColor = baseColor * (diffuseTexel * v_surfaceColor);
	}
	else
	{
		gl_FragColor = baseColor * v_surfaceColor;
	}
}


//-----------------------------------------------------------------------------------------------
//#version 110

////Fragment shader, GLSL 1.10
////
//// INPUTS
////

////uniform float u_time;
//uniform sampler2D u_diffuseTexture;
//uniform int u_useTextures;
//uniform vec3 u_cameraPosition;

//uniform vec4 u_fogColor;
//uniform float u_fogStartDistance;
//uniform float u_fogEndDistance;

//varying vec4 v_surfaceColor;	// RGBA color of the triangle at this point
//varying vec2 v_textureCoordinates; //Texture coordinates of the triangle at this point
//varying vec4 v_worldPosition;	// position of this pixel in the world space

//// OUTPUTS
//// gl_FragColor - final color (RGBA) to report to the framebuffer (to be blended)

////-----------------------------------------------------------------------------------------------
//vec4 ClampVec4(vec4 value, float minValue, float maxValue)
//{
//	value.r = min(maxValue,max(minValue, value.r));
//	value.g = min(maxValue,max(minValue, value.g));
//	value.b = min(maxValue,max(minValue, value.b));
//	value.a = min(maxValue,max(minValue, value.a));

//	return value;
//}

////-----------------------------------------------------------------------------------------------
//vec3 ClampVec3(vec3 value)
//{
//	value.r = min(1.0,max(0.0, value.r));
//	value.g = min(1.0,max(0.0, value.g));
//	value.b = min(1.0,max(0.0, value.b));

//	return value;
//}

////-----------------------------------------------------------------------------------------------
//float Clampf(float value)
//{
//	value = min(1.0,max(0.0,value));

//	return value;
//}

////-----------------------------------------------------------------------------------------------
//float SmoothStep(float x)
//{
//	return (3.0 * (x * x) - 2.0 * (x * x * x));
//}

////-----------------------------------------------------------------------------------------------
//void AddFog(inout vec4 fragColor, vec3 cameraToPixelVector)
//{
//	float distanceBetweenPixelAndCamera = sqrt(dot(cameraToPixelVector, cameraToPixelVector));

//	float fogFraction = Clampf((distanceBetweenPixelAndCamera - u_fogStartDistance) / (u_fogEndDistance - u_fogStartDistance));
//	float fogStrength = fogFraction * u_fogColor.a;

//	fragColor = ClampVec4(fragColor * (1.0 - fogStrength), 0.0, 1.0) + ClampVec4(u_fogColor * fogStrength, 0.0, 1.0);
//}



////-----------------------------------------------------------------------------------------------
//void main()
//{
//	vec4 baseColor = vec4(1.0, 1.0, 1.0, 1.0);

//	vec3 cameraToPixelVector = v_worldPosition.xyz - u_cameraPosition;

//	vec4 fragColor;

//	if(u_useTextures == 1)
//	{
//		vec4 diffuseTexel = texture2D( u_diffuseTexture, v_textureCoordinates );
//		fragColor = baseColor * (diffuseTexel * v_surfaceColor);
//	}
//	else
//	{
//		fragColor = baseColor * v_surfaceColor;
//	}

//	AddFog(fragColor, cameraToPixelVector);

//	gl_FragColor = fragColor;
//}