#ifndef TEXTURE
#define TEXTURE

#include <map>
#include <string>

#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"

class Texture{

public:

	Texture(const std::string& imageFilePath);
	static Texture* CreateOrGetTexture(const std::string& imageFilePath);

	uint		m_openglTextureID;
	Vector2i	m_size;

private:

	static Texture* GetTextureByName (const std::string& imageFilePath);
	static std::map<std::string, Texture*>	s_textureMap;
};

#endif