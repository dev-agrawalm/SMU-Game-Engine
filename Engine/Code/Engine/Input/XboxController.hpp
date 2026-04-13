#pragma once
#include"Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Core/EngineCommon.hpp"

constexpr float TRIGGER_LOWER_THRESHOLD = 30.0f;
constexpr float TRIGGER_UPPER_THRESHOLD = 250.0f;

constexpr float JOYSTICK_INNER_DEADZONE_THRESHOLD = 0.30f;
constexpr float JOYSTICK_OUTER_DEADZONE_THRESHOLD = 0.90f;

class XboxController
{
	friend class InputSystem;

public:
	enum XBoxButtonID
	{
		INVALID = -1,
		XBOX_BUTTON_A = 0,
		XBOX_BUTTON_B = 1,
		XBOX_BUTTON_X = 2,
		XBOX_BUTTON_Y = 3,
		XBOX_BUTTON_DPAD_UP = 4,
		XBOX_BUTTON_DPAD_DOWN = 5,
		XBOX_BUTTON_DPAD_LEFT = 6,
		XBOX_BUTTON_DPAD_RIGHT = 7,
		XBOX_BUTTON_RIGHT_BUMPER = 8,
		XBOX_BUTTON_LEFT_BUMPER = 9,
		XBOX_BUTTON_START = 10,
		XBOX_BUTTON_SELECT = 11,
		XBOX_BUTTON_LEFT_JOYSTICK_BUTTON = 12,
		XBOX_BUTTON_RIGHT_JOYSTICK_BUTTON = 13,
		NUM_XBOX_BUTTONS = 14
	};

	XboxController();
	XboxController(int id);
	~XboxController();
	
	//controller queries
	bool IsConnected() const;
	int GetControllerID() const;
	
	//joystick queries
	AnalogJoystick const& GetLeftStick() const;
	AnalogJoystick const& GetRightStick() const;
	
	//trigger queries
	float GetLeftTrigger() const;
	float GetRightTrigger() const;
	
	//button queries
	KeyButtonState const& GetButton(XBoxButtonID buttonID) const;
	bool IsButtonDown(XBoxButtonID buttonID) const;
	bool IsButtonUp(XBoxButtonID buttonID) const;
	bool WasButtonJustPressed(XBoxButtonID buttonID) const;
	bool WasButtonJustReleased(XBoxButtonID buttonID) const;

	void SetJoystickDeadzoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold);
private:
	void Update();
	void Reset();
	void UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY);
	void UpdateTrigger(float& out_trigger, unsigned char rawValue);
	void UpdateButton(XBoxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag);
private:
	int m_id = -1;
	bool m_isConnected = false;
	float m_leftTrigger = 0.0f;
	float m_rightTrigger = 0.0f;
	KeyButtonState m_buttons[(int)XBoxButtonID::NUM_XBOX_BUTTONS];
	AnalogJoystick m_leftJoystick;
	AnalogJoystick m_rightJoystick;
};
