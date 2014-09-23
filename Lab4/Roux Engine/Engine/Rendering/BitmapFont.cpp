#include "BitmapFont.hpp"

#include <assert.h>
#include <iostream>

#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Utilities/pugixml.hpp"

#include "Engine/Utilities/NewAndDeleteOverrides.hpp"

std::map<std::string, BitmapFont*> BitmapFont::s_fontMap;

//-----------------------------------------------------------------------------------------------
BitmapFont::Glyph::Glyph()
	: m_ucsIndex(0)
	, m_sheetIndex(0)
	, m_minTextureCoords(0.f, 0.f)
	, m_maxTextureCoords(0.f, 0.f)
	, m_preDrawSpacing(0.f)
	, m_drawSpacing(0.f)
	, m_postDrawSpacing(0.f)
{

}

//-----------------------------------------------------------------------------------------------
BitmapFont::Glyph::Glyph(char ucsIndex, uint sheetIndex, Vector2f minTextureCoords, Vector2f maxTextureCoords , float preDrawSpacing, float drawSpacing, float postDrawSpacing)
	: m_ucsIndex(ucsIndex)
	, m_sheetIndex(sheetIndex)
	, m_minTextureCoords(minTextureCoords)
	, m_maxTextureCoords(maxTextureCoords)
	, m_preDrawSpacing(preDrawSpacing)
	, m_drawSpacing(drawSpacing)
	, m_postDrawSpacing(postDrawSpacing)
{

}

//-----------------------------------------------------------------------------------------------
BitmapFont::BitmapFont(const std::string& glyphSheetFilePath, const std::string& metaDataFilePath)
{
	m_texture = Texture::CreateOrGetTexture(glyphSheetFilePath);

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(metaDataFilePath.c_str());	

	assert(!result.status);

	pugi::xml_node fontInfo = doc.child("FontDefinition").child("FontInfo");

	m_fontName = fontInfo.attribute("name").value();
	m_locale = fontInfo.attribute("locale").value();
	m_numTextureSheets = atoi(fontInfo.attribute("numTextureSheets").value());
	m_glpyhHeight = atoi(fontInfo.attribute("cellPixelHeight").value());


	uchar character;
	uint sheetIndex;
	std::string coordText;
	std::vector<std::string> coords;
	Vector2f minTextureCoords;
	Vector2f maxTextureCoords;
	float preDrawSpacing;
	float drawSpacing;
	float postDrawSpacing;

	for (pugi::xml_node glyph = doc.child("FontDefinition").child("Glyph"); glyph; glyph = glyph.next_sibling("Glyph"))
	{
		character = (uchar)atoi(glyph.attribute("ucsIndex").value());
		sheetIndex = atoi(glyph.attribute("sheet").value());

		coordText = (glyph.attribute("texCoordMins").value());
		coords = SplitStringByStringDelimiter(coordText,",");
		minTextureCoords = Vector2f((float)atof(coords[0].c_str()), (float)atof(coords[1].c_str()));
		
		coordText = (glyph.attribute("texCoordMaxs").value());
		coords = SplitStringByStringDelimiter(coordText,",");
		maxTextureCoords = Vector2f((float)atof(coords[0].c_str()), (float)atof(coords[1].c_str()));

		preDrawSpacing = (float)atof(glyph.attribute("ttfA").value());
		drawSpacing = (float)atof(glyph.attribute("ttfB").value());
		postDrawSpacing = (float)atof(glyph.attribute("ttfC").value());

		Glyph glyphToAdd(character, sheetIndex, minTextureCoords, maxTextureCoords, preDrawSpacing, drawSpacing, postDrawSpacing);

		m_glyphs[character] = glyphToAdd;
	}

	std::string compoundName = glyphSheetFilePath + metaDataFilePath;

	BitmapFont::s_fontMap[compoundName] = this;
		
}

//-----------------------------------------------------------------------------------------------
BitmapFont* BitmapFont::GetFontByName (const std::string& glyphSheetFilePath, const std::string& metaDataFilePath)
{
	std::map<std::string, BitmapFont*>::iterator foundEntryIter;
	std::string compoundName = glyphSheetFilePath + metaDataFilePath;

	foundEntryIter = s_fontMap.find(compoundName);

	if(foundEntryIter == s_fontMap.end())
		return nullptr;

	return foundEntryIter->second;
}

//-----------------------------------------------------------------------------------------------
BitmapFont* BitmapFont::CreateOrGetFont(const std::string& glyphSheetFilePath, const std::string& metaDataFilePath)
{
	BitmapFont* font = GetFontByName(glyphSheetFilePath, metaDataFilePath);

	if(!font)
	{
		font = new BitmapFont(glyphSheetFilePath, metaDataFilePath);
	}

	return font;
}