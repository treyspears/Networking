#version 110

//Fragment shader, GLSL 1.10
//
// INPUTS
//

//uniform float u_time;
uniform sampler2D u_diffuseTexture;
uniform int u_useTextures;
uniform float u_damageFlashTime;

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
		float inverseTime = 1.0 - u_damageFlashTime;
		vec4 xFormedTexel = inverseTime * diffuseTexel + u_damageFlashTime * vec4( diffuseTexel.x * u_damageFlashTime, diffuseTexel.yz * inverseTime, diffuseTexel.a );
		gl_FragColor = baseColor * (xFormedTexel * v_surfaceColor) + u_damageFlashTime * vec4(1.0,0.0,0.0,1.0) * diffuseTexel.a;
	}
	else
	{
		gl_FragColor = baseColor * v_surfaceColor;
	}
}