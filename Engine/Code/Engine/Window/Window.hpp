#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include<string>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

class InputSystem;

struct WindowConfig
{
	InputSystem* m_inputSystem = nullptr;
	float m_aspectRatio = 1.0f;
	float m_maxClientFractionOfDesktopWidth = 0.90f;
	float m_maxClientFractionOfDesktopHeight = 0.90f;
	std::string m_windowTitle = "UNNAMED APPLICATION";
};

class Window
{
public: //methods
	Window();
	Window(WindowConfig const& config);
	~Window();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	void OnWindowsActivated();
	void OnWindowsDeactivated();

	void SetMouseVisibility(bool isMouseVisible);
	void SetMouseLockStatus(bool isMouseLocked);
	bool IsMouseVisible() const;
	bool IsMouseLocked() const;

	void SetMouseDesktopPosition(IntVec2 const& position);
	void SetMouseClientPosition(IntVec2 const& position);

	Vec2 GetCurrentNormalizedMouseScreenPosition() const;
	Vec2 GetNormalizedMouseScreenPosition(IntVec2 const& refMousePos) const;
	IntVec2 GetMouseDesktopPosition() const;
	IntVec2 GetMouseClientPosition() const;

	bool  HasFocus() const;
	void* GetWindowHandle() const;
	void* GetDisplayContext() const;
	Vec2  GetWindowDims() const;
	IntVec2 GetClientCenter() const;
	static Window* GetMainWindowInstance();
	WindowConfig const& GetWindowConfig() const;
protected:
	void RunMessagePump();
	void CreateOSWindow();
	void ShowMouse(bool showMouse);
	void LockMouse(bool lockToClient);

protected:
	static Window* s_mainWindow;

protected:
	WindowConfig m_windowConfig;
	void* m_windowHandle = nullptr;	//HWND in windows
	void* m_displayContext = nullptr;	// HDC in windows
	Vec2 m_windowDims;

	bool m_isMouseVisible = false;
	bool m_isMouseLocked = false;
};
