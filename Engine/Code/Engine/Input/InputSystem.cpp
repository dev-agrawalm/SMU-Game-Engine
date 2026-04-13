#include "Engine/Input/InputSystem.hpp"
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include "Engine/Window/Window.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"

const unsigned char KEYCODE_F1			= VK_F1;
const unsigned char KEYCODE_F2			= VK_F2;
const unsigned char KEYCODE_F3			= VK_F3;
const unsigned char KEYCODE_F4			= VK_F4;
const unsigned char KEYCODE_F5			= VK_F5;
const unsigned char KEYCODE_F6			= VK_F6;
const unsigned char KEYCODE_F7			= VK_F7;
const unsigned char KEYCODE_F8			= VK_F8;
const unsigned char KEYCODE_F9			= VK_F9;
const unsigned char KEYCODE_F10			= VK_F10;
const unsigned char KEYCODE_F11			= VK_F11;
const unsigned char KEYCODE_ESCAPE		= VK_ESCAPE;
const unsigned char KEYCODE_SPACE_BAR	= VK_SPACE;
const unsigned char KEYCODE_SHIFT		= VK_SHIFT;
const unsigned char KEYCODE_ENTER		= VK_RETURN;
const unsigned char KEYCODE_UP_ARROW	= VK_UP;
const unsigned char KEYCODE_DOWN_ARROW	= VK_DOWN;
const unsigned char KEYCODE_LEFT_ARROW	= VK_LEFT;
const unsigned char KEYCODE_RIGHT_ARROW = VK_RIGHT;
const unsigned char KEYCODE_TILDE		= VK_OEM_3;
const unsigned char KEYCODE_BACKSPACE	= VK_BACK;
const unsigned char KEYCODE_DELETE		= VK_DELETE;
const unsigned char KEYCODE_HOME		= VK_HOME;
const unsigned char KEYCODE_END			= VK_END;
const unsigned char KEYCODE_CTRL		= VK_CONTROL;
const unsigned char KEYCODE_TAB			= VK_TAB;
const unsigned char KEYCODE_ALT			= VK_MENU;

InputSystem::InputSystem()
{

}


InputSystem::InputSystem(InputConfig const& config)
	: m_inputConfig (config)
{

}


InputSystem::~InputSystem()
{

}


void InputSystem::Startup()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex] = XboxController(controllerIndex);
	}

	m_defaultMouseConfig.m_isHidden = false;
	m_defaultMouseConfig.m_isLocked = false;
	m_defaultMouseConfig.m_isRelative = false;
	m_defaultMouseConfig.m_priority = -1;
	ApplyDefaultMouseConfig();
}


void InputSystem::Shutdown()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex].Reset();
	}
}


void InputSystem::BeginFrame()
{
	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex].Update();
	}

	UpdateMouseState();
}


void InputSystem::UpdateMouseState()
{
	Window* window = Window::GetMainWindowInstance();
	if (window && window->HasFocus())
	{
		m_currentMousePosition = window->GetMouseClientPosition();
		m_mouseDelta = m_currentMousePosition - m_previousMousePosition;

		if (m_isMouseRelative)
		{
			IntVec2 clientCenter = window->GetClientCenter();
			window->SetMouseClientPosition(clientCenter);
			m_currentMousePosition = window->GetMouseClientPosition();
		}

		m_previousMousePosition = m_currentMousePosition;
	}

	m_mouseWheel = 0;
}


void InputSystem::EndFrame()
{
	UpdateKeyStates();
}


void InputSystem::UpdateKeyStates()
{
	for (int index = 0; index < NUM_KEYCODES; index++)
	{
		m_keyStates[index].m_wasPressedLastFrame = m_keyStates[index].m_isPressed;
	}

	for (int index = 0; index < NUM_MOUSE_BUTTON; index++)
	{
		m_mouseButtonStates[index].m_wasPressedLastFrame = m_mouseButtonStates[index].m_isPressed;
	}

	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		if (m_controllers[controllerIndex].IsConnected())
		{
			XboxController& controller = m_controllers[controllerIndex];

			for (int buttonIndex = 0; buttonIndex < XboxController::NUM_XBOX_BUTTONS; buttonIndex++)
			{
				KeyButtonState& button = controller.m_buttons[buttonIndex];
				button.m_wasPressedLastFrame = button.m_isPressed;
			}
		}
	}
}


void InputSystem::ResetInput()
{
	for (int index = 0; index < NUM_KEYCODES; index++)
	{
		m_keyStates[index].m_wasPressedLastFrame = false;
		m_keyStates[index].m_isPressed = false;
	}

	for (int index = 0; index < NUM_MOUSE_BUTTON; index++)
	{
		m_mouseButtonStates[index].m_wasPressedLastFrame = false;
		m_mouseButtonStates[index].m_isPressed = false;
	}

	for (int controllerIndex = 0; controllerIndex < NUM_XBOX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex].Reset();
	}

	m_mouseDelta = IntVec2(0, 0);
}


bool InputSystem::IsKeyPressed(unsigned char keyCode)
{
	return m_keyStates[keyCode].m_isPressed;
}


bool InputSystem::IsKeyUp(unsigned char keyCode)
{
	return !m_keyStates[keyCode].m_isPressed;
}


bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	KeyButtonState& key = m_keyStates[keyCode];
	return (key.m_isPressed && !key.m_wasPressedLastFrame);
}


bool InputSystem::WasKeyJustReleased(unsigned char keyCode)
{
	KeyButtonState& key = m_keyStates[keyCode];
	return (key.m_wasPressedLastFrame && !key.m_isPressed);
}


bool InputSystem::IsMouseButtonPressed(int buttonIndex)
{
	return m_mouseButtonStates[buttonIndex].m_isPressed;
}


bool InputSystem::IsMouseButtonUp(int buttonIndex)
{
	return !m_mouseButtonStates[buttonIndex].m_isPressed;
}


bool InputSystem::WasMouseButtonJustPressed(int buttonIndex)
{
	KeyButtonState& key = m_mouseButtonStates[buttonIndex];
	return (key.m_isPressed && !key.m_wasPressedLastFrame);
}


bool InputSystem::WasMouseButtonJustReleased(int buttonIndex)
{
	KeyButtonState& key = m_mouseButtonStates[buttonIndex];
	return (key.m_wasPressedLastFrame && !key.m_isPressed);
}


bool InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressed = true;
	return true;
}


bool InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isPressed = false;
	return true;
}


bool InputSystem::HandleMouseButtonPressed(int buttonIndex)
{
	m_mouseButtonStates[buttonIndex].m_isPressed = true;
	return true;
}


bool InputSystem::HandleMouseButtonReleased(int buttonIndex)
{
	m_mouseButtonStates[buttonIndex].m_isPressed = false;
	return true;
}


bool InputSystem::HandleMouseWheelMovement(int wheelMovement)
{
	m_mouseWheel = wheelMovement;
	return true;
}


void InputSystem::PushMouseConfig(MouseConfig const& config)
{
	for (int configIndex = 0; configIndex < (int) m_pushedMouseConfigs.size(); configIndex++)
	{
		MouseConfig& pushedConfig = m_pushedMouseConfigs[configIndex];
		ASSERT_RECOVERABLE(pushedConfig.m_priority != config.m_priority, Stringf("You're pushing a mouse config with a priority that already exists. Priority: %i", config.m_priority));
	}

	m_pushedMouseConfigs.push_back(config);
	ApplyTopMouseConfig();
}


void InputSystem::PopMouseConfigOfPriority(int priority)
{
	auto it = m_pushedMouseConfigs.begin();

	while (it != m_pushedMouseConfigs.end())
	{
		if (it->m_priority == priority)
		{
			m_pushedMouseConfigs.erase(it);
			ApplyTopMouseConfig();
			return;
		}

		it++;
	}
}


void InputSystem::OnWindowActivated()
{
	ResetInput();
	Window* window = Window::GetMainWindowInstance();
	if (window && m_isMouseRelative)
	{
		IntVec2 clientCenter = window->GetClientCenter();
		window->SetMouseClientPosition(clientCenter);
		m_currentMousePosition = window->GetMouseClientPosition();
	}
	m_previousMousePosition = m_currentMousePosition;
	m_mouseDelta = IntVec2(0, 0);
}


int InputSystem::GetMouseWheel() const
{
	return m_mouseWheel;
}


void InputSystem::ApplyTopMouseConfig()
{
	if (m_pushedMouseConfigs.size() > 0)
	{
		MouseConfig configToApply = m_pushedMouseConfigs[0];
		for (int configIndex = 0; configIndex < (int) m_pushedMouseConfigs.size(); configIndex++)
		{
			MouseConfig config = m_pushedMouseConfigs[configIndex];
			if (config.m_priority > configToApply.m_priority)
			{
				configToApply = config;
			}
		}

		if (m_activeMouseConfig.m_priority != configToApply.m_priority)
			ApplyMouseConfig(configToApply);
	}
	else
	{
		ApplyDefaultMouseConfig();
	}
}


void InputSystem::SetDefaultMouseConfig(MouseConfig const& config)
{
	m_defaultMouseConfig = config;
}


void InputSystem::ApplyDefaultMouseConfig()
{
	if (m_defaultMouseConfig.m_priority != m_activeMouseConfig.m_priority)
		ApplyMouseConfig(m_defaultMouseConfig);
}


XboxController const& InputSystem::GetController(int controllerID) const
{
	return m_controllers[controllerID];
}


IntVec2 InputSystem::GetMouseDelta() const
{
	return m_mouseDelta;
}


IntVec2 InputSystem::GetMousePosition() const
{
	return m_currentMousePosition;
}


Vec2 InputSystem::GetCurrentMouseWorldPosition2D(AABB2 const& windowOrtho, AABB2 const& worldOrtho)
{
	Vec2 worldOrthoMins = worldOrtho.m_mins;
	Vec2 worldOrthoMaxs = worldOrtho.m_maxs;
	return RangeMap(m_currentMousePosition.GetVec2(), windowOrtho.m_mins, windowOrtho.m_maxs, Vec2(worldOrthoMins.x, worldOrthoMaxs.y), Vec2(worldOrthoMaxs.x, worldOrthoMins.y));
}


void InputSystem::ApplyMouseConfig(MouseConfig const& config)
{
	Window* window = Window::GetMainWindowInstance();
	if (window)
	{
		window->SetMouseLockStatus(config.m_isLocked);
		window->SetMouseVisibility(!config.m_isHidden);
		m_isMouseRelative = config.m_isRelative;

		m_activeMouseConfig = config;
	}
}
