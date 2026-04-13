#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DebugRender.hpp"

RandomNumberGenerator* g_rng = nullptr;

Game::Game()
{

}


Game::~Game()
{
	//ShutDown(); //as a safety measure
}


void Game::Startup()
{
	LoadAssets();

	g_rng = new RandomNumberGenerator();
	m_gameState = GameState::GAME_STATE_ATTRACT;
	InitAttractMode();
}


void Game::LoadAssets()
{
	//load audio
	g_audioSystem->CreateOrGetSound("Data/Audio/Click.mp3");
}


void Game::ShutDown()
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		DeInitAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		DeInitPlayMode();		break;
		case GameState::GAME_STATE_GAME_WON:	DeInitGameWonMode();	break;
		case GameState::GAME_STATE_GAME_OVER:	DeInitGameOverMode();	break;
		case GameState::NONE:					//fall through
		default: break;
	}

	delete g_rng;
	g_rng = nullptr;
}


void Game::HandleQuitRequest()
{
	g_theApp->HandleQuitRequested();
}


void Game::CheckInputDeveloperCheats()
{
	//enable debug renderer
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_debugMode = !m_debugMode;
	}

	if (g_inputSystem->IsKeyPressed('O'))
	{
		m_gameClock.StepFrame();
	}

	//pause game
	if (g_inputSystem->WasKeyJustPressed('P'))
	{
		m_gameClock.TogglePause();
	}

	//manipulate time scale
	if (g_inputSystem->IsKeyPressed('R'))
	{
		SetTimeScale(1.0f);
	}
	else if (g_inputSystem->IsKeyPressed('T'))
	{
		SetTimeScale(0.2f);
	}
	else if (g_inputSystem->IsKeyPressed('Y'))
	{
		SetTimeScale(5.0f);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}
}


void Game::Update()
{
	CheckInputDeveloperCheats();
	float deltaSeconds = (float) m_gameClock.GetFrameDeltaSeconds();

	//check input
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		CheckInputAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		CheckInputPlayMode();		break;
		case GameState::GAME_STATE_GAME_OVER:	CheckInputGameOverMode();	break;
		case GameState::GAME_STATE_GAME_WON:	CheckInputGameWonMode();	break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}

	//update game mode
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		UpdateAttractMode(deltaSeconds);	break;
		case GameState::GAME_STATE_PLAYING:		UpdatePlayMode(deltaSeconds);		break;
		case GameState::GAME_STATE_GAME_OVER:	UpdateGameOverMode(deltaSeconds);	break;
		case GameState::GAME_STATE_GAME_WON:	UpdateGameWonMode(deltaSeconds);	break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}

	UpdateCameras();
}


void Game::UpdateCameras()
{
	float worldSizeX	= g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
	float worldSizeY	= g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);
	float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);

	m_worldCamera.SetOrthoView(0.0f, 0.0f, worldSizeX, worldSizeY);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, uiCanvasSizeX, uiCanvasSizeY);
}


void Game::Render() const
{

	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		RenderAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		RenderPlayMode();		break;
		case GameState::GAME_STATE_GAME_OVER:	RenderGameOverMode();	break;
		case GameState::GAME_STATE_GAME_WON:	RenderGameWonMode();	break;
		case GameState::NONE:					//fallthrough case
		default: break;
	}
}


void Game::InitAttractMode()
{
}


void Game::DeInitAttractMode()
{
}


void Game::CheckInputAttractMode()
{
	//keep quitting code in here and shift rest to their respective places
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ENTER))
	{
		SetGameState(GameState::GAME_STATE_PLAYING);
	}
}


void Game::UpdateAttractMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Game::RenderAttractMode() const
{
	g_theRenderer->CreateOrGetShader("Data/Shaders/Default");

 	g_theRenderer->ClearScreen(Rgba8::GREEN);

	std::vector<Vertex_PCU> triangle;
	//clockwise
	triangle.emplace_back(Vec3(50.0f, 50.0f, 0.0f), Rgba8::RED, Vec2::ZERO);
	triangle.emplace_back(Vec3(50.0f, 150.0f, 0.0f), Rgba8::RED, Vec2::ZERO);
	triangle.emplace_back(Vec3(150.0f, 50.0f, 0.0f), Rgba8::RED, Vec2::ZERO);

	//counter clockwise
	triangle.emplace_back(Vec3(50.0f, 150.0f, 0.0f), Rgba8::RED, Vec2::ZERO);
	triangle.emplace_back(Vec3(150.0f, 50.0f, 0.0f), Rgba8::RED, Vec2::ZERO);
	triangle.emplace_back(Vec3(150.0f, 150.0f, 0.0f), Rgba8::RED, Vec2::ZERO);


	g_theRenderer->BeginCamera(m_uiCamera);
	g_theRenderer->BindShaderByName("Data/Shaders/Default");
	g_theRenderer->DrawVertexArray((int) triangle.size(), triangle.data());
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::InitPlayMode()
{
	m_gameClock.Unpause();
}


void Game::DeInitPlayMode()
{
}


void Game::CheckInputPlayMode()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ENTER))
	{
		SoundID clickSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/Click.mp3");
		g_audioSystem->StartSound(clickSoundId);
	}
}


void Game::UpdatePlayMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_orientation += m_angularVelocity * deltaSeconds;
}


void Game::RenderPlayMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	//World Camera
	{
		g_theRenderer->BeginCamera(m_worldCamera);

		float worldSizeX = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
		float worldSizeY = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);

		Vec2 line = Vec2(5.0f, 0.0f).GetRotatedDegrees(m_orientation);
		Vec2 worldCenter = Vec2(worldSizeX * 0.5f, worldSizeY * 0.5f);
		DrawLine(worldCenter, worldCenter + line, 0.2f, Rgba8::MAGENTA);
		DrawRing(worldCenter, 10.0f, 0.5f, Rgba8::CYAN);

		if (m_debugMode)
		{
			RenderPlayModeDebugger();
		}

		g_theRenderer->EndCamera(m_worldCamera);
	}

	//UI Camera
	{
		g_theRenderer->BeginCamera(m_uiCamera);
		RenderPlayModeUI();
		g_theRenderer->EndCamera(m_uiCamera);
	}
}


void Game::RenderPlayModeDebugger() const
{
	
}


void Game::InitGameWonMode()
{

}


void Game::UpdateGameWonMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Game::CheckInputGameWonMode()
{

}


void Game::RenderGameWonMode() const
{

}


void Game::DeInitGameWonMode()
{

}


void Game::InitGameOverMode()
{

}


void Game::UpdateGameOverMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Game::CheckInputGameOverMode()
{

}


void Game::RenderGameOverMode() const
{

}


void Game::DeInitGameOverMode()
{

}


void Game::RenderPlayModeUI() const
{

}


void Game::SetGameState(GameState gameState)
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		DeInitAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		DeInitPlayMode();		break;
		case GameState::GAME_STATE_GAME_WON:	DeInitGameWonMode();	break;
		case GameState::GAME_STATE_GAME_OVER:	DeInitGameOverMode();	break;
		case GameState::NONE:					//fall through
		default: break;
	}

	m_gameState = gameState;

	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		InitAttractMode(); break;
		case GameState::GAME_STATE_PLAYING:		InitPlayMode(); break;
		case GameState::GAME_STATE_GAME_WON:	InitGameWonMode(); break;
		case GameState::GAME_STATE_GAME_OVER:	InitGameOverMode(); break;
		case GameState::NONE:					//fall through
		default: break;
	}
}


void Game::SetTimeScale(float timeScale)
{
	m_gameClock.SetTimeScale((double) timeScale);
}
