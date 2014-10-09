#include "Material.hpp"

#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Rendering/Vertex.hpp"

//-----------------------------------------------------------------------------------------------
Material::Material()
	: m_shaderProgramID(-1)
	, m_diffuseTexture(nullptr)
	, m_normalMap(nullptr)
	, m_specularMap(nullptr)
	, m_emissiveMap(nullptr)
	, m_bumpMap(nullptr)

	, m_diffuseTextureLocation(-1)
	, m_normalMapLocation(-1)
	, m_specularMapLocation(-1)
	, m_emissiveMapLocation(-1)
	, m_bumpMapLocation(-1)

	, m_modelMatrixLocation(-1)
	, m_modelViewMatrixLocation(-1)
	, m_cameraPositionLocation(-1)

	, m_fogColorLocation(-1)
	, m_fogStartDistanceLocation(-1)
	, m_fogEndDistanceLocation(-1)

	, m_useDiffuseLocation(-1)
	, m_useTexturesLocation(-1)
	, m_useNormalMapLocation(-1)
	, m_shouldCalculateSpecularLightLocation(-1)
	, m_useSpecularMapLocation(-1)
	, m_useBumpMapLocation(-1)
	, m_useEmissiveMapLocation(-1)

	, m_vertexAttributeLocation(-1)
	, m_normalAttributeLocation(-1)
	, m_tangentAttributeLocation(-1)
	, m_bitangentAttributeLocation(-1)
	, m_colorAttributeLocation(-1)
	, m_textureCoordsAttributeLocation(-1)
{

}

//-----------------------------------------------------------------------------------------------
void Material::Initialize(int shaderProgramID, Texture* diffuseTexture
	, Texture* normalMap, Texture* specularMap, Texture* emissiveMap, Texture* bumpMap)
{
	m_shaderProgramID = shaderProgramID;

	m_diffuseTexture = diffuseTexture;
	m_normalMap = normalMap;
	m_specularMap = specularMap;
	m_emissiveMap = emissiveMap;
	m_bumpMap = bumpMap;

	OpenGLRenderer::UseShaderProgram(shaderProgramID);

	m_diffuseTextureLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_diffuseTexture");
	m_normalMapLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_normalTexture");
	m_specularMapLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_specularTexture");
	m_emissiveMapLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_emissiveTexture");
	m_bumpMapLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_bumpTexture");

	m_modelMatrixLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_modelMatrix");
	m_modelViewMatrixLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_modelViewMatrix");
	m_cameraPositionLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_cameraPosition");

	m_fogColorLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_fogColor");
	m_fogStartDistanceLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_fogStartDistance");
	m_fogEndDistanceLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_fogEndDistance");

	m_useDiffuseLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_useDiffuse");
	m_useTexturesLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_useTextures");
	m_useNormalMapLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_useNormalMap");
	m_shouldCalculateSpecularLightLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_shouldCalculateSpecularLight");
	m_useSpecularMapLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_useSpecularMap");
	m_useEmissiveMapLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_useEmissiveMap");
	m_useBumpMapLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_useBumpMap");

	m_vertexAttributeLocation = OpenGLRenderer::GetAttribLocation(shaderProgramID, "a_vertex");
	m_normalAttributeLocation = OpenGLRenderer::GetAttribLocation(shaderProgramID, "a_normal");
	m_tangentAttributeLocation = OpenGLRenderer::GetAttribLocation(shaderProgramID, "a_tangent");
	m_bitangentAttributeLocation = OpenGLRenderer::GetAttribLocation(shaderProgramID, "a_bitangent");
	m_colorAttributeLocation = OpenGLRenderer::GetAttribLocation(shaderProgramID, "a_color");
	m_textureCoordsAttributeLocation = OpenGLRenderer::GetAttribLocation(shaderProgramID, "a_texCoords");
}

//-----------------------------------------------------------------------------------------------
void Material::Enable( int bufferID, const Vector3f& cameraPosition, bool useDiffuse,
	bool useTextures, bool useNormalMap, bool shouldCalculateSpecularLight, bool useSpecularMap,
	bool useBumpMap, bool useEmissiveMap, const Vector4f& fogColor, float fogStartDistance, float fogEndDistance)
{
	const float* colorOffset		= reinterpret_cast< float* >( offsetof( VertexPositionNTBColorTexture, VertexPositionNTBColorTexture::diffuseColor ) );
	const float* vertexOffset		= reinterpret_cast< float* >( offsetof( VertexPositionNTBColorTexture, VertexPositionNTBColorTexture::position ) );
	const float* normalOffset		= reinterpret_cast< float* >( offsetof( VertexPositionNTBColorTexture, VertexPositionNTBColorTexture::normal ) );
	const float* tangentOffset		= reinterpret_cast< float* >( offsetof( VertexPositionNTBColorTexture, VertexPositionNTBColorTexture::tangent ) );
	const float* bitangentOffset	= reinterpret_cast< float* >( offsetof( VertexPositionNTBColorTexture, VertexPositionNTBColorTexture::bitangent ) );
	const float* UVOffset			= reinterpret_cast< float* >( offsetof( VertexPositionNTBColorTexture, VertexPositionNTBColorTexture::textureCoordinates ) );
	const int	 stride				= sizeof( VertexPositionNTBColorTexture );

	OpenGLRenderer::Enable(GL_TEXTURE_2D);

	OpenGLRenderer::UseShaderProgram(m_shaderProgramID);

	int iUseDiffuse						= static_cast< int >( useDiffuse == true );
	int iUseTextures					= static_cast< int >( useTextures == true );
	int iUseNormalMap					= static_cast< int >( useNormalMap == true );
	int iShouldCalculateSpecularLight	= static_cast< int >( shouldCalculateSpecularLight == true );
	int iUseSpecularMap					= static_cast< int >( useSpecularMap == true );
	int iUseBumpMap						= static_cast< int >( useBumpMap == true );
	int iUseEmissiveMap					= static_cast< int >( useEmissiveMap == true );

	OpenGLRenderer::Uniform1i(m_useDiffuseLocation, iUseDiffuse);

	if(m_diffuseTexture != nullptr)
	{
		OpenGLRenderer::ActiveTexture(GL_TEXTURE0);
		OpenGLRenderer::BindTexture(GL_TEXTURE_2D, m_diffuseTexture->m_openglTextureID);
		OpenGLRenderer::Uniform1i(m_diffuseTextureLocation,0);
	}
	else
	{
		iUseTextures = 0;
	}

	OpenGLRenderer::Uniform1i(m_useTexturesLocation, iUseTextures);


	if(m_normalMap != nullptr)
	{
		OpenGLRenderer::ActiveTexture(GL_TEXTURE1);
		OpenGLRenderer::BindTexture(GL_TEXTURE_2D, m_normalMap->m_openglTextureID);
		OpenGLRenderer::Uniform1i(m_normalMapLocation, 1);
	}
	else
	{
		iUseNormalMap = 0;
	}

	OpenGLRenderer::Uniform1i(m_useNormalMapLocation, iUseNormalMap);

	OpenGLRenderer::Uniform1i(m_shouldCalculateSpecularLightLocation, iShouldCalculateSpecularLight);

	if(m_specularMap != nullptr)
	{
		OpenGLRenderer::ActiveTexture(GL_TEXTURE2);
		OpenGLRenderer::BindTexture(GL_TEXTURE_2D, m_specularMap->m_openglTextureID);
		OpenGLRenderer::Uniform1i(m_specularMapLocation, 2);
	}
	else
	{
		iUseSpecularMap = 0;
	}

	OpenGLRenderer::Uniform1i(m_useSpecularMapLocation, iUseSpecularMap);

	if(m_emissiveMap != nullptr)
	{
		OpenGLRenderer::ActiveTexture(GL_TEXTURE3);
		OpenGLRenderer::BindTexture(GL_TEXTURE_2D, m_emissiveMap->m_openglTextureID);
		OpenGLRenderer::Uniform1i(m_emissiveMapLocation, 3);
	}

	OpenGLRenderer::Uniform1i(m_useEmissiveMapLocation, iUseEmissiveMap);

	if(m_bumpMap != nullptr)
	{
		OpenGLRenderer::ActiveTexture(GL_TEXTURE4);
		OpenGLRenderer::BindTexture(GL_TEXTURE_2D, m_bumpMap->m_openglTextureID);
		OpenGLRenderer::Uniform1i(m_bumpMapLocation, 4);
	}
	else
	{
		iUseBumpMap = 0;
	}

	OpenGLRenderer::Uniform1i(m_useBumpMapLocation, iUseBumpMap);

	Matrix44f modelMatrix = OpenGLRenderer::s_rendererStack.GetCurrent(MatrixStack44f::MODEL_STACK);

	Matrix44f modelViewMatrix = OpenGLRenderer::s_rendererStack.GetMVWithProjection();

	OpenGLRenderer::UniformMatrix4fv(m_modelMatrixLocation, 1, false, modelMatrix.data);
	OpenGLRenderer::UniformMatrix4fv(m_modelViewMatrixLocation, 1, false, modelViewMatrix.data);
	OpenGLRenderer::Uniform3f(m_cameraPositionLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	OpenGLRenderer::Uniform4f(m_fogColorLocation, fogColor.x, fogColor.y, fogColor.z, fogColor.w);
	OpenGLRenderer::Uniform1f(m_fogStartDistanceLocation, fogStartDistance);
	OpenGLRenderer::Uniform1f(m_fogEndDistanceLocation, fogEndDistance);

	OpenGLRenderer::BindBuffer( GL_ARRAY_BUFFER, bufferID );

	OpenGLRenderer::EnableVertexAttribArray(m_vertexAttributeLocation);
	OpenGLRenderer::VertexAttribPointer(m_vertexAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, vertexOffset);	

	OpenGLRenderer::EnableVertexAttribArray(m_normalAttributeLocation);
	OpenGLRenderer::VertexAttribPointer(m_normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, normalOffset);

	OpenGLRenderer::EnableVertexAttribArray(m_tangentAttributeLocation);
	OpenGLRenderer::VertexAttribPointer(m_tangentAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, tangentOffset);

	OpenGLRenderer::EnableVertexAttribArray(m_bitangentAttributeLocation);
	OpenGLRenderer::VertexAttribPointer(m_bitangentAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, bitangentOffset);

	OpenGLRenderer::EnableVertexAttribArray(m_colorAttributeLocation);
	OpenGLRenderer::VertexAttribPointer(m_colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, colorOffset);

	OpenGLRenderer::EnableVertexAttribArray(m_textureCoordsAttributeLocation);
	OpenGLRenderer::VertexAttribPointer(m_textureCoordsAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, UVOffset);
}

//-----------------------------------------------------------------------------------------------
void Material::Disable()
{
	OpenGLRenderer::DisableVertexAttribArray(m_vertexAttributeLocation);
	OpenGLRenderer::DisableVertexAttribArray(m_normalAttributeLocation);
	OpenGLRenderer::DisableVertexAttribArray(m_tangentAttributeLocation);
	OpenGLRenderer::DisableVertexAttribArray(m_bitangentAttributeLocation);
	OpenGLRenderer::DisableVertexAttribArray(m_colorAttributeLocation);
	OpenGLRenderer::DisableVertexAttribArray(m_textureCoordsAttributeLocation);
	

	OpenGLRenderer::BindBuffer(GL_ARRAY_BUFFER,0);

	OpenGLRenderer::ActiveTexture(GL_TEXTURE0);
	OpenGLRenderer::BindTexture(GL_TEXTURE_2D, 0);

	OpenGLRenderer::ActiveTexture(GL_TEXTURE1);
	OpenGLRenderer::BindTexture(GL_TEXTURE_2D, 0);

	OpenGLRenderer::ActiveTexture(GL_TEXTURE2);
	OpenGLRenderer::BindTexture(GL_TEXTURE_2D, 0);

	OpenGLRenderer::ActiveTexture(GL_TEXTURE3);
	OpenGLRenderer::BindTexture(GL_TEXTURE_2D, 0);

	OpenGLRenderer::ActiveTexture(GL_TEXTURE4);
	OpenGLRenderer::BindTexture(GL_TEXTURE_2D, 0);

	OpenGLRenderer::Disable(GL_TEXTURE_2D);
}