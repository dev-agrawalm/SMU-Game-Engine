#include "Game/App.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/JobSystem.hpp"

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
	InitializeGameConfig();
	InitialiseEngineSystems();
	g_eventSystem->SubscribeEventCallbackFunction("Quit", App::StaticHandleQuitRequest);
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
	InputConfig inputConfig = {};
	g_inputSystem = new InputSystem(inputConfig);

	WindowConfig windowConfig = {};
	windowConfig.m_inputSystem = g_inputSystem;
	windowConfig.m_aspectRatio = g_gameConfigBlackboard.GetValue("windowAspect", 2.0f);
	windowConfig.m_windowTitle = g_gameConfigBlackboard.GetValue("windowTitle", "Game");
	g_window = new Window(windowConfig);

	RenderConfig renderConfig = {};
	renderConfig.m_window = g_window;
	g_theRenderer = new Renderer(renderConfig);

	AudioConfig audioConfig = {};
	g_audioSystem = new AudioSystem(audioConfig);

	DevConsoleConfig consoleConfig = {};
	consoleConfig.m_defaultFontName = "SquirrelFixedFont";
	consoleConfig.m_defaultRenderer = g_theRenderer;
	consoleConfig.m_inputSystem = g_inputSystem;
	g_console = new DevConsole(consoleConfig);

	EventSystemConfig eventSystemConfig = {};
	g_eventSystem = new EventSystem(eventSystemConfig);

	JobSystemConfig jobSysConfig = {};
	jobSysConfig.m_numAdditionalThreads = 2;
	g_jobSystem = new JobSystem(jobSysConfig);

	g_inputSystem->Startup();
	g_window->Startup();
	g_theRenderer->Startup();
	g_audioSystem->Startup();
	g_console->Startup();
	g_eventSystem->Startup();
	g_jobSystem->Startup();
}


void App::DeInitialiseSubSystems()
{
	g_jobSystem->Shutdown();
	g_eventSystem->Shutdown();
	g_console->Shutdown();
	g_audioSystem->Shutdown();
	g_theRenderer->ShutDown();
	g_window->ShutDown();
	g_inputSystem->Shutdown();
	
	delete g_jobSystem;
	g_jobSystem = nullptr;

	delete g_eventSystem;
	g_eventSystem = nullptr;

	delete g_console;
	g_console = nullptr;

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


bool App::IsQuitting() const
{
	return m_isQuitting;
}


void App::CheckInput()
{
	//delete the game and create a new one
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F7))
	{
		DeleteGame();
		CreateGame();
	}
}


bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return true;
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
	CheckInput();
	g_console->Update();
	//update game
	m_game->Update(deltaSeconds);
}


void App::Render() const
{
	//render context function calls - clear screen, begin camera, render ship end camera
	m_game->Render();

	Camera debuggerCamera;
	float debuggerCameraOrthoX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float debuggerCameraOrthoY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	debuggerCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(debuggerCameraOrthoX, debuggerCameraOrthoY));
	
	DebugRenderScreenToCamera(debuggerCamera);
	
	g_theRenderer->BeginCamera(debuggerCamera);
	AABB2 consoleBounds = debuggerCamera.GetOrthoCamBoundingBox();
	g_console->Render(consoleBounds);
	g_theRenderer->EndCamera(debuggerCamera);
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


void App::InitializeGameConfig()
{
	XmlDocument gameConfigXmlFile;
	XmlError result = gameConfigXmlFile.LoadFile("Data/XMLData/GameConfig.xml");
	GUARANTEE_OR_DIE(result == XmlError::XML_SUCCESS, "Failed to load GameConfig file");

	XmlElement* gameConfigElement = gameConfigXmlFile.RootElement();
	GUARANTEE_OR_DIE((gameConfigElement && _stricmp(gameConfigElement->Name(), "GameConfig") == 0), "Unable to find the GameConfig Element");

	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*gameConfigElement);
}


bool App::StaticHandleQuitRequest(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return false;
}
