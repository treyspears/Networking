#include "InputHandler.hpp"

#include <math.h>

#include "Engine/Rendering/OpenGLRenderer.hpp"

#include "Engine/Utilities/NewAndDeleteOverrides.hpp"

HWND InputHandler::m_clientWindow;

InputHandler::InputHandlerState InputHandler::s_state = INPUTHANDLER_GAME;
bool InputHandler::isQuitting = false;
bool InputHandler::m_currentKeyState[NUM_VIRTUAL_KEYS];
bool InputHandler::m_previousKeyState[NUM_VIRTUAL_KEYS];
bool InputHandler::m_previousCharState[NUM_VIRTUAL_KEYS];
bool InputHandler::m_currentCharState[NUM_VIRTUAL_KEYS];

XINPUT_STATE InputHandler::m_currentGamePadState[NUM_GAMEPADS];
XINPUT_STATE InputHandler::m_previousGamePadState[NUM_GAMEPADS];

tagPOINT* InputHandler::m_mousePosition = nullptr;
bool InputHandler::m_currentMouseState[NUM_MOUSE_INPUTS];
bool InputHandler::m_previousMouseState[NUM_MOUSE_INPUTS];


const uchar TRIGGER_STRENGTH_TO_ACTIVATE = 128;
const unsigned short XBOX_CONTROLLER_MAX = 32768;
const float	XBOX_CONTROLLER_SHIFTED_MAX_INVERSE = 1.f / 65535.f;
const float DEAD_ZONE_RADIUS = 0.65f;
const float ONE_MINUS_DEAD_ZONE_RADIUS_INVERSE = 1.f / (1.f - DEAD_ZONE_RADIUS);



InputHandler::InputHandler()
{

}

void InputHandler::Initialize(HWND initialWindow)
{
	InputHandler::m_mousePosition = new tagPOINT();

	memset(&(InputHandler::m_previousKeyState),0,sizeof(InputHandler::m_previousKeyState));
	memset(&(InputHandler::m_previousCharState),0,sizeof(InputHandler::m_previousCharState));
	memset(&(InputHandler::m_previousMouseState),0,sizeof(InputHandler::m_currentMouseState));
	m_clientWindow = initialWindow;
	
}

//-----------------------------------------------------------------------------------------------
void InputHandler::RunMessagePump()
{
	memcpy(&(InputHandler::m_previousKeyState),&(InputHandler::m_currentKeyState),sizeof(InputHandler::m_previousKeyState));
	memcpy(&(InputHandler::m_previousCharState),&(InputHandler::m_currentCharState),sizeof(InputHandler::m_previousCharState));
	memcpy(&(InputHandler::m_previousMouseState),&(InputHandler::m_currentMouseState),sizeof(InputHandler::m_previousMouseState));
	GetCursorPos(m_mousePosition);
	//ScreenToClient(m_clientWindow,m_mousePosition);
	MSG queuedMessage;

	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage );
	}
}

//-----------------------------------------------------------------------------------------------
void InputHandler::RunXboxMessagePump()
{
	for(int gamepadIndex = 0; gamepadIndex < NUM_GAMEPADS; gamepadIndex ++)
	{
		memset(&(InputHandler::m_previousGamePadState[gamepadIndex]),0,sizeof(XINPUT_STATE));
		InputHandler::m_previousGamePadState[gamepadIndex] = InputHandler::m_currentGamePadState[gamepadIndex];

		memset(&(InputHandler::m_currentGamePadState[gamepadIndex]),0,sizeof(XINPUT_STATE));
		XInputGetState(gamepadIndex, &m_currentGamePadState[gamepadIndex]);
	}
}

void InputHandler::QuitApplication()
{
	InputHandler::isQuitting = true;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::ShouldQuit()
{
	return InputHandler::isQuitting;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::ProcessKeyDownEvent(uchar virtualKeyCode)
{
	InputHandler::m_currentKeyState[virtualKeyCode] = true;

	return true;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::ProcessKeyUpEvent(uchar virtualKeyCode)
{
	InputHandler::m_currentKeyState[virtualKeyCode] = false;
	InputHandler::m_currentCharState[virtualKeyCode] = false;

	return true;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::ProcessCharDownEvent(uchar virtualKeyCode)
{
	InputHandler::m_currentCharState[virtualKeyCode] = true;

	return true;
}

//-----------------------------------------------------------------------------------------------
LRESULT CALLBACK InputHandler::WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	unsigned char asKey = (uchar) wParam;

	switch( wmMessageCode )
	{
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
		isQuitting = true;
		return 0;
	case WM_SYSKEYDOWN:
		return 0;
	case WM_KEYDOWN:
		{
			if( asKey == VK_ESCAPE && s_state == INPUTHANDLER_GAME)
			{
				QuitApplication();
				return 0;
			}

			bool wasProcessed = ProcessKeyDownEvent(asKey);
			if(wasProcessed)
			{
				return 0;
			}
		}
		break;
	case WM_KEYUP:
		{
			bool wasProcessed = ProcessKeyUpEvent(asKey);
			if(wasProcessed)
			{
				return 0;
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
 			InputHandler::m_currentMouseState[MOUSE_RIGHT] = true;
			return 0;
		}
		break;
	case WM_RBUTTONUP:
		{
			InputHandler::m_currentMouseState[MOUSE_RIGHT] = false;
			return 0;
		}
		break;
	case WM_RBUTTONDBLCLK:
		{
			InputHandler::m_currentMouseState[MOUSE_R_DOUBLE_CLICK] = true;
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			InputHandler::m_currentMouseState[MOUSE_LEFT] = true;
			return 0;
		}
		break;
	case WM_LBUTTONUP:
		{
			InputHandler::m_currentMouseState[MOUSE_LEFT] = false;
			return 0;
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			InputHandler::m_currentMouseState[MOUSE_L_DOUBLE_CLICK] = true;
			return 0;
		}
	case WM_MBUTTONDOWN:
		{
			InputHandler::m_currentMouseState[MOUSE_MIDDLE] = true;
			return 0;
		}
		break;
	case WM_MBUTTONUP:
		{
			InputHandler::m_currentMouseState[MOUSE_MIDDLE] = false;
			return 0;
		}
		break;
	case WM_MBUTTONDBLCLK:
		{
			InputHandler::m_currentMouseState[MOUSE_M_DOUBLE_CLICK] = true;
			return 0;
		}
		break;
	case WM_CHAR:
		{
			if(!s_state == INPUTHANDLER_DEV_CONSOLE)
				return 0;

			switch(asKey)
			{
			case '`':
			case VK_ESCAPE:
			case VK_BACK:
			case VK_RETURN:
				return 0;
			break;
			}
			bool wasProcessed = ProcessCharDownEvent(asKey);
			if(wasProcessed)
				return 0;
			
		}
		break;
	}

	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isKeyDown(uchar virtualKeyCode)
{
	bool result = InputHandler::m_currentKeyState[virtualKeyCode];
	return result;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isKeyUp(uchar virtualKeyCode)
{
	bool result = !(InputHandler::m_currentKeyState[virtualKeyCode]);
	return result;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isKeyPressed(uchar virtualKeyCode)
{
	bool currentKeyState = InputHandler::m_currentKeyState[virtualKeyCode];
	bool previousKeyState = InputHandler::m_previousKeyState[virtualKeyCode];

 	if(currentKeyState && !previousKeyState)
		return true;

	return false;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isKeyReleased(uchar virtualKeyCode)
{
	bool currentKeyState = InputHandler::m_currentKeyState[virtualKeyCode];
	bool previousKeyState = InputHandler::m_previousKeyState[virtualKeyCode];

	if(!currentKeyState && previousKeyState)
		return true;

	return false;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isButtonDown(uint buttonKeyCode, uint playerIndex)
{
	if(buttonKeyCode == XINPUT_GAMEPAD_LEFT_TRIGGER)
	{
		return InputHandler::m_currentGamePadState[playerIndex].Gamepad.bLeftTrigger > TRIGGER_STRENGTH_TO_ACTIVATE;
	}
	else if(buttonKeyCode == XINPUT_GAMEPAD_RIGHT_TRIGGER)
	{
		return InputHandler::m_currentGamePadState[playerIndex].Gamepad.bRightTrigger > TRIGGER_STRENGTH_TO_ACTIVATE;
	}

	return (InputHandler::m_currentGamePadState[playerIndex].Gamepad.wButtons & buttonKeyCode) != 0;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isButtonUp(uint buttonKeyCode, uint playerIndex)
{
	if(buttonKeyCode == XINPUT_GAMEPAD_LEFT_TRIGGER)
	{
		return InputHandler::m_currentGamePadState[playerIndex].Gamepad.bLeftTrigger <= TRIGGER_STRENGTH_TO_ACTIVATE;
	}
	else if(buttonKeyCode == XINPUT_GAMEPAD_RIGHT_TRIGGER)
	{
		return InputHandler::m_currentGamePadState[playerIndex].Gamepad.bRightTrigger <= TRIGGER_STRENGTH_TO_ACTIVATE;
	}

	return (InputHandler::m_currentGamePadState[playerIndex].Gamepad.wButtons & buttonKeyCode) == 0;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isButtonPressed(uint buttonKeyCode, uint playerIndex)
{
	bool currentButtonState;
	bool previousButtonState;

	if(buttonKeyCode == XINPUT_GAMEPAD_LEFT_TRIGGER)
	{
		previousButtonState = InputHandler::m_previousGamePadState[playerIndex].Gamepad.bLeftTrigger > TRIGGER_STRENGTH_TO_ACTIVATE;
		currentButtonState = InputHandler::m_currentGamePadState[playerIndex].Gamepad.bLeftTrigger > TRIGGER_STRENGTH_TO_ACTIVATE;
	}
	else if(buttonKeyCode == XINPUT_GAMEPAD_RIGHT_TRIGGER)
	{
		previousButtonState = InputHandler::m_previousGamePadState[playerIndex].Gamepad.bRightTrigger > TRIGGER_STRENGTH_TO_ACTIVATE;
		currentButtonState = InputHandler::m_currentGamePadState[playerIndex].Gamepad.bRightTrigger > TRIGGER_STRENGTH_TO_ACTIVATE;
	}
	else
	{
		currentButtonState = (InputHandler::m_currentGamePadState[playerIndex].Gamepad.wButtons & buttonKeyCode) != 0;
		previousButtonState = (InputHandler::m_previousGamePadState[playerIndex].Gamepad.wButtons & buttonKeyCode) != 0;
	}


	if(currentButtonState && !previousButtonState)
		return true;

	return false;
}

//-----------------------------------------------------------------------------------------------
void InputHandler::Vibrate(int leftVal, int rightVal, uint playerIndex)
{
	XINPUT_VIBRATION Vibration;
	memset(&Vibration,0,sizeof(XINPUT_VIBRATION));

	// Set the Vibration Values
	Vibration.wLeftMotorSpeed = (WORD)leftVal;
	Vibration.wRightMotorSpeed = (WORD)rightVal;

	// Vibrate the controller
	XInputSetState(playerIndex, &Vibration);
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isConnected(uint playerIndex)
{
	memset(&(InputHandler::m_currentGamePadState[playerIndex]),0, sizeof(XINPUT_STATE));

	// Get the state
	DWORD Result = XInputGetState(playerIndex, &(InputHandler::m_currentGamePadState[playerIndex]));

	if(Result == ERROR_SUCCESS)
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::ProcessJoystick(Vector2f& out_Direction, float& magnitude, uint playerIndex, uint requestedJoystick)
{
	Vector2f joystick(0.f,0.f);
	_XINPUT_GAMEPAD state = InputHandler::m_currentGamePadState[playerIndex].Gamepad;

	if(requestedJoystick == XINPUT_GAMEPAD_LEFT_JOY)
	{
		joystick.x = (float)state.sThumbLX;
		joystick.y = (float)state.sThumbLY;
	}
	else if(requestedJoystick == XINPUT_GAMEPAD_RIGHT_JOY)
	{
		joystick.x = (float)state.sThumbRX;
		joystick.y = (float)state.sThumbRY;
	}
	else
	{
		return false;
	}

	NormalizeThumbStick(joystick);

	float radius = (float)sqrt(joystick.x * joystick.x + joystick.y * joystick.y);

	if(radius > DEAD_ZONE_RADIUS)
	{
		radius = (radius - DEAD_ZONE_RADIUS) * ONE_MINUS_DEAD_ZONE_RADIUS_INVERSE;
		magnitude = radius;
		out_Direction = joystick;
		return true;
	}
	else
	{
		magnitude = 0.f;
		out_Direction = Vector2f(0,0);
		return false;
	}
}

//-----------------------------------------------------------------------------------------------
void InputHandler::NormalizeThumbStick(Vector2f& thumbstick)
{
	thumbstick.x += XBOX_CONTROLLER_MAX;
	thumbstick.y += XBOX_CONTROLLER_MAX;
	thumbstick *= XBOX_CONTROLLER_SHIFTED_MAX_INVERSE;
	thumbstick *= 2.f;
	thumbstick.x -= 1.f;
	thumbstick.y -= 1.f;
}

//-----------------------------------------------------------------------------------------------
void InputHandler::GetMouseDisplacementVector(Vector2i& out_mouseDisplacement)
{
	const Vector2i screenCenter(OpenGLRenderer::s_screenSize.x / 2, OpenGLRenderer::s_screenSize.y / 2);

	HWND focus = GetForegroundWindow();

	if(focus == m_clientWindow)
	{
			out_mouseDisplacement.x = (int)m_mousePosition->x - screenCenter.x;
			out_mouseDisplacement.y = (int)m_mousePosition->y - screenCenter.y;

			ResetMousePosition();
	}	

	else
	{
		out_mouseDisplacement.x = 0;
		out_mouseDisplacement.y = 0;
	}
}

//-----------------------------------------------------------------------------------------------
void InputHandler::ResetMousePosition()
{
	const Vector2i screenCenter(OpenGLRenderer::s_screenSize.x / 2, OpenGLRenderer::s_screenSize.y / 2);

	SetCursorPos(screenCenter.x,screenCenter.y);
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isLeftMouseDown()
{
	return InputHandler::m_currentMouseState[MOUSE_LEFT];
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isLeftMouseClick()
{
	bool currentLeftMouse = InputHandler::m_currentMouseState[MOUSE_LEFT];
	bool previousLeftMouse = InputHandler::m_previousMouseState[MOUSE_LEFT];

	if(currentLeftMouse && !previousLeftMouse)
		return true;
	return false;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isLeftMouseUp()
{
	return !(InputHandler::m_currentMouseState[MOUSE_LEFT]);
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isLeftMouseDoubleClick()
{
	return InputHandler::m_currentMouseState[MOUSE_L_DOUBLE_CLICK];
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isRightMouseDown()
{
	return InputHandler::m_currentMouseState[MOUSE_RIGHT];
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isRightMouseClick()
{
	bool currentRightMouse = InputHandler::m_currentMouseState[MOUSE_RIGHT];
	bool previousRightMouse = InputHandler::m_previousMouseState[MOUSE_RIGHT];

	if(currentRightMouse && !previousRightMouse)
		return true;
	return false;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isRightMouseUp()
{
	return !(InputHandler::m_currentMouseState[MOUSE_RIGHT]);
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isRightMouseDoubleClick()
{
	return InputHandler::m_currentMouseState[MOUSE_L_DOUBLE_CLICK];
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isMiddleMouseDown()
{
	return InputHandler::m_currentMouseState[MOUSE_MIDDLE];
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isMiddleMouseClick()
{
	bool currentMiddleMouse = InputHandler::m_currentMouseState[MOUSE_MIDDLE];
	bool previousMiddleMouse = InputHandler::m_previousMouseState[MOUSE_MIDDLE];

	if(currentMiddleMouse && !previousMiddleMouse)
		return true;
	return false;
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isMiddleMouseUp()
{
	return !(InputHandler::m_currentMouseState[MOUSE_RIGHT]);
}

//-----------------------------------------------------------------------------------------------
bool InputHandler::isMiddleMouseDoubleClick()
{
	return InputHandler::m_currentMouseState[MOUSE_M_DOUBLE_CLICK];
}

//-----------------------------------------------------------------------------------------------
InputHandler::InputHandlerState InputHandler::GetInputState()
{
	return InputHandler::s_state;
}

//-----------------------------------------------------------------------------------------------
void InputHandler::SetInputState(InputHandlerState state)
{
	InputHandler::s_state = state;
}