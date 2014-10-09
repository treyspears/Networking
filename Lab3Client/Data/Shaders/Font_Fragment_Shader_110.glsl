#version 110

//Fragment shader, GLSL 1.10
//
// INPUTS
//

uniform sampler2D u_diffuseTexture;
uniform int u_useDropShadowColor;
uniform vec4 u_dropShadowColor;

varying vec4 v_surfaceColor;	// RGBA color of the triangle at this point
varying vec2 v_textureCoordinates; //Texture coordinates of the triangle at this point

// OUTPUTS
// gl_FragColor - final color (RGBA) to report to the framebuffer (to be blended)

//-----------------------------------------------------------------------------------------------
void main()
{
	vec4 baseColor = vec4(1.0, 1.0, 1.0, 1.0);

	vec4 diffuseTexel = texture2D(u_diffuseTexture, v_textureCoordinates );
	vec4 color = baseColor * (diffuseTexel * v_surfaceColor);

	if(u_useDropShadowColor == 1)
	{
		color = color * u_dropShadowColor;
	}

	gl_FragColor = color;
}