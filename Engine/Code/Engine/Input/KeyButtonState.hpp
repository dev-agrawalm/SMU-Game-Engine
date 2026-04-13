#pragma once

class KeyButtonState
{
	friend class InputSystem;
	friend class XboxController;

private:
	bool m_wasPressedLastFrame = false;
	bool m_isPressed = false;
};

