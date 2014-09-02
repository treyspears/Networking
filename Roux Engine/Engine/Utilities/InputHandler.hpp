#ifndef INPUTHANDLER
#define INPUTHANDLER

#define WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------------------------
#include <windows.h>
#include <XInput.h> // include the Xinput API

#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Utilities/CommandRegistry.hpp"

#pragma comment(lib, "XInput.lib")
//-----------------------------------------------------------------------------------------------

const int NUM_GAMEPADS = 4;
const int NUM_VIRTUAL_KEYS = 256;
const int NUM_MOUSE_INPUTS = 6;

enum ControllerID
{
	CONTROLLER_MOUSE_AND_KEYBOARD = -1,
	CONTROLLER_GAMEPAD_1 = 0,
	CONTROLLER_GAMEPAD_2 = 1,
	CONTROLLER_GAMEPAD_3 = 2,
	CONTROLLER_GAMEPAD_4 = 3
};


class InputHandler
{
public:

	enum InputHandlerState
	{
		INPUTHANDLER_GAME,
		INPUTHANDLER_DEV_CONSOLE
	};

	InputHandler();

	static void Initialize(HWND initialWindow);

	static LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam );

	static bool ShouldQuit();
	static void QuitApplication();

	void RunMessagePump();
	void RunXboxMessagePump();

	static bool ProcessCharDownEvent(uchar virtualKeyCode);
	static bool ProcessKeyDownEvent(uchar virtualKeyCode);
	static bool ProcessKeyUpEvent(uchar virtualKeyCode);

	static bool isKeyDown(uchar virtualKeyCode);
	static bool isKeyUp(uchar virtualKeyCode);
	static bool isKeyReleased(uchar virtualKeyCode);
	static bool isKeyPressed(uchar virtualKeyCode);

	static bool isButtonDown(uint buttonKeyCode, uint playerIndex);
	static bool isButtonUp(uint buttonKeyCode, uint playerIndex);
	static bool isButtonPressed(uint buttonKeyCode, uint playerIndex);

	static void Vibrate(int leftVal, int rightVal, uint playerIndex);
	static bool isConnected(uint playerIndex);
	static bool ProcessJoystick(Vector2f& out_Direction, float& magnitude, uint playerIndex, uint requestedJoystick);

	static void GetMouseDisplacementVector(Vector2i& out_mouseDisplacement);
	static void ResetMousePosition();

	static bool isLeftMouseDown();
	static bool isLeftMouseClick();
	static bool isLeftMouseUp();
	static bool isLeftMouseDoubleClick();

	static bool isRightMouseDown();
	static bool isRightMouseClick();
	static bool isRightMouseUp();
	static bool isRightMouseDoubleClick();

	static bool isMiddleMouseDown();
	static bool isMiddleMouseClick();
	static bool isMiddleMouseUp();
	static bool isMiddleMouseDoubleClick();

 	static InputHandlerState GetInputState();
 	static void SetInputState(InputHandlerState state);

	static bool m_previousCharState[NUM_VIRTUAL_KEYS];
	static bool m_currentCharState[NUM_VIRTUAL_KEYS];

private:

	static InputHandlerState s_state;
	static void NormalizeThumbStick(Vector2f& thumbstick);
	static HWND m_clientWindow;
	static bool isQuitting;

	static bool m_previousKeyState[NUM_VIRTUAL_KEYS];
	static bool m_currentKeyState[NUM_VIRTUAL_KEYS];

	static XINPUT_STATE m_previousGamePadState[NUM_GAMEPADS];
	static XINPUT_STATE m_currentGamePadState[NUM_GAMEPADS];

	static bool m_previousMouseState[NUM_MOUSE_INPUTS];
	static bool m_currentMouseState[NUM_MOUSE_INPUTS];
	static tagPOINT *m_mousePosition;
};

#endif