#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/IntVec2.hpp"

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;
constexpr int NUM_MOUSE_BUTTON = 3;

extern const unsigned char KEYCODE_F1;
extern const unsigned char KEYCODE_F2;
extern const unsigned char KEYCODE_F3;
extern const unsigned char KEYCODE_F4;
extern const unsigned char KEYCODE_F5;
extern const unsigned char KEYCODE_F6;
extern const unsigned char KEYCODE_F7;
extern const unsigned char KEYCODE_F8;
extern const unsigned char KEYCODE_F9;
extern const unsigned char KEYCODE_F10;
extern const unsigned char KEYCODE_F11;
extern const unsigned char KEYCODE_ESCAPE;
extern const unsigned char KEYCODE_SPACE_BAR;
extern const unsigned char KEYCODE_SHIFT;
extern const unsigned char KEYCODE_ENTER;
extern const unsigned char KEYCODE_UP_ARROW;
extern const unsigned char KEYCODE_DOWN_ARROW;
extern const unsigned char KEYCODE_LEFT_ARROW;
extern const unsigned char KEYCODE_RIGHT_ARROW;
extern const unsigned char KEYCODE_TILDE;
extern const unsigned char KEYCODE_BACKSPACE;
extern const unsigned char KEYCODE_DELETE;
extern const unsigned char KEYCODE_HOME;
extern const unsigned char KEYCODE_END;
extern const unsigned char KEYCODE_CTRL;
extern const unsigned char KEYCODE_TAB;
extern const unsigned char KEYCODE_ALT;

class Window;
struct AABB2;

struct InputConfig
{
};


struct MouseConfig
{
	int m_priority = 0;
	bool m_isLocked = false;
	bool m_isHidden = false;
	bool m_isRelative = false;
};


class InputSystem
{
public:
	InputSystem();
	InputSystem(InputConfig const& config);
	~InputSystem();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();
	void ResetInput();
	void UpdateKeyStates();

	bool IsKeyPressed(unsigned char keyCode);
	bool IsKeyUp(unsigned char keyCode);
	bool WasKeyJustPressed(unsigned char keyCode);
	bool WasKeyJustReleased(unsigned char keyCode);
	
	bool IsMouseButtonPressed(int buttonIndex);
	bool IsMouseButtonUp(int buttonIndex);
	bool WasMouseButtonJustPressed(int buttonIndex);
	bool WasMouseButtonJustReleased(int buttonIndex);

	bool HandleKeyPressed(unsigned char keyCode);
	bool HandleKeyReleased(unsigned char keyCode);
	bool HandleMouseButtonPressed(int buttonIndex);
	bool HandleMouseButtonReleased(int buttonIndex);
	bool HandleMouseWheelMovement(int wheelMovement);

	void PushMouseConfig(MouseConfig const& config);
	void PopMouseConfigOfPriority(int priority);

	void OnWindowActivated();

	int GetMouseWheel() const;
	IntVec2 GetMouseDelta() const;
	IntVec2 GetMousePosition() const;
	Vec2 GetCurrentMouseWorldPosition2D(AABB2 const& windowOrtho, AABB2 const& worldOrtho);
	XboxController const& GetController(int controllerID) const;
protected:
	void ApplyMouseConfig(MouseConfig const& config);
	void ApplyTopMouseConfig();
	void SetDefaultMouseConfig(MouseConfig const& config);
	void ApplyDefaultMouseConfig();
	void UpdateMouseState();

private:
	KeyButtonState  m_keyStates[NUM_KEYCODES];
	KeyButtonState	m_mouseButtonStates[NUM_MOUSE_BUTTON];
	XboxController  m_controllers[NUM_XBOX_CONTROLLERS];
	InputConfig		m_inputConfig;

	MouseConfig				 m_defaultMouseConfig;
	MouseConfig				 m_activeMouseConfig;
	std::vector<MouseConfig> m_pushedMouseConfigs;

	bool	m_isMouseRelative = false;
	int		m_mouseWheel = 0;
	IntVec2 m_mouseDelta;
	IntVec2 m_previousMousePosition;
	IntVec2 m_currentMousePosition;
};
