#ifndef OPENGLRENDERER
#define OPENGLRENDERER

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <vector>
#include <gl/gl.h>
#include <string>

#include "Engine/Rendering/glext.h"
#include "Engine/Rendering/DrawableTextQuad.hpp"
#include "Engine/Primitives/Color.hpp"
#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Primitives/Vector4.hpp"
#include "Engine/Primitives/Matrix44.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Physics/AABB3.hpp"


const std::string PROJECT_DELIMITER = "\\ShaderProject1\\";
const uint UNASSINGED_BUFFERID = 2147483647;
const Vector4f NO_COLOR = Vector4f( -1.f , -1.f, -1.f, 0.f );

#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library

class BitmapFont;

class OpenGLRenderer
{

public:

	enum QuadDirection 
	{				   
		NONE = -1,	   
		EAST,		   
		NORTH,		   
		WEST,		   
		SOUTH,		   
		TOP,		   
		BOTTOM		  
	};

	enum DrawMode
	{
		DEPTH_TEST,
		NO_DEPTH_TEST,
		DUAL_MODE
	};

	enum TriangleDensityLevel
	{
		LOW = 6,
		MID = 10,
		HIGH = 20,
		VERY_HIGH = 30
	};

	enum CrosshairType
	{
		CROSSHAIR_CROSS,
		CROSS_HAIR_DOT
	};

	static void CreateOpenGLWindow(HINSTANCE applicationInstanceHandle, int screenWidth, int screenHeight);
	
	static void Begin(uint mode);
	static void End();
	static void PushMatrix();
	static void PopMatrix();

	static void Color3f(float r, float g, float b);
	static void Color4f(float r, float g, float b, float a);
	static void Vertex2f(float x, float y);
	static void Vertex3f(float x, float y, float z);
	static void TexCoord2f(float x, float y);
	static void LineWidth(float width);

	static void Scalef(float x, float y, float z);
	static void Rotatef(float angle, float x, float y, float z);
	static void Translatef(float x, float y, float z);

	static void Enable(uint state);
	static void Disable(uint state);

	static void PixelStorei(uint pname, int param);
	static void GenTextures(int numberOfTextures, uint* bufferId);
	static void BindTexture(uint target, uint texture);
	static void TexParameteri(uint target, uint pname, int param);
	static void TexImage2D(uint target, int level, int internalformat, int width, int height, int border, uint format, uint type, const void *pixels);
	static void GenerateMipmap(uint mode);

	static void GenBuffers(int n, uint *buffers);
	static void BindBuffer(uint target, uint buffer);
	static void BufferData(uint target, int size, const void * data, uint usage);
	static void DeleteBuffers(int n, const uint *buffers);

	static void EnableClientState(uint arrayToEnable);
	static void DisableClientState(uint arrayToDisable);
	static void DrawArrays(uint mode, int first, int count);
	static void DrawElements(uint mode, int count, uint type, const void* indices);

	static void VertexPointer(int size, uint type, int stride, const void *pointer);
	static void ColorPointer(int size, uint type, int stride, const void *pointer);
	static void TexCoordPointer(int size, uint type, int stride, const void *pointer);

	static void ClearColor(float r, float g, float b, float a);
	static void Clear(uint mask);
	static void ClearDepth(double depth);
	static void BlendFunc(uint sfactor, uint dfactor);
	static void DepthMask(uchar flag);
	static void FrontFace(uint mode);

	static void Perspective(double fovy, double aspect, double zNear, double zFar);
	static void Ortho(double left, double right, double bottom, double top, double zNear, double zFar);
	static void LoadMatrix(const Matrix44f& matrix);
	static void MultMatrix(const Matrix44f& matrix);
	static void GetCurrentOpenGLMatrix(uint matrixType, float* matrixData);

	static void RenderErrorWindow( const std::string& errorMessage, const std::string& optionalCaption = "" );
	static HWND GetWindow();
	static void RenderSolidCube(const Vector3f& minCoords, float scale = 1.f);
	static void RenderWireFrameCube(const Vector3f& minCoords, float scale = 1.f);
	static void RenderQuad(const Vector3f& minCoords, QuadDirection direction, float scale = 1.f);
	static void RenderWireFrameBox(const Vector3f& minCoords, const Vector3f& maxCoords);
	static void RenderSolidBox(const Vector3f& minCoords, const Vector3f& maxCoords);
	static void Render2DQuad( const Vector2f& minCoords, const Vector2f& maxCoords, const Vector3f& color );
	static void RenderCrossHair( CrosshairType typeOfCrossHair = CROSSHAIR_CROSS, const Vector4f& color = NO_COLOR );
	static void RenderText(const std::string& textToRender, BitmapFont* fontToUse, float fontSize, const Vector3f& minScreenPosition,
		const Vector4f& fontColor = Vector4f(1.f,1.f,1.f,1.f), bool useDropShadow = false, const Vector4f& dropShadowColor = Vector4f(0.f,0.f,0.f,1.f));

	static void RenderTextQuad(const DrawableTextQuad& quad, const Vector3f& additionalTranslation = Vector3f(0.f,0.f,0.f), bool useDropShadow = false
		, const Vector4f& dropShadowColor = Vector4f(0.f,0.f,0.f,1.f));

	static float CalcTextWidth(const std::string& textToRender, BitmapFont* fontToUse, float fontSize);

	static void Update(float deltaSeconds);
	static void RenderDebugShapes();
	static void RenderOriginBasisAxes(float length = 1.f, DrawMode drawMode= DEPTH_TEST, float duration = 0.f);
	static void RenderDebugPoint(const Vector3f& position, const Color& color, DrawMode drawMode = DEPTH_TEST, float duration = 0.f);
	static void RenderDebugLine(const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, DrawMode drawMode = DEPTH_TEST, float duration = 0.f);
	static void RenderDebugArrow(const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, DrawMode drawMode = DEPTH_TEST, float duration = 0.f);
	static void RenderDebugAABB3(const AABB3& boundingBox, const Color& wireframeColor, const Color& faceColor, DrawMode drawMode = DEPTH_TEST, float duration = 0.f);
	static void RenderDebugUVSphere(const Vector3f& center, float radius, const Color& wireframeColor, const Color& faceColor, DrawMode drawMode = DEPTH_TEST, float duration = 0.f, TriangleDensityLevel density = MID);

	static uint CreateShaderProgramAndGetID(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	static void UseShaderProgram(uint shaderProgramID);
	static uint GetCurrentShaderProgram();
	static int GetUniformLocation(int programID, const char* variableName);
	static void Uniform1i(int location, int value);
	static void Uniform1f(int location, float value);
	static void Uniform4f(int location, float value1, float value2, float value3, float value4);
	static void Uniform3f(int location, float value1, float value2, float value3);
	static void Uniform4fv(int location, int count, const float *value);
	static void Uniform3fv(int location, int count, const float *value);
	static void UniformMatrix4fv(int location, int count, bool transpose, const float *value);
	static int GetAttribLocation(uint programID, const char* variableName);

	static void VertexAttribPointer (uint index, int size, uint type, bool normalized, int stride, const void * pointer);
	static void EnableVertexAttribArray(uint index);
	static void DisableVertexAttribArray(uint index);
	static void VertexAttrib4f(uint index, float value0, float value1, float value2, float value3);
	static void VertexAttrib3f(uint index, float value0, float value1, float value2);
	static void VertexAttrib2f(uint index, float value0, float value1);
	static void ActiveTexture(uint textureID);

	static HCURSOR cursor;
	static HDC displayDeviceContext;
	static Vector2i s_screenSize;
	static uint s_fixedFunctionPipelineShaderID;
	static int s_fixedFunctionUseTexturesUniformLocation;
	static int s_fixedFunctionDiffuseTextureUniformLocation;
	static uint s_modernShaderID;
	static MatrixStack44f s_rendererStack;
	static Vector4f s_clearColor;

private:

	static GLuint	CreateShaderAndGetID(const std::string& shaderFilePath, uint shaderType, const std::string& folderNameDelimiter);
	static void		ReportShaderErrors(const std::string& shaderFilePath, const std::string& folderNameDelimiter, GLint logLength, const char* logText);
	static void		ReportLinkerErrors(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, GLint logLength, const char* logText);
	static void		EnableAndBindGLFunctions();

	class DebugShape
	{

	public:
		virtual void Render() const = 0;
		OpenGLRenderer::DrawMode m_drawMode;
		float m_duration;
	};

	class DebugPoint : public DebugShape
	{

	public:
		DebugPoint(const Vector3f& position, const Color& color, DrawMode drawMode, float duration);
		virtual void Render() const;
		void DrawPoint(float lineWidth) const;

		Vector3f m_position;
		Vector3f m_color;
	};

	class DebugLine : public DebugShape
	{
	public:
		DebugLine(const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, DrawMode drawMode, float duration);
		virtual void Render() const;
		void DrawLine(float lineWidth) const;

		Vector3f m_startPosition;
		Vector3f m_endPosition;
		Vector3f m_startColor;
		Vector3f m_endColor;
	};

	class DebugArrow : public DebugLine
	{
	public:
		DebugArrow(const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, DrawMode drawMode, float duration);
		virtual void Render() const;
		void DrawArrow(float alpha) const;

		Matrix44f m_arrowTransform;
	};

	class DebugAABB3 : public DebugShape
	{
	public:
		DebugAABB3(const AABB3& boundingBox, const Color& wireframeColor, const Color& faceColor, DrawMode drawMode, float duration);
		virtual void Render() const;
		void DrawWireFrameBox(float lineWidth) const;
		void DrawSolidBox(float alpha) const;

		Vector3f m_minCoords;
		Vector3f m_maxCoords;
		Vector3f m_wireframeColor;
		Vector3f m_faceColor;
	};

	class DebugUVSphere : public DebugShape
	{
	public:

		DebugUVSphere(const Vector3f& center, float radius, const Color& wireframeColor, const Color& faceColor, DrawMode drawMode, float duration, TriangleDensityLevel density);
		virtual void Render() const;
		void DrawWireFrameUVSphere(float lineWidth) const;
		void DrawSolidUVSphere(float alpha) const;

		Vector3f m_centerPoint;
		float m_radius;
		Vector3f m_wireframeColor;
		Vector3f m_faceColor;
		TriangleDensityLevel m_density;
	};

	static std::vector<DebugShape*> m_debugShapeVector;
	static uint currentShaderProgram;
};


#endif