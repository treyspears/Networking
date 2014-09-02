#version 110 /* version number 1.10, 3.0 + matches GLSL */

//Vertex shader
//
//INPUTS
// gl_Vertex - vertex positon in world space (raw, from VBO)
// gl_ModelViewProjectionMatrix - concatenation of GL_MODELVIW and GL_PROJECTION
// typically the world to screen transform

//OUTPUTS
// gl_Position - vertex position in screen space (for OpenGL to make triangles out of)
// v_screenPosition - vertex position in screen space (for me, to pass to my own fragment shader)

varying vec4 v_surfaceColor;
varying vec2 v_textureCoordinates;

//varying vec4 v_worldPosition;

void main()
{
	//v_worldPosition = gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	v_surfaceColor = gl_Color; 
	v_textureCoordinates = gl_MultiTexCoord0.xy;
}