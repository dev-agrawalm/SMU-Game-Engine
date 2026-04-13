#include "Engine/Window/Window.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h> 
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include<zmouse.h>
#include "ThirdParty/ImGUI/imgui_impl_win32.h"

Window* Window::s_mainWindow = nullptr;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	Window const* window = Window::GetMainWindowInstance();
	InputSystem* input = nullptr;
	GUARANTEE_OR_DIE(window, "Main Window was null");
	input = window->GetWindowConfig().m_inputSystem;
	GUARANTEE_OR_DIE(input, "Input system was null");
	
	if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam))
		return true;

	switch (wmMessageCode)
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			if (g_eventSystem)
			{
				g_eventSystem->FireEvent("Quit");
				return 0; // "Consumes" this message (tells Windows "okay, we handled it")
			}
		}

		case WM_CHAR:
		{
			unsigned char asChar = (unsigned char) wParam;
			bool wasConsumed = false;
			if (g_console)
			{
				wasConsumed = g_console->HandleCharInput(asChar);

				if (g_eventSystem)
				{
					EventArgs args;
					std::string key(1, asChar);
					args.AddProperty("InputKey", key);
					g_eventSystem->FireEvent("CharDown", args);
				}

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char) wParam;

			if (input)
			{
				bool wasConsumed;
				wasConsumed = input->HandleKeyPressed(asKey);
				
				if (g_eventSystem)
				{
					EventArgs args;
					std::string key(1, asKey);
					args.AddProperty("InputKey", key);
					g_eventSystem->FireEvent("KeyDown", args);
				}

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		case WM_SYSKEYDOWN:
		{
			unsigned char asKey = (unsigned char) wParam;

			if (input)
			{
				bool wasConsumed;
				wasConsumed = input->HandleKeyPressed(asKey);

				if (g_eventSystem)
				{
					EventArgs args;
					std::string key(1, asKey);
					args.AddProperty("InputKey", key);
					g_eventSystem->FireEvent("KeyDown", args);
				}

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char) wParam;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyReleased(asKey);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		case WM_SYSKEYUP:
		{
			unsigned char asKey = (unsigned char) wParam;
			bool wasConsumed = false;
			if (input)
			{
				wasConsumed = input->HandleKeyReleased(asKey);
				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}

		case WM_ACTIVATE:
		{
			bool isActive = wParam != WA_INACTIVE;
			Window* mainWindow = Window::GetMainWindowInstance();
			if (mainWindow)
			{
				if (isActive)
				{
					mainWindow->OnWindowsActivated();

					if (input)
					{
						input->OnWindowActivated();
					}
				}
				else
				{
					mainWindow->OnWindowsDeactivated();
				}
				return 0;
			}
			break;
		}
		case WM_RBUTTONDOWN:
		{
			if (input)
			{
				bool wasConsumed;
				wasConsumed = input->HandleMouseButtonPressed(1);

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			if (input)
			{
				bool wasConsumed;
				wasConsumed = input->HandleMouseButtonPressed(0);

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}
		case WM_MBUTTONDOWN:
		{
			if (input)
			{
				bool wasConsumed;
				wasConsumed = input->HandleMouseButtonPressed(2);

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}
		case WM_RBUTTONUP:
		{
			if (input)
			{
				bool wasConsumed;
				wasConsumed = input->HandleMouseButtonReleased(1);

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			if (input)
			{
				bool wasConsumed;
				wasConsumed = input->HandleMouseButtonReleased(0);

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}
		case WM_MBUTTONUP:
		{
			if (input)
			{
				bool wasConsumed;
				wasConsumed = input->HandleMouseButtonReleased(2);

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			if (input)
			{
 				bool wasConsumed = false;
				if (input)
				{
					int mouseWheel = ((int) wParam >> 16) / 120;
					wasConsumed = input->HandleMouseWheelMovement(mouseWheel);
				}

				if (wasConsumed)
				{
					return 0;
				}
			}
			break;
		}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}


Window::Window()
{
}


Window::Window(WindowConfig const& config)
	: m_windowConfig(config)
{
	if (!s_mainWindow)
	{
		s_mainWindow = this;
	}
}


Window::~Window()
{
}


void Window::Startup()
{
	CreateOSWindow();
}


void Window::BeginFrame()
{
	RunMessagePump();
}


void Window::EndFrame()
{
}


void Window::ShutDown()
{

}


void Window::SetMouseVisibility(bool isMouseVisible)
{
	if (m_isMouseVisible != isMouseVisible)
	{
		m_isMouseVisible = isMouseVisible;
		if (HasFocus())
		{
			ShowMouse(m_isMouseVisible);
		}
	}
}


void Window::SetMouseLockStatus(bool isMouseLocked)
{
	if (m_isMouseLocked != isMouseLocked)
	{
		m_isMouseLocked = isMouseLocked;
		if (HasFocus())
		{
			LockMouse(m_isMouseLocked);
		}
	}
}


void Window::ShowMouse(bool showMouse)
{
	if (showMouse)
	{
		int counter = ShowCursor(showMouse);
		while (counter < 0)
		{
			counter = ShowCursor(showMouse);
		}
	}
	else
	{
		int counter = ShowCursor(showMouse);
		while (counter >= 0)
		{
			counter = ShowCursor(showMouse);
		}
	}
}


bool Window::IsMouseVisible() const
{
	return m_isMouseVisible;
}


void Window::LockMouse(bool lockToClient)
{
	if (lockToClient)
	{
		RECT clientRect;
		POINT origin;
		origin.x = 0;
		origin.y = 0;
		GetClientRect((HWND) m_windowHandle, &clientRect);
		ClientToScreen((HWND) m_windowHandle, &origin);

		clientRect.left += origin.x;
		clientRect.right += origin.x;
		clientRect.top += origin.y;
		clientRect.bottom += origin.y;

		ClipCursor(&clientRect);
	}
	else
	{
		ClipCursor(nullptr);
	}
}


void Window::OnWindowsActivated()
{
	InputSystem*& input = m_windowConfig.m_inputSystem;
	if (input)
		input->PopMouseConfigOfPriority(500);
	SetFocus((HWND) m_windowHandle);
}


void Window::OnWindowsDeactivated()
{
	MouseConfig config = {};
	config.m_isHidden = false;
	config.m_isLocked = false;
	config.m_isRelative = false;
	config.m_priority = 500;
	InputSystem*& input = m_windowConfig.m_inputSystem;
	if (input)
		input->PushMouseConfig(config);
}


bool Window::IsMouseLocked() const
{
	return m_isMouseLocked;
}


void Window::SetMouseDesktopPosition(IntVec2 const& position)
{
	SetCursorPos(position.x, position.y);
}


void Window::SetMouseClientPosition(IntVec2 const& position)
{
	POINT mousePos;
	mousePos.x = position.x;
	mousePos.y = position.y;
	ClientToScreen((HWND) m_windowHandle, &mousePos);
	IntVec2 screenPos = IntVec2(mousePos.x, mousePos.y);
	SetMouseDesktopPosition(screenPos);
}


Vec2 Window::GetCurrentNormalizedMouseScreenPosition() const
{
	IntVec2 desktopPos = GetMouseDesktopPosition();
	POINT mousePos;
	mousePos.x = desktopPos.x;
	mousePos.y = desktopPos.y;
	ScreenToClient((HWND) m_windowHandle, &mousePos);
	RECT clientRect;
	GetClientRect((HWND) m_windowHandle, &clientRect);
	float normalizedX = (float) mousePos.x / clientRect.right;
	float normalizedY = (float) mousePos.y / clientRect.bottom;
	return Vec2(normalizedX, 1.0f - normalizedY);
}


Vec2 Window::GetNormalizedMouseScreenPosition(IntVec2 const& refMousePos) const
{
	RECT clientRect;
	GetClientRect((HWND) m_windowHandle, &clientRect);
	float normalizedX = (float) refMousePos.x / clientRect.right;
	float normalizedY = (float) refMousePos.y / clientRect.bottom;
	return Vec2(normalizedX, 1.0f - normalizedY);
}


IntVec2 Window::GetMouseDesktopPosition() const
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	return IntVec2(mousePos.x, mousePos.y);
}


IntVec2 Window::GetMouseClientPosition() const
{
	IntVec2 desktopPos = GetMouseDesktopPosition();
	POINT mousePos;
	mousePos.x = desktopPos.x;
	mousePos.y = desktopPos.y;
	ScreenToClient((HWND) m_windowHandle, &mousePos);

	IntVec2 clientPos(mousePos.x, mousePos.y);
	return clientPos;
}


bool Window::HasFocus() const
{
	HWND hwnd = GetForegroundWindow();
	return (hwnd == (HWND) m_windowHandle);
}


void* Window::GetWindowHandle() const
{
	return m_windowHandle;
}


void* Window::GetDisplayContext() const
{
	return m_displayContext;
}


WindowConfig const& Window::GetWindowConfig() const
{
	return m_windowConfig;
}


Window* Window::GetMainWindowInstance()
{
	return s_mainWindow;
}


Vec2 Window::GetWindowDims() const
{
	return m_windowDims;
}


IntVec2 Window::GetClientCenter() const
{
	RECT clientRect;
	GetClientRect((HWND) m_windowHandle, &clientRect);

	IntVec2 clientCenter;
	clientCenter.x = ((clientRect.right - clientRect.left) / 2);
	clientCenter.y = ((clientRect.bottom - clientRect.top) / 2);
	return clientCenter;
}


void Window:: RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}


void Window::CreateOSWindow()
{
	float clientAspect = m_windowConfig.m_aspectRatio;
	HMODULE applicationInstanceHandle = GetModuleHandle(NULL);
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = applicationInstanceHandle;
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	// #SD1ToDo: Add support for full screen mode (requires different window style flags than windowed mode)
	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER |  WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float) (desktopRect.right - desktopRect.left);
	float desktopHeight = (float) (desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * m_windowConfig.m_maxClientFractionOfDesktopWidth;
	float clientHeight = desktopHeight * m_windowConfig.m_maxClientFractionOfDesktopHeight;
	if (clientAspect > desktopAspect)
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * clientAspect;
	}
	m_windowDims = Vec2(clientWidth, clientHeight);

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int) clientMarginX;
	clientRect.right = clientRect.left + (int) clientWidth;
	clientRect.top = (int) clientMarginY;
	clientRect.bottom = clientRect.top + (int) clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_windowConfig.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	HWND hwnd = CreateWindowEx(
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
		NULL);

	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	HDC dc = ::GetDC(hwnd); //:: ensures the compiler know this function is not part of the windows class or any class

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	m_windowHandle = hwnd;
	m_displayContext = dc;
}
