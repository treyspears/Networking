#ifndef BITMAPFONT
#define BITMAPFONT

#include <map>
#include <string>

#include "Engine/Rendering/Texture.hpp"

class BitmapFont
{
	
private:

	static BitmapFont* GetFontByName(const std::string& glyphSheetFilePath, const std::string& metaDataFilePath);
	static std::map<std::string, BitmapFont*> s_fontMap;

public:

	class Glyph
	{
	public:
		Glyph();
		Glyph(char ucsIndex, uint sheetIndex, Vector2f minTextureCoords, Vector2f maxTextureCoords
			, float preDrawSpacing, float drawSpacing, float postDrawSpacing);

		uchar m_ucsIndex;
		uint m_sheetIndex;
		Vector2f m_minTextureCoords;
		Vector2f m_maxTextureCoords;
		float m_preDrawSpacing;
		float m_drawSpacing;
		float m_postDrawSpacing;
	};

	BitmapFont(const std::string& glyphSheetFilePath, const std::string& metaDataFilePath);
	static BitmapFont* CreateOrGetFont(const std::string& glyphSheetFilePath, const std::string& metaDataFilePath);

	Texture* m_texture;
	std::map<uchar, Glyph> m_glyphs;
	std::string m_fontName;
	std::string m_locale;
	uint m_numTextureSheets;
	uint m_glpyhHeight;
};

#endif