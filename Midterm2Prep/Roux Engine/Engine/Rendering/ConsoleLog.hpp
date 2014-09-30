#ifndef CONSOLELOG
#define CONSOLELOG

#include <vector>
#include <string>
#include <map>

#include "Engine/Rendering/DrawableTextQuad.hpp"
#include "Engine/Rendering/Vertex.hpp"

const int NUM_QUAD_VERTICES = 8;


class BitmapFont;

class ConsoleLog
{
public:

	ConsoleLog(BitmapFont* fontToUse, float fontSize);
	~ConsoleLog();

	void ConsolePrint(const std::string& textToRender, const Vector4f& fontColor = Vector4f(1.f,1.f,1.f,1.f)
		, bool useDropShadow = false, const Vector4f& dropShadowColor = Vector4f(0.f,0.f,0.f,1.f));
	
	bool ToggleLogToRender();
	void ScrollUpLog();
	void ScrollDownLog();
	int GetCommandPromptLength();
	void ClearCommandPrompt();
	void ClearConsoleLog();
	void PrintToCommandPrompt(const std::string& textToRender);
	inline void BackSpace();
	void EnterCommand();

	void Update(float deltaSeconds);
	void Render() const;

	static std::vector<ConsoleLog*> s_logs;
	static ConsoleLog* s_currentLog;

private:

	void CreateTranslucentQuads();
	void CreateTranslucentQuadVBOs(const VertexPositionColor (&vertices) [NUM_QUAD_VERTICES]);
	void RenderTranslucentQuads() const;

	uint m_quadBufferId;
	
	bool m_showLog;
	BitmapFont* m_fontToUse;
	float m_fontSize;
	int m_stackIndex;
 	Vector4f m_commandPromptColor;

	struct TextQuad
	{
		TextQuad(const DrawableTextQuad& theQuad, bool renderDropShadow, const Vector4f& shadowColor)
		: quad(theQuad)
		, shouldRenderDropShadow(renderDropShadow)
		, dropShadowColor(shadowColor)
		{

		}

		DrawableTextQuad quad;
		bool shouldRenderDropShadow;
		Vector4f dropShadowColor;
	};

	TextQuad* m_commandPromptQuad;
	std::vector<TextQuad> m_logStack;

	class Cursor
	{
	public:
		Cursor();
		void CreateVertices(float deltaSeconds, const TextQuad* commandPrompt, const Vector4f& commandPromptColor);
		void Update(float deltaSeconds, const TextQuad* commandPrompt, const Vector4f& commandPromptColor);
		void Render() const;
		std::vector<Vector3f> m_positionArray;
		std::vector<Vector4f> m_colorArray;	
	};

	Cursor m_cursor;
};

//-----------------------------------------------------------------------------------------------
void ConsoleLog::BackSpace()
{
	if(m_commandPromptQuad->quad.GetStringLength() > 1)
		m_commandPromptQuad->quad.DeleteEndCharacter();
}


#endif