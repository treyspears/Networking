#include "ConsoleLog.hpp"


#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/OpenGLRenderer.hpp"
#include "Engine/Primitives/Color.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Utilities/CommandRegistry.hpp"
#include "Engine/Utilities/InputHandler.hpp"

#include "Engine/Utilities/NewAndDeleteOverrides.hpp"

#define UNUSED(x) (void)(x);

std::vector<ConsoleLog*> ConsoleLog::s_logs;
ConsoleLog* ConsoleLog::s_currentLog;


//-----------------------------------------------------------------------------------------------
ConsoleLog::ConsoleLog(BitmapFont* fontToUse, float fontSize)
	: m_fontToUse(fontToUse)
	, m_fontSize(fontSize)
	, m_showLog(false)
	, m_stackIndex(0)
	, m_quadBufferId( UNASSINGED_BUFFERID )
{
	CreateTranslucentQuads();
	DrawableTextQuad aQuad(fontSize,Vector3f(0.f,0.f,0.f),fontToUse);
	m_commandPromptQuad = new TextQuad(aQuad,true,Vector4f(0.f,0.f,0.f,1.f));
	Vector3f color = Color(Chartreuse).ToVector3fNormalizedAndFullyOpaque();
	m_commandPromptColor = Vector4f(color.x,color.y,color.z,1.f);
	m_commandPromptQuad->quad.AppendQuadFromString(">",m_commandPromptColor);
	m_cursor.CreateVertices(0.f,m_commandPromptQuad,m_commandPromptColor);
}

//-----------------------------------------------------------------------------------------------
ConsoleLog::~ConsoleLog()
{
	OpenGLRenderer::DeleteBuffers( 1, &m_quadBufferId );
}

//-----------------------------------------------------------------------------------------------
bool ConsoleLog::ToggleLogToRender()
{
	m_showLog = !m_showLog;
	return m_showLog;
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::ConsolePrint(const std::string& textToRender, const Vector4f& fontColor
	, bool useDropShadow, const Vector4f& dropShadowColor)
{
	const Vector3f ZERO_VECTOR(0.f,0.f,0.f);

	DrawableTextQuad textQuad(m_fontSize,ZERO_VECTOR,m_fontToUse);

	textQuad.AppendQuadFromString(textToRender,fontColor);

	TextQuad quadInfo(textQuad,useDropShadow,dropShadowColor);

	m_logStack.push_back(quadInfo);

	m_stackIndex = (int)m_logStack.size() - 1;
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::ScrollUpLog()
{
	if(m_logStack.size() == 0)
		return;

	++m_stackIndex;

	if(m_stackIndex == (int)m_logStack.size())
		m_stackIndex = 0;
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::ScrollDownLog()
{
	if(m_logStack.size() == 0)
		return;

	--m_stackIndex;
	
	if(m_stackIndex < 0)
		m_stackIndex = m_logStack.size() - 1;
}

//-----------------------------------------------------------------------------------------------
int ConsoleLog::GetCommandPromptLength()
{
	return m_commandPromptQuad->quad.GetStringLength();
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::ClearCommandPrompt()
{
	m_commandPromptQuad->quad.ClearQuad();
	m_commandPromptQuad->quad.AppendQuadFromString(">",m_commandPromptColor);
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::ClearConsoleLog()
{
	m_logStack.clear();
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::PrintToCommandPrompt(const std::string& textToRender)
{
	m_commandPromptQuad->quad.AppendQuadFromString(textToRender,m_commandPromptColor);
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::Update(float deltaSeconds)
{
	m_cursor.Update(deltaSeconds,m_commandPromptQuad,m_commandPromptColor);
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::Render() const
{
	if(!m_showLog)
		return;

	RenderTranslucentQuads();
	OpenGLRenderer::RenderTextQuad(m_commandPromptQuad->quad, Vector3f(0.f,0.f,0.f),m_commandPromptQuad->shouldRenderDropShadow,m_commandPromptQuad->dropShadowColor);

	const float inverseAspectRatio = (float)OpenGLRenderer::s_screenSize.y / (float)OpenGLRenderer::s_screenSize.x;
	const float orthoWidth = 1024.f;
	const float orthoHeight = orthoWidth * inverseAspectRatio;
	const float startingY = orthoHeight / 4.f;

	Vector3f position(0.f,startingY,0.f);

	if(m_logStack.size() > 0)
	{

		for(int i = m_stackIndex; i >= 0; -- i)
		{
			if(position.y > orthoHeight - m_fontSize)
				break;
			OpenGLRenderer::RenderTextQuad(m_logStack[i].quad,position,m_logStack[i].shouldRenderDropShadow,m_logStack[i].dropShadowColor);	
			position.y += m_fontSize;
		}	

		for(int i = m_logStack.size() - 1; i > m_stackIndex; -- i)
		{
			if(position.y > orthoHeight - m_fontSize)
				break;

			OpenGLRenderer::RenderTextQuad(m_logStack[i].quad,position,m_logStack[i].shouldRenderDropShadow,m_logStack[i].dropShadowColor);	
			position.y += m_fontSize;	
		}
	}
	m_cursor.Render();
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::CreateTranslucentQuads()
{
	const Vector4f color(0.f,0.f,0.f,.66f);

	const float inverseAspectRatio = (float)OpenGLRenderer::s_screenSize.y / (float)OpenGLRenderer::s_screenSize.x;
	const float orthoWidth = 1024.f;
	const float orthoHeight = orthoWidth * inverseAspectRatio;
	const float startingY = orthoHeight / 4.f;

	VertexPositionColor vertices[NUM_QUAD_VERTICES];

	//-----------------------------------------------------------------------------------------------
	Vector3f minXminYPosition(0.f, startingY,0.f);
	Vector3f maxXminYPosition(orthoWidth, startingY, 0.f);
	Vector3f maxXmaxYPosition(orthoWidth, orthoHeight, 0.f);
	Vector3f minXmaxYPosition(0.f, orthoHeight, 0.f);

	vertices[0] = VertexPositionColor(minXminYPosition,color);
	vertices[1] = VertexPositionColor(maxXminYPosition,color);
	vertices[2] = VertexPositionColor(maxXmaxYPosition,color);
	vertices[3] = VertexPositionColor(minXmaxYPosition,color);
	//-----------------------------------------------------------------------------------------------

	minXminYPosition.y = 0.f;
	maxXminYPosition.y = 0.f;
	maxXmaxYPosition.y = m_fontSize;
	minXmaxYPosition.y = m_fontSize;

	vertices[4] = VertexPositionColor(minXminYPosition,color);
	vertices[5] = VertexPositionColor(maxXminYPosition,color);
	vertices[6] = VertexPositionColor(maxXmaxYPosition,color);
	vertices[7] = VertexPositionColor(minXmaxYPosition,color);

	CreateTranslucentQuadVBOs(vertices);
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::CreateTranslucentQuadVBOs(const VertexPositionColor (&vertices) [NUM_QUAD_VERTICES])
{
	OpenGLRenderer::GenBuffers(1, &m_quadBufferId);

	OpenGLRenderer::BindBuffer(GL_ARRAY_BUFFER,m_quadBufferId);

	int sizeOfData = NUM_QUAD_VERTICES * sizeof(VertexPositionColor);

	OpenGLRenderer::BufferData(GL_ARRAY_BUFFER,sizeOfData,vertices,GL_STATIC_DRAW);

	OpenGLRenderer::BindBuffer(GL_ARRAY_BUFFER,0);
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::RenderTranslucentQuads() const
{
	const float*colorOffset = (float*)offsetof(VertexPositionColor,VertexPositionColor::diffuseColor);
	const float* vertexOffset = (float*)offsetof(VertexPositionColor,VertexPositionColor::position);
	const int stride = sizeof(VertexPositionColor);

	const float inverseAspectRatio = (float)OpenGLRenderer::s_screenSize.y / (float)OpenGLRenderer::s_screenSize.x;
	const float orthoWidth = 1024;
	const float orthoHeight = orthoWidth * inverseAspectRatio;

	OpenGLRenderer::UseShaderProgram(OpenGLRenderer::s_fixedFunctionPipelineShaderID);
	OpenGLRenderer::Uniform1i( OpenGLRenderer::s_fixedFunctionUseTexturesUniformLocation , 0 );

	OpenGLRenderer::s_rendererStack.Push(MatrixStack44f::MODEL_STACK);

	OpenGLRenderer::Disable(GL_DEPTH_TEST);
	OpenGLRenderer::Disable(GL_CULL_FACE);

	OpenGLRenderer::s_rendererStack.ApplyOrtho(0.f, orthoWidth, 0.f, orthoHeight, 0.f, 1.f);
	OpenGLRenderer::LoadMatrix(OpenGLRenderer::s_rendererStack.GetMVWithProjection());

	OpenGLRenderer::BindBuffer(GL_ARRAY_BUFFER,m_quadBufferId);

	OpenGLRenderer::VertexPointer(3,GL_FLOAT,stride,vertexOffset);
	OpenGLRenderer::ColorPointer(4, GL_FLOAT,stride,colorOffset);

	OpenGLRenderer::EnableClientState(GL_VERTEX_ARRAY);
	OpenGLRenderer::EnableClientState(GL_COLOR_ARRAY);

	OpenGLRenderer::DrawArrays(GL_QUADS,0,NUM_QUAD_VERTICES);

	OpenGLRenderer::DisableClientState(GL_COLOR_ARRAY);
	OpenGLRenderer::DisableClientState(GL_VERTEX_ARRAY);

	//OpenGLRenderer::Enable(GL_CULL_FACE);
	//OpenGLRenderer::Enable(GL_DEPTH_TEST);

	OpenGLRenderer::s_rendererStack.Pop(MatrixStack44f::MODEL_STACK);
}

//-----------------------------------------------------------------------------------------------
ConsoleLog::Cursor::Cursor()
{	

}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::Cursor::CreateVertices(float deltaSeconds, const TextQuad* commandPrompt, const Vector4f& commandPromptColor)
{
	float xOffset = commandPrompt->quad.GetQuadLength();
	float fontSize = commandPrompt->quad.GetFontSize();

	static float transparency = 0.f;
	transparency += TWO_PI * deltaSeconds;

	Vector4f color = commandPromptColor;
	color.w = cos(transparency);
	//-----------------------------------------------------------------------------------------------
	Vector3f minXminYPosition(xOffset, 0.f,0.f);
	Vector3f maxXminYPosition(xOffset + fontSize * 0.05f, 0.f, 0.f);
	Vector3f maxXmaxYPosition(xOffset + fontSize * 0.05f, fontSize, 0.f);
	Vector3f minXmaxYPosition(xOffset, fontSize, 0.f);

	m_positionArray.push_back(minXminYPosition);
	m_positionArray.push_back(maxXminYPosition);
	m_positionArray.push_back(maxXmaxYPosition);
	m_positionArray.push_back(minXmaxYPosition);

	//-----------------------------------------------------------------------------------------------

	m_colorArray.push_back(color);
	m_colorArray.push_back(color);
	m_colorArray.push_back(color);
	m_colorArray.push_back(color);
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::Cursor::Update(float deltaSeconds, const TextQuad* commandPrompt, const Vector4f& commandPromptColor)
{
	m_positionArray.clear();
	m_colorArray.clear();

	CreateVertices(deltaSeconds, commandPrompt, commandPromptColor);
}

//-----------------------------------------------------------------------------------------------
void ConsoleLog::Cursor::Render() const
{	
	const float inverseAspectRatio = (float)OpenGLRenderer::s_screenSize.y / (float)OpenGLRenderer::s_screenSize.x;
	const float orthoWidth = 1024;
	const float orthoHeight = orthoWidth * inverseAspectRatio;

	OpenGLRenderer::UseShaderProgram(OpenGLRenderer::s_fixedFunctionPipelineShaderID);
	OpenGLRenderer::Uniform1i( OpenGLRenderer::s_fixedFunctionUseTexturesUniformLocation , 0 );

	OpenGLRenderer::s_rendererStack.Push(MatrixStack44f::MODEL_STACK);

	OpenGLRenderer::Disable(GL_DEPTH_TEST);
	OpenGLRenderer::Disable(GL_CULL_FACE);
	OpenGLRenderer::s_rendererStack.ApplyOrtho(0.f, orthoWidth, 0.f, orthoHeight, 0.f, 1.f);
	OpenGLRenderer::LoadMatrix(OpenGLRenderer::s_rendererStack.GetMVWithProjection());

	OpenGLRenderer::BindBuffer(GL_ARRAY_BUFFER,0);

	OpenGLRenderer::VertexPointer(3,GL_FLOAT,0,m_positionArray.data());
	OpenGLRenderer::ColorPointer(4, GL_FLOAT,0,m_colorArray.data());

	OpenGLRenderer::EnableClientState(GL_VERTEX_ARRAY);
	OpenGLRenderer::EnableClientState(GL_COLOR_ARRAY);

	OpenGLRenderer::DrawArrays(GL_QUADS,0,m_positionArray.size());

	OpenGLRenderer::DisableClientState(GL_COLOR_ARRAY);
	OpenGLRenderer::DisableClientState(GL_VERTEX_ARRAY);

	//OpenGLRenderer::Enable(GL_CULL_FACE);
	//OpenGLRenderer::Enable(GL_DEPTH_TEST);

	OpenGLRenderer::s_rendererStack.Pop(MatrixStack44f::MODEL_STACK);
}

//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
void ConsoleLog::EnterCommand()
{
	const std::string quadString = m_commandPromptQuad->quad.GetString();
	std::string commandString;
	commandString.resize(quadString.size() - 1);
	for(unsigned int i = 1; i < quadString.size(); ++i)
		commandString[i-1] = quadString[i];


	FireEvent(commandString, this);
}