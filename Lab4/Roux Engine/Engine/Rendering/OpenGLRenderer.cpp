#include "OpenGLRenderer.hpp"

#include <assert.h>

#include "gl/GLU.h"

#include "Engine/Rendering/BitmapFont.hpp"
#include "Engine/Rendering/geometry.h"
#include "Engine/Utilities/InputHandler.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"

#include "Engine/Utilities/NewAndDeleteOverrides.hpp"

#define UNUSED(x) (void)(x);

#pragma comment( lib, "glu32" )

//-----------------------------------------------------------------------------------------------
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;
PFNGLDELETEBUFFERSPROC	glDeleteBuffers = nullptr;

PFNGLCREATESHADERPROC	glCreateShader	= nullptr;
PFNGLSHADERSOURCEPROC	glShaderSource	= nullptr;
PFNGLCOMPILESHADERPROC	glCompileShader = nullptr;
PFNGLGETSHADERIVPROC	glGetShaderiv	= nullptr;
PFNGLGETSHADERINFOLOGPROC	glGetShaderInfoLog = nullptr;
PFNGLGETPROGRAMINFOLOGPROC	glGetProgramInfoLog = nullptr;
PFNGLCREATEPROGRAMPROC	glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC	glAttachShader	= nullptr;
PFNGLLINKPROGRAMPROC	glLinkProgram	= nullptr;
PFNGLGETPROGRAMIVPROC	glGetProgramiv	= nullptr;
PFNGLUSEPROGRAMPROC		glUseProgram	= nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation	= nullptr;
PFNGLUNIFORM1FPROC	glUniform1f	= nullptr;
PFNGLUNIFORM3FPROC	glUniform3f	= nullptr;
PFNGLUNIFORM4FPROC	glUniform4f	= nullptr;
PFNGLUNIFORM1IPROC	glUniform1i = nullptr;
PFNGLUNIFORM4FVPROC glUniform4fv = nullptr;
PFNGLUNIFORM3FVPROC glUniform3fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;

PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f = nullptr;
PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f = nullptr;
PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f = nullptr;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = nullptr;



HDC OpenGLRenderer::displayDeviceContext = nullptr;
HCURSOR OpenGLRenderer::cursor;
uint OpenGLRenderer::s_fixedFunctionPipelineShaderID;
int OpenGLRenderer::s_fixedFunctionUseTexturesUniformLocation = -1;
int OpenGLRenderer::s_fixedFunctionDiffuseTextureUniformLocation = -1;
uint OpenGLRenderer::s_modernShaderID;
MatrixStack44f OpenGLRenderer::s_rendererStack;

HWND g_hWnd;
HGLRC g_openGLRenderingContext;
Vector2i OpenGLRenderer::s_screenSize(0,0);
std::vector<OpenGLRenderer::DebugShape*> OpenGLRenderer::m_debugShapeVector;
uint OpenGLRenderer::currentShaderProgram = 0;
uint g_fontShaderID;
Vector4f OpenGLRenderer::s_clearColor = Vector4f(0.f,0.f,0.f,1.f);

const char* APP_NAME = "Free Falling RedFish";



//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::CreateOpenGLWindow(HINSTANCE applicationInstanceHandle, int screenWidth, int screenHeight)
{
	s_screenSize.x = screenWidth;
	s_screenSize.y = screenHeight;

	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast< WNDPROC >(InputHandler::WindowsMessageHandlingProcedure ); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );

	RECT windowRect = {50, 50, 50 + screenWidth, 50 + screenHeight };
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[ 1024 ];
	MultiByteToWideChar( GetACP(), 0, APP_NAME, -1, windowTitle, sizeof(windowTitle)/sizeof(windowTitle[0]) );
	g_hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL );

	ShowWindow( g_hWnd, SW_SHOW );
	SetForegroundWindow( g_hWnd );
	SetFocus( g_hWnd );

	displayDeviceContext = GetDC( g_hWnd );

	cursor = LoadCursor( NULL, IDC_HAND );
	SetCursor( cursor );
	ShowCursor(false);

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset( &pixelFormatDescriptor, 0, sizeof( pixelFormatDescriptor ) );
	pixelFormatDescriptor.nSize			= sizeof( pixelFormatDescriptor );
	pixelFormatDescriptor.nVersion		= 1;
	pixelFormatDescriptor.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType	= PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits	= 24; //3B
	pixelFormatDescriptor.cDepthBits	= 24; //3B
	pixelFormatDescriptor.cAccumBits	= 0;
	pixelFormatDescriptor.cStencilBits	= 8;

	int pixelFormatCode = ChoosePixelFormat( displayDeviceContext, &pixelFormatDescriptor );
	SetPixelFormat( displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor );
	g_openGLRenderingContext = wglCreateContext( displayDeviceContext );
	wglMakeCurrent( displayDeviceContext, g_openGLRenderingContext );

	EnableAndBindGLFunctions();

	// Set up the OPenGl states the way we want 'em
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	g_fontShaderID = CreateShaderProgramAndGetID("Data\\Shaders\\Font_Vertex_Shader_110.glsl", "Data\\Shaders\\Font_Fragment_Shader_110.glsl");
	s_fixedFunctionPipelineShaderID = CreateShaderProgramAndGetID("Data\\Shaders\\FFP_Vertex_110.glsl", "Data\\Shaders\\FFP_Fragment_110.glsl");
	s_modernShaderID = CreateShaderProgramAndGetID("Data\\Shaders\\Vertex_Shader_330.glsl", "Data\\Shaders\\Fragment_Shader_330.glsl");
	s_fixedFunctionUseTexturesUniformLocation = GetUniformLocation( s_fixedFunctionPipelineShaderID, "u_useTextures" );
	s_fixedFunctionDiffuseTextureUniformLocation = GetUniformLocation( s_fixedFunctionPipelineShaderID, "u_diffuseTexture" );

	Color::Initialize();
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Begin(uint mode)
{
	glBegin(mode);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::End()
{
	glEnd();
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::PushMatrix()
{
	glPushMatrix();
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::PopMatrix()
{
	glPopMatrix();
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Color3f(float r, float g, float b)
{
	glColor3f(r, g, b);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Color4f(float r, float g, float b, float a)
{
	glColor4f(r, g, b, a);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Vertex2f(float x, float y)
{
	glVertex2f(x, y);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Vertex3f(float x, float y, float z)
{
	glVertex3f(x, y, z);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::TexCoord2f(float x, float y)
{
	glTexCoord2f(x, y);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::LineWidth(float width)
{
	glLineWidth(width);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Scalef(float x, float y, float z)
{
	glScalef(x, y, z);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Rotatef(float angle, float x, float y, float z)
{
	glRotatef(angle, x, y, z);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Translatef(float x, float y, float z)
{
	glTranslatef(x, y, z);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Enable(uint state)
{
	glEnable(state);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Disable(uint state)
{
	glDisable(state);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::PixelStorei(uint pname, int param)
{
	glPixelStorei(pname, param);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::GenTextures(int numberOfTextures, uint* bufferId)
{
	glGenTextures(numberOfTextures, bufferId);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::BindTexture(uint target, uint texture)
{
	glBindTexture(target, texture);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::TexParameteri(uint target, uint pname, int param)
{
	glTexParameteri(target, pname, param);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::TexImage2D(uint target, int level, int internalformat, int width, int height, int border, uint format, uint type, const void *pixels)
{
	glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::GenerateMipmap(uint mode)
{
	glGenerateMipmap(mode);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::GenBuffers(int n, uint *buffers)
{
	if( *buffers == UNASSINGED_BUFFERID )
		glGenBuffers(n, buffers);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::BindBuffer(uint target, uint buffer)
{
	glBindBuffer(target, buffer);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::BufferData(uint target, int size, const void * data, uint usage)
{
	glBufferData(target, size, data, usage);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::DeleteBuffers(int n, const uint *buffers)
{
	glDeleteBuffers(n, buffers);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::EnableClientState(uint arrayToEnable)
{
	glEnableClientState(arrayToEnable);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::DisableClientState(uint arrayToDisable)
{
	glDisableClientState(arrayToDisable);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::DrawArrays(uint mode, int first, int count)
{
	glDrawArrays(mode, first, count);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::DrawElements(uint mode, int count, uint type, const void* indices)
{
	glDrawElements(mode, count, type, indices);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::VertexPointer(int size, uint type, int stride, const void *pointer)
{
	glVertexPointer(size, type, stride, pointer);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::ColorPointer(int size, uint type, int stride, const void *pointer)
{
	glColorPointer(size, type, stride, pointer);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::TexCoordPointer(int size, uint type, int stride, const void *pointer)
{
	glTexCoordPointer(size, type, stride, pointer);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::ClearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Clear(uint mask)
{
	glClear(mask);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::ClearDepth(double depth)
{
	glClearDepth(depth);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::BlendFunc(uint sfactor, uint dfactor)
{
	glBlendFunc(sfactor, dfactor);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::DepthMask(uchar flag)
{
	glDepthMask(flag);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::FrontFace(uint mode)
{
	glFrontFace(mode);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Perspective(double fovy, double aspect, double zNear, double zFar)
{
	gluPerspective(fovy, aspect, zNear, zFar);
	//Matrix44f m = Matrix44f::GetPerspectiveMatrix((float)fovy, (float)aspect, (float)zNear, (float)zFar);
	//MultMatrix(m);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Ortho(double left, double right, double bottom, double top, double zNear, double zFar)
{
	glOrtho(left, right, bottom, top, zNear, zFar);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::LoadMatrix(const Matrix44f& matrix)
{
	glLoadMatrixf(matrix.data);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::MultMatrix(const Matrix44f& matrix)
{
	glMultMatrixf(matrix.data);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::GetCurrentOpenGLMatrix(uint matrixType, float* matrixData)
{
	glGetFloatv(matrixType,matrixData);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderErrorWindow( const std::string& errorMessage, const std::string& optionalCaption )
{
	MessageBoxA(g_hWnd,errorMessage.c_str(),optionalCaption.c_str(), MB_OK);
}

//-----------------------------------------------------------------------------------------------
HWND OpenGLRenderer::GetWindow()
{
	return GetActiveWindow();
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderSolidCube(const Vector3f& minCoords, float scale)
{
	s_rendererStack.Push(MatrixStack44f::MODEL_STACK);

	glScalef(scale, scale, scale);

	glBegin(GL_QUADS);
	{
		//front

		glVertex3f(minCoords.x, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z + 1.f);
		glVertex3f(minCoords.x, minCoords.y, minCoords.z + 1.f);
		//back

		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z + 1.f);
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z + 1.f);

		//left

		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x, minCoords.y, minCoords.z + 1.f);
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z + 1.f);

		//right

		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z + 1.f);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z + 1.f);

		//top

		glVertex3f(minCoords.x, minCoords.y, minCoords.z + 1.f);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z + 1.f);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z + 1.f);
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z + 1.f);

		//bottom

		glVertex3f(minCoords.x, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
	}
	glEnd();

	s_rendererStack.Pop(MatrixStack44f::MODEL_STACK);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderWireFrameCube(const Vector3f& minCoords, float scale)
{
	s_rendererStack.Push(MatrixStack44f::MODEL_STACK);

	glScalef(scale, scale, scale);

	glDisable(GL_TEXTURE_2D);

	//front
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z + 1.f);
		glVertex3f(minCoords.x, minCoords.y, minCoords.z + 1.f);
	}
	glEnd();

	//back
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z + 1.f);
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z + 1.f);
	}
	glEnd();

	//left
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x, minCoords.y, minCoords.z + 1.f);
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z + 1.f);
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
	}
	glEnd();

	//right
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z + 1.f);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z + 1.f);
	}
	glEnd();

	//top
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, minCoords.y, minCoords.z + 1.f);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z + 1.f);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z + 1.f);
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z + 1.f);
	}
	glEnd();

	//bottom
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z);
		glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
	}
	glEnd();

	glEnable(GL_TEXTURE_2D);

	s_rendererStack.Pop(MatrixStack44f::MODEL_STACK);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderQuad(const Vector3f& minCoords, QuadDirection direction, float scale)
{
	s_rendererStack.Push(MatrixStack44f::MODEL_STACK);

	glScalef(scale, scale, scale);

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	{
		switch(direction)
		{
		case OpenGLRenderer::EAST:
			glVertex3f(minCoords.x, minCoords.y, minCoords.z);
			glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z);
			glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z + 1.f);
			glVertex3f(minCoords.x, minCoords.y, minCoords.z + 1.f);
			break;
		case OpenGLRenderer::NORTH:
			glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z);
			glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z);
			glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z + 1.f);
			glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z + 1.f);
			break;
		case OpenGLRenderer::WEST:
			glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
			glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z);
			glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z + 1.f);
			glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z + 1.f);
			break;
		case OpenGLRenderer::SOUTH:
			glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
			glVertex3f(minCoords.x, minCoords.y, minCoords.z);
			glVertex3f(minCoords.x, minCoords.y, minCoords.z + 1.f);
			glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z + 1.f);
			break;
		case OpenGLRenderer::TOP:
			glVertex3f(minCoords.x, minCoords.y, minCoords.z + 1.f);
			glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z + 1.f);
			glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z + 1.f);
			glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z + 1.f);
			break;
		case OpenGLRenderer::BOTTOM:
			glVertex3f(minCoords.x, minCoords.y, minCoords.z);
			glVertex3f(minCoords.x + 1.f, minCoords.y, minCoords.z);
			glVertex3f(minCoords.x + 1.f, minCoords.y + 1.f, minCoords.z);
			glVertex3f(minCoords.x, minCoords.y + 1.f, minCoords.z);
			break;
		}
	}
	glEnd();

	glEnable(GL_TEXTURE_2D);

	s_rendererStack.Pop(MatrixStack44f::MODEL_STACK);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderWireFrameBox(const Vector3f& minCoords, const Vector3f& maxCoords)
{
	glDisable(GL_TEXTURE_2D);

	//front
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, minCoords.y, minCoords.z);
		glVertex3f(maxCoords.x, minCoords.y, minCoords.z);
		glVertex3f(maxCoords.x, minCoords.y, maxCoords.z);
		glVertex3f(minCoords.x, minCoords.y, maxCoords.z);
	}
	glEnd();

	//back
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, maxCoords.y, minCoords.z);
		glVertex3f(maxCoords.x, maxCoords.y, minCoords.z);
		glVertex3f(maxCoords.x, maxCoords.y, maxCoords.z);
		glVertex3f(minCoords.x, maxCoords.y, maxCoords.z);
	}
	glEnd();

	//left
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, maxCoords.y, minCoords.z);
		glVertex3f(minCoords.x, minCoords.y, minCoords.z);
		glVertex3f(minCoords.x, minCoords.y, maxCoords.z);
		glVertex3f(minCoords.x, maxCoords.y, maxCoords.z);
		glVertex3f(minCoords.x, maxCoords.y, minCoords.z);
	}
	glEnd();

	//right
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(maxCoords.x, minCoords.y, minCoords.z);
		glVertex3f(maxCoords.x, maxCoords.y, minCoords.z);
		glVertex3f(maxCoords.x, maxCoords.y, maxCoords.z);
		glVertex3f(maxCoords.x, minCoords.y, maxCoords.z);
	}
	glEnd();

	//top
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, minCoords.y, maxCoords.z);
		glVertex3f(maxCoords.x, minCoords.y, maxCoords.z);
		glVertex3f(maxCoords.x, maxCoords.y, maxCoords.z);
		glVertex3f(minCoords.x, maxCoords.y, maxCoords.z);
	}
	glEnd();

	//bottom
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(minCoords.x, minCoords.y, minCoords.z);
		glVertex3f(maxCoords.x, minCoords.y, minCoords.z);
		glVertex3f(maxCoords.x, maxCoords.y, minCoords.z);
		glVertex3f(minCoords.x, maxCoords.y, minCoords.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Render2DQuad( const Vector2f& minCoords, const Vector2f& maxCoords, const Vector3f& color )
{
	glBegin( GL_QUADS );
	{
		glColor3f( color.x, color.y, color.z );
		glVertex2f( minCoords.x, minCoords.y );
		glVertex2f( maxCoords.x, minCoords.y );
		glVertex2f( maxCoords.x, maxCoords.y );
		glVertex2f( minCoords.x, maxCoords.y );
	}
	glEnd();
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderCrossHair( CrosshairType typeOfCrossHair, const Vector4f& color )
{
	UseShaderProgram( s_fixedFunctionPipelineShaderID );
	Uniform1i( s_fixedFunctionUseTexturesUniformLocation, 0 );

	const float screenWidth		= s_screenSize.x / 10.f;
	const float screenHeight	= s_screenSize.y / 10.f;
	const float halfWidth		= screenWidth * 0.5f;
	const float halfHeight		= screenHeight * 0.5f;
	float lineWidth				= 3.f;

	s_rendererStack.Push( MatrixStack44f::MODEL_STACK );
	{
		Disable( GL_DEPTH_TEST );
		Disable( GL_CULL_FACE );
		
		if( color == NO_COLOR )
		{
			BlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ZERO );
			Color3f( 1.f,1.f,1.f );
		}
		else
		{
			Color4f( color.x, color.y, color.z, color.w );
		}

		s_rendererStack.ApplyOrtho( 0.f, screenWidth, 0.f, screenHeight, 0.f, 1.f ) ;
		LoadMatrix( s_rendererStack.GetMVWithProjection() );

		if(	typeOfCrossHair == CROSSHAIR_CROSS )
		{
			LineWidth( lineWidth );
			Begin( GL_LINES );
			{
				Vertex2f( halfWidth + 0.5f, halfHeight );
				Vertex2f( halfWidth + 2.f, halfHeight );

				Vertex2f( halfWidth, halfHeight + 0.5f );
				Vertex2f( halfWidth, halfHeight + 2.f );

				Vertex2f( halfWidth - 0.5f, halfHeight );
				Vertex2f( halfWidth - 2.f, halfHeight );

				Vertex2f( halfWidth, halfHeight - 0.5f );
				Vertex2f( halfWidth, halfHeight - 2.f );
			}
			End();
		}
		else if( typeOfCrossHair == CROSS_HAIR_DOT )
		{
			const Vector2f baseCoords( halfWidth, halfHeight );

			//North
			Render2DQuad( baseCoords + Vector2f( -0.1f, 0.f), baseCoords + Vector2f( 0.1f, 0.2f), Vector3f( 1.f, 1.f, 1.f ) );
			//East
			Render2DQuad( baseCoords + Vector2f( 0.f, -0.1f), baseCoords + Vector2f( 0.2f, 0.1f), Vector3f( 1.f, 1.f, 1.f ) );
			//South
			Render2DQuad( baseCoords + Vector2f( -0.1f, -0.2f), baseCoords + Vector2f( 0.1f, 0.0f), Vector3f( 1.f, 1.f, 1.f ) );
			//West
			Render2DQuad( baseCoords + Vector2f( -0.2f, -0.1f ), baseCoords + Vector2f( 0.0f, 0.1f), Vector3f( 1.f, 1.f, 1.f ) );
			Render2DQuad( baseCoords + Vector2f( -0.1f, -0.1f ), baseCoords + Vector2f( 0.1f, 0.1f ), Vector3f( 1.f, 1.f, 1.f ) );

			////NorthEast
			//Render2DQuad( baseCoords + Vector2f( 0.05f, 0.05f ), baseCoords + Vector2f( 0.15f, 0.15f) );
			////SouthEast
			//Render2DQuad( baseCoords + Vector2f( 0.05f, -0.15f ), baseCoords + Vector2f( 0.15f, -0.05f) );
			////SouthWest
			//Render2DQuad( baseCoords + Vector2f( -0.15f, -0.15f ), baseCoords + Vector2f( -0.05f, -0.05f) );
			////NorthWest
			//Render2DQuad( baseCoords + Vector2f( -0.15f, 0.05f ), baseCoords + Vector2f( -0.05f, 0.15f) );
		}


		if( color == NO_COLOR )
			BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		Enable( GL_CULL_FACE );
		Enable( GL_DEPTH_TEST );	
	}	
	s_rendererStack.Pop( MatrixStack44f::MODEL_STACK );
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderText(const std::string& textToRender, BitmapFont* fontToUse, float fontSize, const Vector3f& minScreenPosition,
	const Vector4f& fontColor, bool useDropShadow , const Vector4f& dropShadowColor)
{

	DrawableTextQuad textQuad(fontSize, minScreenPosition, fontToUse);
	textQuad.AppendQuadFromString(textToRender,fontColor);
	textQuad.Render(g_fontShaderID,Vector3f(0.f,0.f,0.f),useDropShadow,dropShadowColor);

	DeleteBuffers(1,&(textQuad.m_bufferId));
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderTextQuad(const DrawableTextQuad& quad, const Vector3f& additionalTranslation, bool useDropShadow
	,const Vector4f& dropShadowColor)
{
	quad.Render(g_fontShaderID, additionalTranslation, useDropShadow, dropShadowColor);
}

//-----------------------------------------------------------------------------------------------
float OpenGLRenderer::CalcTextWidth(const std::string& textToRender, BitmapFont* fontToUse, float fontSize)
{
	std::map<uchar, BitmapFont::Glyph>::iterator foundIter;
	float lengthOfText = 0.f;

	for(auto iter = textToRender.begin(); iter != textToRender.end(); ++ iter)
	{
		foundIter = fontToUse->m_glyphs.find(*iter);

		if(foundIter != fontToUse->m_glyphs.end())
		{
			BitmapFont::Glyph currentGlyph = foundIter->second;
			if(currentGlyph.m_preDrawSpacing > 0.f)
			{
				lengthOfText += currentGlyph.m_preDrawSpacing * fontSize;
			}
			lengthOfText += currentGlyph.m_drawSpacing * fontSize;

			if(currentGlyph.m_postDrawSpacing > 0.f)
			{
				lengthOfText += currentGlyph.m_postDrawSpacing * fontSize;
			}
		}
	}
	return lengthOfText;
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Update(float deltaSeconds)
{
	for(auto iter = m_debugShapeVector.begin(); iter != m_debugShapeVector.end(); ++ iter)
	{
		(*iter)->m_duration -= deltaSeconds;
	}
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderDebugShapes()
{
	UseShaderProgram( s_fixedFunctionPipelineShaderID );
	for(int i = 0; i < (int)m_debugShapeVector.size(); ++ i)
	{
		Uniform1i( s_fixedFunctionDiffuseTextureUniformLocation ,0 );
		Uniform1i( s_fixedFunctionUseTexturesUniformLocation , 0 );

		m_debugShapeVector[i]->Render();

		if(m_debugShapeVector[i]->m_duration <= 0.f)
		{
			DebugShape* temp = m_debugShapeVector.back();
			if(i != (int)m_debugShapeVector.size() - 1)
			{	
				delete m_debugShapeVector[i];
				m_debugShapeVector[i] = temp;
				temp = nullptr;
				--i;
			}
			delete temp;
			m_debugShapeVector.pop_back();
		}
	}
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderOriginBasisAxes(float length, DrawMode drawMode, float duration)
{
	UseShaderProgram(s_fixedFunctionPipelineShaderID);

	const Vector3f ORIGIN(0.f, 0.f, 0.f);
	Vector3f END_X(1.f * length, 0.f, 0.f);
	Vector3f END_Y(0.f, 1.f * length, 0.f);
	Vector3f END_Z(0.f, 0.f, 1.f * length);

	const Color X_COLOR = Color(Red);
	const Color Y_COLOR = Color(Green);
	const Color Z_COLOR = Color(Blue);

	RenderDebugLine(ORIGIN, END_X, X_COLOR, X_COLOR, drawMode, duration);
	RenderDebugLine(ORIGIN, END_Y, Y_COLOR, Y_COLOR, drawMode, duration);
	RenderDebugLine(ORIGIN, END_Z, Z_COLOR, Z_COLOR, drawMode, duration);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderDebugPoint(const Vector3f& position, const Color& color, DrawMode drawMode, float duration)
{
	m_debugShapeVector.push_back(new DebugPoint(position,color,drawMode,duration));
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderDebugLine(const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, DrawMode drawMode, float duration)
{
	m_debugShapeVector.push_back(new DebugLine(startPosition,endPosition,startColor,endColor,drawMode,duration));
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderDebugArrow(const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, DrawMode drawMode, float duration)
{
	m_debugShapeVector.push_back(new DebugArrow(startPosition,endPosition,startColor,endColor,drawMode,duration));
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderDebugAABB3(const AABB3& boundingBox, const Color& wireframeColor, const Color& faceColor, DrawMode drawMode, float duration)
{
	m_debugShapeVector.push_back(new DebugAABB3(boundingBox,wireframeColor,faceColor,drawMode,duration));
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::RenderDebugUVSphere(const Vector3f& center, float radius, const Color& wireframeColor, const Color& faceColor, DrawMode drawMode, float duration, TriangleDensityLevel density)
{
	m_debugShapeVector.push_back(new DebugUVSphere(center,radius,wireframeColor,faceColor,drawMode,duration,density));
}

//-----------------------------------------------------------------------------------------------
uint OpenGLRenderer::CreateShaderProgramAndGetID(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	GLuint vertexShaderID = CreateShaderAndGetID(vertexShaderPath, GL_VERTEX_SHADER, PROJECT_DELIMITER);
	GLuint fragmentShaderID = CreateShaderAndGetID(fragmentShaderPath, GL_FRAGMENT_SHADER, PROJECT_DELIMITER);

	GLuint shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShaderID);
	glAttachShader(shaderProgramID, fragmentShaderID);
	glLinkProgram(shaderProgramID);

	GLint wasSuccessful;
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &wasSuccessful);

	if(!wasSuccessful)
	{
		GLint infoLogLength = 0;
		glGetProgramiv(shaderProgramID, GL_INFO_LOG_LENGTH, &infoLogLength );
		char* infoLogText = new char [ infoLogLength + 1 ];
		glGetProgramInfoLog(shaderProgramID, infoLogLength, NULL, infoLogText);

		ReportLinkerErrors(vertexShaderPath, fragmentShaderPath, infoLogLength, infoLogText);
		delete infoLogText;
		std::exit(0);
	}

	return shaderProgramID;
}

//-----------------------------------------------------------------------------------------------
GLuint OpenGLRenderer::CreateShaderAndGetID(const std::string& shaderFilePath, uint shaderType, const std::string& folderNameDelimiter)
{
	GLuint id = glCreateShader(shaderType);
	FILE* shaderFile = fopen(shaderFilePath.c_str(), "rb");

	fseek(shaderFile,0L,SEEK_END);
	int fileSize = (int)ftell(shaderFile);
	++fileSize;
	rewind(shaderFile);

	char* shaderText = new char[fileSize];

	fread(shaderText,fileSize,1,shaderFile);
	shaderText[fileSize-1] = '\0';

	glShaderSource(id,1,&shaderText,NULL);
	glCompileShader(id);

	GLint wasSuccessful;
	glGetShaderiv(id, GL_COMPILE_STATUS, &wasSuccessful);

	//-----------------------------------------------------------------------------------------------

	if(!wasSuccessful)
	{
		//Get error log, store it into infoLogText

		GLint infoLogLength = 0;
		glGetShaderiv( id, GL_INFO_LOG_LENGTH, &infoLogLength );
		char* infoLogText = new char [ infoLogLength + 1 ];
		glGetShaderInfoLog( id, infoLogLength, NULL, infoLogText);

		ReportShaderErrors(shaderFilePath, folderNameDelimiter, infoLogLength, infoLogText);

		delete infoLogText;
		std::exit(0);
	}

	//-----------------------------------------------------------------------------------------------
	return id;
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::ReportShaderErrors(const std::string& shaderFilePath, const std::string& folderNameDelimiter, GLint logLength, const char* logText)
{
	//split the various errors up
	std::vector<std::string> errorLogStrings = SplitStringByStringDelimiter(logText, "0(");

	//get the actual shader file name
	std::vector<std::string> shaderName = SplitStringByStringDelimiter(shaderFilePath, "\\");
	char* captionString = new char[20 + shaderName.back().size() + 1];

	captionString[0] = '\0';
	strcat(captionString, "Error(s) in file : ");
	strcat(captionString, shaderName.back().c_str());

	//if there are no line specific erros out raw message and return
	if(errorLogStrings.size() <= 1)
	{
		MessageBoxA(g_hWnd,logText,captionString, MB_OK);
		delete captionString;
		return;
	}

	//get the current working directory path
	const int BUFFERSIZE = 2024;
	char* currentWorkingDirectoryText = new char[BUFFERSIZE];
	int actualSizeInBytes = GetModuleFileNameA(NULL, currentWorkingDirectoryText, BUFFERSIZE + 1);

	//make sure we didn't need to truncate
	if(actualSizeInBytes > BUFFERSIZE)
	{
		assert(actualSizeInBytes < BUFFERSIZE);
	}
	
	//create the correct filepath to shader program
	std::vector<std::string> directoryName = SplitStringByStringDelimiter(currentWorkingDirectoryText, folderNameDelimiter);
	int filePathLength = folderNameDelimiter.size() + directoryName[0].size() + shaderFilePath.size() + 1;

	char* actualFilePath = new char[filePathLength];
	actualFilePath[0] = '\0';
	strcat(actualFilePath,directoryName[0].c_str());
	strcat(actualFilePath, folderNameDelimiter.c_str());
	strcat(actualFilePath, shaderFilePath.c_str());

	//concatenate the filepath to the various errors in the shader program
	//output the resultant strings to output window
	char* outputString;
	char* messageBoxString = new char[7 * (errorLogStrings.size() - 1) + logLength + 1];
	messageBoxString[0] = '\0';
	auto iter = errorLogStrings.begin();

	for( ++iter ;iter != errorLogStrings.end(); ++ iter)
	{
		outputString = new char[iter->size() + 1 + filePathLength + 1];
		outputString[0] = '\0';
		strcat(outputString,actualFilePath);
		strcat(outputString, "(");
		strcat(outputString, iter->c_str());
		strcat(messageBoxString, "Line (");
		strcat(messageBoxString, iter->c_str());

		OutputDebugStringA(outputString);
		delete outputString;
	}

	MessageBoxA(g_hWnd,messageBoxString,captionString, MB_OK);
	
	delete messageBoxString;
	delete captionString;
	delete actualFilePath;
	delete currentWorkingDirectoryText;
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::ReportLinkerErrors(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, GLint logLength, const char* logText)
{
	//get the actual shader file name
	std::vector<std::string> vertexShaderName = SplitStringByStringDelimiter(vertexShaderPath, "\\");
	std::vector<std::string> fragmentShaderName = SplitStringByStringDelimiter(fragmentShaderPath, "\\");
	UNUSED(logLength);
	
	char* captionString = new char[28 + vertexShaderName.back().size() + 1 + fragmentShaderName.back().size()];
	captionString[0] = '\0';
	strcat(captionString, "Error Linking Shaders: ");
	strcat(captionString, vertexShaderName.back().c_str());
	strcat(captionString, " & ");
	strcat(captionString, fragmentShaderName.back().c_str());

	MessageBoxA(g_hWnd,logText,captionString, MB_OK);
	delete captionString;
	return;
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::EnableAndBindGLFunctions()
{
	glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress( "glGenBuffers" );
	glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress( "glBindBuffer" );
	glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress( "glBufferData" );
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) wglGetProcAddress("glDeleteBuffers");

	glCreateShader	= (PFNGLCREATESHADERPROC) wglGetProcAddress	("glCreateShader");
	glShaderSource	= (PFNGLSHADERSOURCEPROC) wglGetProcAddress	("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");	
	glGetShaderiv	= (PFNGLGETSHADERIVPROC) wglGetProcAddress	("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
	glAttachShader	= (PFNGLATTACHSHADERPROC) wglGetProcAddress	("glAttachShader");
	glLinkProgram	= (PFNGLLINKPROGRAMPROC) wglGetProcAddress	("glLinkProgram");
	glGetProgramiv	= (PFNGLGETPROGRAMIVPROC) wglGetProcAddress	("glGetProgramiv");
	glUseProgram	= (PFNGLUSEPROGRAMPROC) wglGetProcAddress	("glUseProgram");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
	glUniform1f	= (PFNGLUNIFORM1FPROC) wglGetProcAddress ("glUniform1f");
	glUniform1i = (PFNGLUNIFORM1IPROC) wglGetProcAddress ("glUniform1i");
	glUniform4f = (PFNGLUNIFORM4FPROC) wglGetProcAddress ("glUniform4f");
	glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress ("glUniform3f");
	glUniform4fv = (PFNGLUNIFORM4FVPROC) wglGetProcAddress("glUniform4fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC) wglGetProcAddress ("glUniform3fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) wglGetProcAddress("glUniformMatrix4fv");

	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) wglGetProcAddress("glVertexAttribPointer");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glDisableVertexAttribArray");
	glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC) wglGetProcAddress("glVertexAttrib4f");
	glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC) wglGetProcAddress("glVertexAttrib3f");
	glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC) wglGetProcAddress("glVertexAttrib2f");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) wglGetProcAddress("glGetAttribLocation"); 
	glActiveTexture = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress("glActiveTexture");
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::UseShaderProgram(uint shaderProgramID)
{
	currentShaderProgram = shaderProgramID;
	glUseProgram(currentShaderProgram);
}

//-----------------------------------------------------------------------------------------------
int OpenGLRenderer::GetUniformLocation(int programID, const char* variableName)
{
	return glGetUniformLocation(programID,variableName);
}

//-----------------------------------------------------------------------------------------------
uint OpenGLRenderer::GetCurrentShaderProgram()
{
	return currentShaderProgram;
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Uniform1i(int location, int value)
{
	glUniform1i(location, value);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Uniform1f(int location, float value)
{
	glUniform1f(location, value);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Uniform4f(int location, float value1, float value2, float value3, float value4)
{
	glUniform4f(location,value1,value2,value3,value4);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Uniform3f(int location, float value1, float value2, float value3)
{
	glUniform3f(location,value1,value2,value3);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Uniform4fv(int location, int count, const float *value)
{
	glUniform4fv(location,count,value);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::Uniform3fv(int location, int count, const float *value)
{
	glUniform3fv(location,count,value);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::UniformMatrix4fv(int location,  int count, bool transpose, const float *value)
{
	glUniformMatrix4fv(location, count, transpose, value);
}

//-----------------------------------------------------------------------------------------------
int OpenGLRenderer::GetAttribLocation(uint programID, const char* variableName)
{
	return glGetAttribLocation(programID, variableName);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::VertexAttribPointer(uint index, int size, uint type, bool normalized, int stride, const void * pointer)
{
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::EnableVertexAttribArray(uint index)
{
	glEnableVertexAttribArray(index);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::DisableVertexAttribArray(uint index)
{
	glDisableVertexAttribArray(index);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::VertexAttrib4f(uint index, float value0, float value1, float value2, float value3)
{
	glVertexAttrib4f(index, value0, value1, value2, value3);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::VertexAttrib3f(uint index, float value0, float value1, float value2)
{
	glVertexAttrib3f(index, value0, value1, value2);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::VertexAttrib2f(uint index, float value0, float value1)
{
	glVertexAttrib2f(index, value0, value1);
}

//-----------------------------------------------------------------------------------------------
void OpenGLRenderer::ActiveTexture(uint textureID)
{
	glActiveTexture(textureID);
}

//-----------------------------------------------------------------------------------------------
OpenGLRenderer::DebugPoint::DebugPoint(const Vector3f& position, const Color& color, DrawMode drawMode, float duration) 
	: m_position(position)
	, m_color(color.ToVector3fNormalizedAndFullyOpaque())
{
	m_drawMode = drawMode;
	m_duration = duration;
}

void OpenGLRenderer::DebugPoint::Render() const
{

	switch(m_drawMode)
	{
	case DEPTH_TEST:
		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawPoint(3.f);

		break;
	case NO_DEPTH_TEST:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);

		DrawPoint(3.f);

		break;
	case DUAL_MODE:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);

		DrawPoint(1.f);

		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawPoint(3.f);

		break;
	}
}

void OpenGLRenderer::DebugPoint::DrawPoint(float lineWidth) const
{
	const float offset = 0.25f;

	LineWidth(lineWidth);
	Color3f(m_color.x, m_color.y, m_color.z);

	Begin(GL_LINES);
	{
		Vertex3f(m_position.x, m_position.y - offset, m_position.z - offset);
		Vertex3f(m_position.x, m_position.y + offset, m_position.z + offset);

		Vertex3f(m_position.x, m_position.y + offset, m_position.z - offset);
		Vertex3f(m_position.x, m_position.y - offset, m_position.z + offset);

		Vertex3f(m_position.x - offset, m_position.y, m_position.z - offset);
		Vertex3f(m_position.x + offset, m_position.y, m_position.z + offset);

		Vertex3f(m_position.x - offset, m_position.y, m_position.z + offset);
		Vertex3f(m_position.x + offset, m_position.y, m_position.z - offset);
	}
	End();
}

//-----------------------------------------------------------------------------------------------
OpenGLRenderer::DebugLine::DebugLine(const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, DrawMode drawMode, float duration)
	: m_startPosition(startPosition)
	, m_endPosition(endPosition)
	, m_startColor(startColor.ToVector3fNormalizedAndFullyOpaque())
	, m_endColor(endColor.ToVector3fNormalizedAndFullyOpaque())
{
	m_drawMode = drawMode;
	m_duration = duration;
}

void OpenGLRenderer::DebugLine::Render() const
{
	switch(m_drawMode)
	{
	case DEPTH_TEST:
		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawLine(3.f);

		break;
	case NO_DEPTH_TEST:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);

		DrawLine(3.f);

		break;
	case DUAL_MODE:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);

		DrawLine(1.f);

		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawLine(3.f);

		break;
	}
}

void OpenGLRenderer::DebugLine::DrawLine(float lineWidth) const
{
	LineWidth(lineWidth);

	Begin(GL_LINES);
	{
		Color3f(m_startColor.x, m_startColor.y, m_startColor.z);
		Vertex3f(m_startPosition.x, m_startPosition.y, m_startPosition.z);

		Color3f(m_endColor.x, m_endColor.y, m_endColor.z);
		Vertex3f(m_endPosition.x, m_endPosition.y, m_endPosition.z);
	}
	End();
}

//-----------------------------------------------------------------------------------------------
OpenGLRenderer::DebugArrow::DebugArrow(const Vector3f& startPosition, const Vector3f& endPosition, const Color& startColor, const Color& endColor, DrawMode drawMode, float duration)
	: DebugLine(startPosition
	, endPosition
	, startColor
	, endColor
	, drawMode
	, duration)
{

	Vector3f k = endPosition - startPosition;
	k.Normalize();

	Vector3f i = Vector3f::CrossProduct(Vector3f(0.f,0.f,1.f), k);
	i.Normalize();

	Vector3f j = Vector3f::CrossProduct(k, i);
	j.Normalize();
	
	Matrix44f rotationMatrix;

	rotationMatrix.data[0] = i.x;
	rotationMatrix.data[1] = i.y;
	rotationMatrix.data[2] = i.z;
				  
	rotationMatrix.data[4] = j.x;
	rotationMatrix.data[5] = j.y;
	rotationMatrix.data[6] = j.z;
				  
	rotationMatrix.data[8] = k.x;
	rotationMatrix.data[9] = k.y;
	rotationMatrix.data[10] = k.z;

	m_arrowTransform.data[12] = endPosition.x;
	m_arrowTransform.data[13] = endPosition.y;
	m_arrowTransform.data[14] = endPosition.z;

	m_arrowTransform = Matrix44f::MultMatrix(rotationMatrix, m_arrowTransform);
}

void OpenGLRenderer::DebugArrow::Render() const
{
	switch(m_drawMode)
	{
	case DEPTH_TEST:
		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawLine(3.f);
		DrawArrow(3.f);

		break;
	case NO_DEPTH_TEST:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);

		DrawLine(3.f);
		DrawArrow(3.f);

		break;
	case DUAL_MODE:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);

		DrawLine(1.f);
		DrawArrow(0.5f);

		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawLine(3.f);
		DrawArrow(1.0f);

		break;
	}
}

void OpenGLRenderer::DebugArrow::DrawArrow(float alpha) const
{
	const float offsetZ = 0.25f;
	const float offset = 0.125f;

	Color4f(m_endColor.x,m_endColor.y,m_endColor.z,alpha);

	OpenGLRenderer::Disable(GL_CULL_FACE);

	s_rendererStack.Push(MatrixStack44f::MODEL_STACK);

	glMultMatrixf(m_arrowTransform.data);

	Begin(GL_TRIANGLES);
	{
		Vertex3f(-offset, offset, -offsetZ);
		Vertex3f(0.f, 0.f, 0.f);
		Vertex3f(offset, offset, -offsetZ);

		Vertex3f(offset, offset, -offsetZ);
		Vertex3f(0.f, 0.f, 0.f);
		Vertex3f(offset, -offset, -offsetZ);

		Vertex3f(offset, -offset, -offsetZ);
		Vertex3f(0.f, 0.f, 0.f);
		Vertex3f(-offset, -offset, -offsetZ);

		Vertex3f(-offset, -offset, -offsetZ);	
		Vertex3f(0.f, 0.f, 0.f);
		Vertex3f(-offset, offset, -offsetZ);
	}
	End();

	s_rendererStack.Pop(MatrixStack44f::MODEL_STACK);
}

//-----------------------------------------------------------------------------------------------
OpenGLRenderer::DebugAABB3::DebugAABB3(const AABB3& boundingBox, const Color& wireframeColor, const Color& faceColor, DrawMode drawMode, float duration)
	: m_minCoords(boundingBox.GetMinCoords())
	, m_maxCoords(boundingBox.GetMaxCoords())
	, m_wireframeColor(wireframeColor.ToVector3fNormalizedAndFullyOpaque())
	, m_faceColor(faceColor.ToVector3fNormalizedAndFullyOpaque())
{
	m_drawMode = drawMode;
	m_duration = duration;
}

void OpenGLRenderer::DebugAABB3::Render() const
{
	switch(m_drawMode)
	{
	case DEPTH_TEST:
		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawSolidBox(3.f);
		DrawWireFrameBox(3.f);
		
		break;
	case NO_DEPTH_TEST:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);

		DrawSolidBox(3.f);
		DrawWireFrameBox(3.f);
		
		break;
	case DUAL_MODE:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);

		//DrawSolidBox(0.5f);
		DrawWireFrameBox(1.f);

		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawSolidBox(1.0f);
		DrawWireFrameBox(3.f);

		break;
	}
}

void OpenGLRenderer::DebugAABB3::DrawWireFrameBox(float lineWidth) const
{
	Disable(GL_TEXTURE_2D);
	LineWidth(lineWidth);
	Color3f(m_wireframeColor.x, m_wireframeColor.y, m_wireframeColor.z);

	//front
	Begin(GL_LINE_STRIP);
	{
		Vertex3f(m_minCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_maxCoords.z);
		Vertex3f(m_minCoords.x, m_minCoords.y, m_maxCoords.z);
	}
	End();

	//back
	Begin(GL_LINE_STRIP);
	{
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_maxCoords.z);
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_maxCoords.z);
	}
	End();

	//left
	Begin(GL_LINE_STRIP);
	{
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_minCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_minCoords.x, m_minCoords.y, m_maxCoords.z);
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_maxCoords.z);
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_minCoords.z);
	}
	End();

	//right
	Begin(GL_LINE_STRIP);
	{
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_maxCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_maxCoords.z);
	}
	End();

	//top
	Begin(GL_LINE_STRIP);
	{
		Vertex3f(m_minCoords.x, m_minCoords.y, m_maxCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_maxCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_maxCoords.z);
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_maxCoords.z);
	}
	End();

	//bottom
	Begin(GL_LINE_STRIP);
	{		
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_minCoords.x, m_minCoords.y, m_minCoords.z);
	}
	End();
}

void OpenGLRenderer::DebugAABB3::DrawSolidBox(float alpha) const
{
	Disable(GL_TEXTURE_2D);
	Color4f(m_faceColor.x, m_faceColor.y, m_faceColor.z, alpha);

	//front
	Begin(GL_QUADS);
	{
		Vertex3f(m_minCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_maxCoords.z);
		Vertex3f(m_minCoords.x, m_minCoords.y, m_maxCoords.z);
	}
	End();

	//back
	Begin(GL_QUADS);
	{
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_maxCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_maxCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_minCoords.z);	
	}
	End();

	//left
	Begin(GL_QUADS);
	{
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_minCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_minCoords.x, m_minCoords.y, m_maxCoords.z);
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_maxCoords.z);
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_minCoords.z);
	}
	End();

	//right
	Begin(GL_QUADS);
	{
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_maxCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_maxCoords.z);
	}
	End();

	//top
	Begin(GL_QUADS);
	{
		Vertex3f(m_minCoords.x, m_minCoords.y, m_maxCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_maxCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_maxCoords.z);
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_maxCoords.z);
	}
	End();

	//bottom
	Begin(GL_QUADS);
	{
		Vertex3f(m_minCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_maxCoords.y, m_minCoords.z);
		Vertex3f(m_maxCoords.x, m_minCoords.y, m_minCoords.z);
		Vertex3f(m_minCoords.x, m_minCoords.y, m_minCoords.z);
	}
	End();
}

//-----------------------------------------------------------------------------------------------
OpenGLRenderer::DebugUVSphere::DebugUVSphere(const Vector3f& center, float radius, const Color& wireframeColor, const Color& faceColor, DrawMode drawMode, float duration, TriangleDensityLevel density)
	: m_centerPoint(center)
	, m_radius(radius)
	, m_wireframeColor(wireframeColor.ToVector3fNormalizedAndFullyOpaque())
	, m_faceColor(faceColor.ToVector3fNormalizedAndFullyOpaque())
	, m_density(density)
{
	m_drawMode = drawMode;
	m_duration = duration;
}

void OpenGLRenderer::DebugUVSphere::Render() const
{
	PushMatrix();
	//s_rendererStack.Push(MatrixStack44f::MODEL_STACK);
	Translatef(m_centerPoint.x, m_centerPoint.y, m_centerPoint.z);
	switch(m_drawMode)
	{
	case DEPTH_TEST:
		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawSolidUVSphere(3.f);
		DrawWireFrameUVSphere(3.f);
		
		break;
	case NO_DEPTH_TEST:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);

		DrawSolidUVSphere(3.f);
		DrawWireFrameUVSphere(3.f);

		break;
	case DUAL_MODE:

		OpenGLRenderer::Disable(GL_DEPTH_TEST);
	
		//DrawSolidUVSphere(0.5f);
		DrawWireFrameUVSphere(1.f);

		OpenGLRenderer::Enable(GL_DEPTH_TEST);

		DrawSolidUVSphere(1.0f);
		DrawWireFrameUVSphere(3.f);

		break;
	}
	//s_rendererStack.Pop(MatrixStack44f::MODEL_STACK);
	PopMatrix();
}

void OpenGLRenderer::DebugUVSphere::DrawWireFrameUVSphere(float lineWidth) const
{
	Disable(GL_TEXTURE_2D);
	LineWidth(lineWidth);
	Color3f(m_wireframeColor.x, m_wireframeColor.y, m_wireframeColor.z);
	
	renderWireSphere(m_radius,m_density,m_density);

}

void OpenGLRenderer::DebugUVSphere::DrawSolidUVSphere(float alpha) const
{
	Disable(GL_TEXTURE_2D);
	Color4f(m_faceColor.x, m_faceColor.y, m_faceColor.z, alpha);

	renderSolidSphere(m_radius,m_density,m_density);
}