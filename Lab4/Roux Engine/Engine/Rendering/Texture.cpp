#define STATIC

#include "Texture.hpp"

#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Rendering/stb_image.h"
#include "Engine/Utilities/CommonUtilities.hpp"

#include "Engine/Utilities/NewAndDeleteOverrides.hpp"

//-----------------------------------------------------------------------------------------------
STATIC std::map<std::string, Texture*> Texture::s_textureMap;

Texture::Texture( const std::string& imageFilePath )
	: m_openglTextureID( 0 )
	, m_size( 0, 0 )
{
	int numComponents = 0; 
	int numComponentsRequested = 0; 
	uchar* imageData = stbi_load( imageFilePath.c_str(), &m_size.x, &m_size.y, &numComponents, numComponentsRequested );

	OpenGLRenderer::Enable( GL_TEXTURE_2D );

	OpenGLRenderer::PixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	OpenGLRenderer::GenTextures( 1, (GLuint*) &m_openglTextureID );

	OpenGLRenderer::BindTexture( GL_TEXTURE_2D, m_openglTextureID );

	OpenGLRenderer::TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP ); 
	OpenGLRenderer::TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP ); 

	OpenGLRenderer::TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); 
	OpenGLRenderer::TexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR ); 


	OpenGLRenderer::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
	GLenum bufferFormat = GL_RGBA; 
	if( numComponents == 3 )
		bufferFormat = GL_RGB;

	GLenum internalFormat = bufferFormat; 

	OpenGLRenderer::TexImage2D(			
		GL_TEXTURE_2D,		
		0,					
		internalFormat,		
		m_size.x,			
		m_size.y,			
		0,					
		bufferFormat,		
		GL_UNSIGNED_BYTE,	
		imageData );		

	stbi_image_free( imageData );
	OpenGLRenderer::GenerateMipmap(GL_TEXTURE_2D);

	s_textureMap[imageFilePath] = this;
}
	

//-----------------------------------------------------------------------------------------------
STATIC Texture* Texture::GetTextureByName (const std::string& imageFilePath)
{
	std::map<std::string, Texture*>::iterator foundEntryIter;
	
	foundEntryIter = s_textureMap.find(imageFilePath);

	if(foundEntryIter == s_textureMap.end())
		return nullptr;

	return foundEntryIter->second;
}

//-----------------------------------------------------------------------------------------------
STATIC Texture* Texture::CreateOrGetTexture(const std::string& imageFilePath)
{
	Texture* texture = GetTextureByName(imageFilePath);

	if(!texture)
	{
		texture = new Texture(imageFilePath);
	}

	return texture;
}