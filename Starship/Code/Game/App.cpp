#include "Game/App.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"

App* g_theApp = nullptr;				// Created and owned by Main_Windows.cpp
Renderer* g_theRenderer = nullptr;				// Created and owned by the App
InputSystem* g_inputSystem = nullptr;
AudioSystem* g_audioSystem = nullptr;
Window* g_window = nullptr;

App::App()
{

}

App::~App()
{

}

void App::Startup()
{
	InitialiseEngineSystems();
	CreateGame();
}

void App::ShutDown()
{
	DeleteGame();
	DeInitialiseSubSystems();
}

void App::CreateGame()
{
	m_game = new Game();
	m_game->Startup();
}

void App::DeleteGame()
{
	m_game->ShutDown();
	delete m_game;
	m_game = nullptr;
}

void App::InitialiseEngineSystems()
{
	InputConfig inputConfig;
	g_inputSystem = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_inputSystem;
	windowConfig.m_aspectRatio = WINDOW_ASPECT_RATIO;
	windowConfig.m_windowTitle = "Starship Gold";
	g_window = new Window(windowConfig);

	RenderConfig renderConfig;
	renderConfig.m_window = g_window;
	g_theRenderer = new Renderer(renderConfig);

	AudioConfig audioConfig;
	g_audioSystem = new AudioSystem(audioConfig);

	DevConsoleConfig consoleConfig;
	consoleConfig.m_defaultFontName = "SquirrelFixedFont";
	consoleConfig.m_defaultRenderer = g_theRenderer;
	consoleConfig.m_inputSystem = g_inputSystem;
	g_console = new DevConsole(consoleConfig);

	EventSystemConfig eventSystemConfig;
	g_eventSystem = new EventSystem(eventSystemConfig);
	
	g_inputSystem->Startup();
	g_window->Startup();
	g_theRenderer->Startup();
	g_audioSystem->Startup();
	g_console->Startup();
	g_eventSystem->Startup();
}

void App::DeInitialiseSubSystems()
{
	g_eventSystem->Shutdown();
	g_console->Shutdown();
	g_audioSystem->Shutdown();
	g_theRenderer->ShutDown();
	g_window->ShutDown();
	g_inputSystem->Shutdown();
	
	delete g_eventSystem;
	g_eventSystem = nullptr;

	delete g_eventSystem;
	g_eventSystem = nullptr;

	delete g_audioSystem;
	g_audioSystem = nullptr;
	
	delete g_theRenderer;
	g_theRenderer = nullptr;
	
	delete g_window;
	g_window = nullptr;
	
	delete g_inputSystem;
	g_inputSystem = nullptr;
}

void App::RunFrame()
{
	m_currentFrameStartTime = static_cast<float>(GetCurrentTimeSeconds());
	float deltaSeconds = m_currentFrameStartTime - m_previousFrameStartTime;
	deltaSeconds = Clamp(deltaSeconds, 0.0f, FRAME_TIME_10_FPS);
	m_previousFrameStartTime = m_currentFrameStartTime;

	BeginFrame();
	Update(deltaSeconds);
	Render();
	EndFrame();
}

bool App::IsPaused() const
{
	return m_isPaused;
}

bool App::IsQuitting() const
{
	return m_isQuitting;
}

void App::CheckInput()
{
	//run a single time step
	if (g_inputSystem->IsKeyPressed('O'))
	{
		m_runSingleStep = true;
		m_isPaused = !m_isPaused;
	}
	
	//pause game
	if (g_inputSystem->WasKeyJustPressed('P'))
	{
		m_isPaused = !m_isPaused;
	}
	
	//slow down time
	if (g_inputSystem->IsKeyPressed('T'))
	{
		m_isSlowMo = true;
	}
	else
	{
		m_isSlowMo = false;
	}

	//delete the game and create a new one
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
	{
		DeleteGame();
		CreateGame();
		m_isPaused = false;
	}
}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return true;
}

void App::SetPause(bool isPaused)
{
	m_isPaused = isPaused;
}

void App::BeginFrame()
{
	g_inputSystem->BeginFrame();
	g_window->BeginFrame();
	g_theRenderer->BeginFrame();
	g_audioSystem->BeginFrame();
	g_console->BeginFrame();
	g_eventSystem->BeginFrame();
}

void App::Update(float deltaSeconds)
{
	g_console->Update();
	CheckInput();

	//slow down time
	if (m_isSlowMo)
	{
		deltaSeconds /= 10.0f; //slow down time to 1/10th the time
	}
	
	//update game
	m_game->Update(deltaSeconds);

	//end single step
	if (m_runSingleStep)
	{
		m_runSingleStep = false;
		m_isPaused = true;
	}
}

void App::Render() const
{
	//render context function calls - clear screen, begin camera, render ship end camera
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255)); //set the background of the game to black
	m_game->Render();
}

void App::EndFrame()
{
	g_eventSystem->EndFrame();
	g_console->EndFrame();
	g_audioSystem->EndFrame();
	g_theRenderer->EndFrame();
	g_window->EndFrame();
	g_inputSystem->EndFrame();
}