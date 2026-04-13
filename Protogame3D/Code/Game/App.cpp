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
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"

App* g_theApp = nullptr;				// Created and owned by Main_Windows.cpp
Renderer* g_theRenderer = nullptr;				// Created and owned by the App
InputSystem* g_inputSystem = nullptr;
AudioSystem* g_audioSystem = nullptr;
Window* g_window = nullptr;
Game* g_game = nullptr;

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
	g_game = new Game();
	g_game->Startup();
}


void App::DeleteGame()
{
	g_game->ShutDown();
	delete g_game;
	g_game = nullptr;
}


void App::InitialiseEngineSystems()
{
	InputConfig inputConfig;
	g_inputSystem = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_inputSystem;
	windowConfig.m_aspectRatio = g_gameConfigBlackboard.GetValue("windowAspect", 2.0f);
	windowConfig.m_windowTitle = g_gameConfigBlackboard.GetValue("windowTitle", "Game");
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
	BeginFrame();
	Update();
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
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
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
	Clock::SystemBeginFrame();

	g_inputSystem->BeginFrame();
	g_window->BeginFrame();
	g_theRenderer->BeginFrame();
	g_audioSystem->BeginFrame();
	g_console->BeginFrame();
	g_eventSystem->BeginFrame();
}


void App::Update()
{
	g_console->Update();
	CheckInput();
	//update game
	g_game->Update();
}


void App::Render() const
{
	//render context function calls - clear screen, begin camera, render ship end camera
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255)); //set the background of the game to black
	g_game->Render();

	Camera debugScreenCamera;
	float debugScreenCameraOrthoX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float debugScreenCameraOrthoY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	debugScreenCamera.SetOrthoView(0.0f, 0.0f, debugScreenCameraOrthoX, debugScreenCameraOrthoY);
	
	DebugRenderScreenToCamera(debugScreenCamera);
	
	g_theRenderer->BeginCamera(debugScreenCamera);
	AABB2 consoleBounds = debugScreenCamera.GetOrthoCamBoundingBox();
	g_console->Render(consoleBounds);
	g_theRenderer->EndCamera(debugScreenCamera);
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
