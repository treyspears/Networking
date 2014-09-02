#ifndef DRAWABLETEXTQUAD
#define DRAWABLETEXTQUAD

#include <vector>
#include <string>

#include "Engine/Rendering/Texture.hpp"
#include "Engine/Rendering/Vertex.hpp"
#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Primitives/Vector4.hpp"

class BitmapFont;

class DrawableTextQuad
{
public:

	DrawableTextQuad(float fontSize, const Vector3f& position, BitmapFont* fontToUse);

	~DrawableTextQuad();

	void AppendQuadFromString(const std::string& textToRender,
		const Vector4f& fontColor);

	void DeleteEndCharacter();
	void ClearQuad();
	float GetFontSize() const;
	inline int GetStringLength() const;
	inline float GetQuadLength() const;
	inline const std::string GetString() const;


	void Render(uint shaderProgramID, const Vector3f& additionalTranslation = Vector3f(0.f,0.f,0.f), bool useDropShadow = false
		, const Vector4f& dropShadowColor = Vector4f(0.f,0.f,0.f,1.f)) const;

	uint m_bufferId;

private:

	void CreateVBO();
	void AppendTextQuad(float startingXOffest, float endingXOffset, const Vector2f& minTextureCoords
		, const Vector2f& maxTextureCoords, const Vector4f& color);

	float m_currentTextLength;
	uint m_numVertices;
	float m_fontSize;
	Vector4f m_lastFontColor;
	Vector3f m_position;
	BitmapFont* m_font;
	std::string m_currentString;
	std::vector<VertexPositionColorTexture> m_vertices;
};

int DrawableTextQuad::GetStringLength() const
{
	return m_currentString.size();
}

float DrawableTextQuad::GetQuadLength() const
{
	return m_currentTextLength;
}

const std::string DrawableTextQuad::GetString() const
{
	return m_currentString;
}

#endif