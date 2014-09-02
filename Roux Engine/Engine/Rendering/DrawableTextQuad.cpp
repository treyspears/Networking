#include "DrawableTextQuad.hpp"

#include <string>

#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Rendering/BitmapFont.hpp"

//-----------------------------------------------------------------------------------------------
DrawableTextQuad::DrawableTextQuad(float fontSize, const Vector3f& position, BitmapFont* fontToUse)
	: m_fontSize(fontSize)
	, m_position(position)
	, m_font(fontToUse)
	, m_currentString("")
	, m_lastFontColor(Vector4f(1.f,1.f,1.f,1.f))
	, m_currentTextLength(0.f)
	, m_bufferId( UNASSINGED_BUFFERID )
{

}


//-----------------------------------------------------------------------------------------------
DrawableTextQuad::~DrawableTextQuad()
{
	//OpenGLRenderer::DeleteBuffers( 1, &m_bufferId );

	m_vertices.clear();
	m_font = nullptr;
	delete m_font;
}

//-----------------------------------------------------------------------------------------------
void DrawableTextQuad::AppendTextQuad(float startingXOffest, float endingXOffset, const Vector2f& minTextureCoords
	, const Vector2f& maxTextureCoords, const Vector4f& color)
{
	Vector3f minXminYQuadPosition = Vector3f(0.f,0.f,0.f);
	minXminYQuadPosition.x += startingXOffest;

	Vector3f minXmaxYQuadPosition = minXminYQuadPosition;
	minXmaxYQuadPosition.y += m_fontSize;

	Vector3f maxXminYQuadPosition = minXminYQuadPosition;
	maxXminYQuadPosition.x += endingXOffset;

	Vector3f maxXmaxYQuadPosition(maxXminYQuadPosition.x, minXmaxYQuadPosition.y, m_position.z);

	VertexPositionColorTexture v0(minXminYQuadPosition, color, Vector2f(minTextureCoords.x, maxTextureCoords.y)); 
	VertexPositionColorTexture v1(maxXminYQuadPosition, color, maxTextureCoords);
	VertexPositionColorTexture v2(maxXmaxYQuadPosition, color, Vector2f(maxTextureCoords.x, minTextureCoords.y));
	VertexPositionColorTexture v3(minXmaxYQuadPosition, color, minTextureCoords);

	m_vertices.push_back(v2);
	m_vertices.push_back(v3);
	m_vertices.push_back(v0);
	m_vertices.push_back(v1);		
}

//-----------------------------------------------------------------------------------------------
void DrawableTextQuad::DeleteEndCharacter()
{	
	if(m_currentString.size() > 0)
	{
		std::string newString;
		m_currentString.pop_back();
		newString = m_currentString;
		m_currentString.clear();
		AppendQuadFromString(newString,m_lastFontColor);
	}
}

//-----------------------------------------------------------------------------------------------
void DrawableTextQuad::ClearQuad()
{
	m_currentString.clear();
	m_currentTextLength = 0.f;
	AppendQuadFromString("",m_lastFontColor);
}

//-----------------------------------------------------------------------------------------------
float DrawableTextQuad::GetFontSize() const
{
	return m_fontSize;
}

//-----------------------------------------------------------------------------------------------
void DrawableTextQuad::AppendQuadFromString(const std::string& textToRender,
	const Vector4f& fontColor)
{
	float lengthOfText = 0.f;
	const Vector2f ZERO_VECTOR(0.f, 0.f);
	std::map<uchar, BitmapFont::Glyph>::iterator foundIter;
	m_lastFontColor = fontColor;

	m_currentString += textToRender;

	for(auto iter = m_currentString.begin(); iter != m_currentString.end(); ++ iter)
	{
		foundIter = m_font->m_glyphs.find(*iter);

		if(foundIter != m_font->m_glyphs.end())
		{
			BitmapFont::Glyph currentGlyph = foundIter->second;
			if(currentGlyph.m_preDrawSpacing > 0.f)
			{
				this->AppendTextQuad(lengthOfText,currentGlyph.m_preDrawSpacing * m_fontSize,ZERO_VECTOR,ZERO_VECTOR,fontColor);
				lengthOfText += currentGlyph.m_preDrawSpacing * m_fontSize;
			}
			this->AppendTextQuad(lengthOfText, currentGlyph.m_drawSpacing * m_fontSize, currentGlyph.m_minTextureCoords, currentGlyph.m_maxTextureCoords, fontColor);
			lengthOfText += currentGlyph.m_drawSpacing * m_fontSize;

			if(currentGlyph.m_postDrawSpacing > 0.f)
			{
				this->AppendTextQuad(lengthOfText,currentGlyph.m_postDrawSpacing * m_fontSize,ZERO_VECTOR,ZERO_VECTOR,fontColor);
				lengthOfText += currentGlyph.m_postDrawSpacing * m_fontSize;
			}
		}
	}
	m_currentTextLength = lengthOfText;
	CreateVBO();
}

//-----------------------------------------------------------------------------------------------
void DrawableTextQuad::CreateVBO()
{
	OpenGLRenderer::GenBuffers(1, &m_bufferId);

	OpenGLRenderer::BindBuffer(GL_ARRAY_BUFFER,m_bufferId);

	int sizeOfData = m_vertices.size() * sizeof(VertexPositionColorTexture);

	OpenGLRenderer::BufferData(GL_ARRAY_BUFFER,sizeOfData,m_vertices.data(),GL_STATIC_DRAW);

	OpenGLRenderer::BindBuffer(GL_ARRAY_BUFFER,0);

	m_numVertices = m_vertices.size();
	m_vertices.clear();
}

//-----------------------------------------------------------------------------------------------
void DrawableTextQuad::Render(uint shaderProgramID, const Vector3f& additionalTranslation, bool useDropShadow
	, const Vector4f& dropShadowColor) const
{	 
	const float*colorOffset = (float*)offsetof(VertexPositionColorTexture,VertexPositionColorTexture::diffuseColor);
	const float* vertexOffset = (float*)offsetof(VertexPositionColorTexture,VertexPositionColorTexture::position);
	const float* UVOffset = (float*)offsetof(VertexPositionColorTexture,VertexPositionColorTexture::textureCoordinates);
	const int stride = sizeof(VertexPositionColorTexture);

	const float inverseAspectRatio = (float)OpenGLRenderer::s_screenSize.y / (float)OpenGLRenderer::s_screenSize.x;
	const float orthoWidth = 1024;
	const float orthoHeight = orthoWidth * inverseAspectRatio;

	const float dropShadowScale = 0.0694444444444444444f;
	Vector3f dropShadowOffset(0.f,0.f,0.f);
	if(useDropShadow)
	 dropShadowOffset = Vector3f(m_fontSize, -m_fontSize, 0.f) * dropShadowScale;

	OpenGLRenderer::s_rendererStack.Push(MatrixStack44f::MODEL_STACK);

	OpenGLRenderer::Disable(GL_DEPTH_TEST);
	OpenGLRenderer::Disable(GL_CULL_FACE);
	OpenGLRenderer::Enable(GL_TEXTURE_2D);
	OpenGLRenderer::ActiveTexture(GL_TEXTURE0);
	OpenGLRenderer::BindTexture(GL_TEXTURE_2D, m_font->m_texture->m_openglTextureID);

	OpenGLRenderer::s_rendererStack.ApplyOrtho(0.f, orthoWidth, 0.f, orthoHeight, 0.f, 1.f);
	OpenGLRenderer::s_rendererStack.ApplyTranslation(MatrixStack44f::MODEL_STACK, m_position);
	OpenGLRenderer::s_rendererStack.ApplyTranslation(MatrixStack44f::MODEL_STACK, additionalTranslation);

	OpenGLRenderer::LoadMatrix(OpenGLRenderer::s_rendererStack.GetMVWithProjection());

	OpenGLRenderer::UseShaderProgram(shaderProgramID);
	int uniformLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_diffuseTexture");
	OpenGLRenderer::Uniform1i(uniformLocation,0);

	int dropShadowLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_dropShadowColor");
	OpenGLRenderer::Uniform4f(dropShadowLocation, dropShadowColor.x, dropShadowColor.y, dropShadowColor.z, dropShadowColor.w);
	
	int useDropShadowLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_useDropShadowColor");
	OpenGLRenderer::Uniform1i(useDropShadowLocation,(int)useDropShadow);

	int dropShadowOffsetLocation = OpenGLRenderer::GetUniformLocation(shaderProgramID, "u_dropShadowOffset");
	OpenGLRenderer::Uniform4f(dropShadowOffsetLocation, dropShadowOffset.x, dropShadowOffset.y, dropShadowOffset.z, 0.f);

	OpenGLRenderer::BindBuffer(GL_ARRAY_BUFFER,m_bufferId);

	OpenGLRenderer::VertexPointer(3,GL_FLOAT,stride,vertexOffset);
	OpenGLRenderer::ColorPointer(4,GL_FLOAT,stride,colorOffset);
	OpenGLRenderer::TexCoordPointer(2,GL_FLOAT,stride,UVOffset);

	OpenGLRenderer::EnableClientState(GL_VERTEX_ARRAY);
	OpenGLRenderer::EnableClientState(GL_COLOR_ARRAY);
	OpenGLRenderer::EnableClientState(GL_TEXTURE_COORD_ARRAY);

	OpenGLRenderer::DrawArrays(GL_QUADS,0,m_numVertices);

	OpenGLRenderer::DisableClientState(GL_TEXTURE_COORD_ARRAY);
	OpenGLRenderer::DisableClientState(GL_COLOR_ARRAY);
	OpenGLRenderer::DisableClientState(GL_VERTEX_ARRAY);

	//OpenGLRenderer::Enable(GL_CULL_FACE);
	//OpenGLRenderer::Enable(GL_DEPTH_TEST);

	OpenGLRenderer::s_rendererStack.Pop(MatrixStack44f::MODEL_STACK);

	if(useDropShadow)
		Render(shaderProgramID,additionalTranslation);
}