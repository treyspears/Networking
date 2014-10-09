#version 110

//Fragment shader, GLSL 1.10
//
// INPUTS
//

//uniform float u_time;
uniform sampler2D u_diffuseTexture;
uniform sampler3D u_normalTexture;
uniform vec3 u_lightPosition;
uniform float u_time;
uniform int u_debugInt;

varying vec3 v_screenPosition;	// screenPosition - position of this pixel in screen space
varying vec4 v_worldPosition;	// position of this pixel in the world space
varying vec4 v_surfaceColor;	// RGBA color of the triangle at this point
varying vec2 v_textureCoordinates; //Texture coordinates of the triangle at this point


//-----------------------------------------------------------------------------------------------
void main()
{
	vec4 diffuseTexel = texture2D( u_diffuseTexture, v_textureCoordinates );
	gl_FragColor = (diffuseTexel * v_surfaceColor);

}