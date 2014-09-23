#ifndef MATERIAL 
#define MATERIAL

#include "Engine/Rendering/Texture.hpp"
#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Primitives/Vector4.hpp"
#include "Engine/Primitives/Matrix44.hpp"

class Material 
{
public:

	Material();

	void Initialize(int shaderProgramID, Texture* diffuseTexture
		, Texture* normalMap, Texture* specularMap, Texture* emissiveMap, Texture* bumpMap);

	void Enable( int bufferID, const Vector3f& cameraPosition, bool useDiffuse,
			    bool useTextures, bool useNormalMap, bool shouldCalculateSpecularLight, bool useSpecularMap,
				bool useBumpMap, bool useEmissiveMap, const Vector4f& fogColor, float fogStartDistance, float fogEndDistance);

	void Disable();

private:

	Texture* m_diffuseTexture;
	Texture* m_normalMap;
	Texture* m_specularMap;
	Texture* m_emissiveMap;
	Texture* m_bumpMap;

	int m_shaderProgramID;

	int m_diffuseTextureLocation;
	int m_normalMapLocation;
	int m_specularMapLocation;
	int m_emissiveMapLocation;
	int m_bumpMapLocation;
	
	int m_modelMatrixLocation;
	int m_modelViewMatrixLocation;
	int m_cameraPositionLocation;

	int m_fogColorLocation;
	int m_fogStartDistanceLocation;
	int m_fogEndDistanceLocation;

	int m_useDiffuseLocation;
	int m_useTexturesLocation;
	int m_useNormalMapLocation;
	int m_shouldCalculateSpecularLightLocation;
	int m_useSpecularMapLocation;
	int m_useEmissiveMapLocation;
	int m_useBumpMapLocation;

	int m_vertexAttributeLocation;
	int m_normalAttributeLocation;
	int m_tangentAttributeLocation;
	int m_bitangentAttributeLocation;
	int m_colorAttributeLocation;
	int m_textureCoordsAttributeLocation;
};

#endif