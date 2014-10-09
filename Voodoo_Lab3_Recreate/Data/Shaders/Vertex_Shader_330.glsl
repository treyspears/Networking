#version 330 
//Vertex shader

//UNIFORMS
uniform mat4 u_modelViewMatrix;
uniform mat4 u_modelMatrix;

//INPUTS
in vec4 a_vertex;
in vec3 a_normal;
in vec3 a_tangent;
in vec3 a_bitangent;
in vec4 a_color;
in vec2 a_texCoords;

//OUTPUTS
out vec4 v_screenPosition;
out vec4 v_worldPosition;
out vec4 v_surfaceColor;
out vec2 v_textureCoordinates;

out vec3 v_normal;
out vec3 v_tangent;
out vec3 v_bitangent;

void main()
{
	v_worldPosition = u_modelMatrix * a_vertex;
	v_surfaceColor = a_color;
	v_textureCoordinates = a_texCoords;

	v_normal = a_normal;
	v_tangent = a_tangent;
	v_bitangent = a_bitangent;

	gl_Position = u_modelViewMatrix * a_vertex;
	
	v_screenPosition = gl_Position;
}