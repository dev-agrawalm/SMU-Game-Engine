#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include<Xinput.h>
#pragma comment( lib, "xinput9_1_0" ) 

#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"

XboxController::XboxController()
{

}


XboxController::XboxController(int id)
{
	m_id = id;

	SetJoystickDeadzoneThresholds(JOYSTICK_INNER_DEADZONE_THRESHOLD, JOYSTICK_OUTER_DEADZONE_THRESHOLD);
	Reset();
}


XboxController::~XboxController()
{

}


bool XboxController::IsConnected() const
{
	return m_isConnected;
}


int XboxController::GetControllerID() const
{
	return m_id;
}


AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftJoystick;
}


AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightJoystick;
}


float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}


float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}


KeyButtonState const& XboxController::GetButton(XBoxButtonID buttonID) const
{
	return m_buttons[buttonID];
}


bool XboxController::IsButtonDown(XBoxButtonID buttonID) const
{
	return m_buttons[buttonID].m_isPressed;
}


bool XboxController::IsButtonUp(XBoxButtonID buttonID) const
{
	return !m_buttons[buttonID].m_isPressed;
}


bool XboxController::WasButtonJustPressed(XBoxButtonID buttonID) const
{
	KeyButtonState key = m_buttons[buttonID];
	return (key.m_isPressed && !key.m_wasPressedLastFrame);
}


bool XboxController::WasButtonJustReleased(XBoxButtonID buttonID) const
{
	KeyButtonState key = m_buttons[buttonID];
	return (!key.m_isPressed && key.m_wasPressedLastFrame);
}


void XboxController::SetJoystickDeadzoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold)
{
	m_leftJoystick.SetDeadzoneThresholds(normalizedInnerDeadzoneThreshold, normalizedOuterDeadzoneThreshold);
	m_rightJoystick.SetDeadzoneThresholds(normalizedInnerDeadzoneThreshold, normalizedOuterDeadzoneThreshold);
}


void XboxController::Update()
{
	XINPUT_STATE xboxControllerState;
	memset(&xboxControllerState, 0, sizeof(xboxControllerState));
	DWORD errorStatus = XInputGetState(m_id, &xboxControllerState);
	if (errorStatus == ERROR_SUCCESS)
	{
		m_isConnected = true;

		UpdateJoystick(m_leftJoystick,	xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY);
		UpdateJoystick(m_rightJoystick, xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY);

		UpdateTrigger(m_leftTrigger,	xboxControllerState.Gamepad.bLeftTrigger);
		UpdateTrigger(m_rightTrigger,	xboxControllerState.Gamepad.bRightTrigger);

		UpdateButton(XBOX_BUTTON_A,						xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_A);
		UpdateButton(XBOX_BUTTON_B,						xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_B);
		UpdateButton(XBOX_BUTTON_X,						xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_X);
		UpdateButton(XBOX_BUTTON_Y,						xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_Y);
		UpdateButton(XBOX_BUTTON_START,					xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_START);
		UpdateButton(XBOX_BUTTON_SELECT,				xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_BACK);
		UpdateButton(XBOX_BUTTON_LEFT_BUMPER,			xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
		UpdateButton(XBOX_BUTTON_RIGHT_BUMPER,			xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		UpdateButton(XBOX_BUTTON_DPAD_UP,				xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP);
		UpdateButton(XBOX_BUTTON_DPAD_DOWN,				xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
		UpdateButton(XBOX_BUTTON_DPAD_LEFT,				xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
		UpdateButton(XBOX_BUTTON_DPAD_RIGHT,			xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
		UpdateButton(XBOX_BUTTON_LEFT_JOYSTICK_BUTTON,	xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
		UpdateButton(XBOX_BUTTON_RIGHT_JOYSTICK_BUTTON, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);
	}
	else
	{
		Reset();
	}
}


void XboxController::Reset()
{
	m_isConnected = false;

	m_leftJoystick.Reset();
	m_rightJoystick.Reset();

	m_leftTrigger = 0.0f;
	m_rightTrigger = 0.0f;

	for (int buttonIndex = 0; buttonIndex < (int) XBoxButtonID::NUM_XBOX_BUTTONS; buttonIndex++)
	{
		m_buttons[buttonIndex].m_isPressed = false;
		m_buttons[buttonIndex].m_wasPressedLastFrame = false;
	}
}


void XboxController::UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY)
{
	float normalisedRawX; 
	float rawXFloat = static_cast<float>(rawX);
	normalisedRawX = rawXFloat / MAX_SHORT_VALUE;

	float normalisedRawY;
	float rawYFloat = static_cast<float>(rawY);
	normalisedRawY = rawYFloat / MAX_SHORT_VALUE;

	out_joystick.UpdatePosition(normalisedRawX, normalisedRawY);
}


void XboxController::UpdateTrigger(float& out_trigger, unsigned char rawValue)
{
	float correctedValue = RangeMapClamped(static_cast<float>(rawValue), TRIGGER_LOWER_THRESHOLD, TRIGGER_UPPER_THRESHOLD, 0.0f, 1.0f);
	out_trigger = correctedValue;
}


void XboxController::UpdateButton(XBoxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag)
{
	if ((buttonFlag & buttonFlags) == buttonFlag)
	{
		m_buttons[buttonID].m_isPressed = true;
	}
	else
	{
		m_buttons[buttonID].m_isPressed = false;
	}
}
