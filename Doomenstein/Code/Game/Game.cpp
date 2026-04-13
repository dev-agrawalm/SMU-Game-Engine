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
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Prop.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/World.hpp"
#include "Engine/Window/Window.hpp"

RandomNumberGenerator* g_rng = nullptr;

constexpr float SLOW_TIME_SCALE = 0.2f;
constexpr float FAST_TIME_SCALE = 5.0f;

Game::Game()
{

}


Game::~Game()
{
}


bool OnSetGameTimeScale(EventArgs& args)
{
	float timeScale = args.GetProperty("scale", 1.0f);
	g_game->SetGameTimeScale(timeScale);
	return true;
}


bool OnToggleEditorMode(EventArgs& args)
{
	UNUSED(args);
	g_game->ToggleEditorMode();
	return false;
}


bool OnToggleDebugRaycast(EventArgs& args)
{
	UNUSED(args);
	g_game->ToggleDebugRaycasts();
	return false;
}


bool OnToggleDebugActors(EventArgs& args)
{
	UNUSED(args);
	g_game->ToggleDebugActors();
	return false;
}


bool OnLoadMap(EventArgs& args)
{
	std::string mapName = args.GetProperty("map", "Error");
	GameState gameState = g_game->GetGameState();
	if (gameState == GameState::GAME_STATE_PLAYING)
	{
		World* world = g_game->GetWorld();
		world->LoadMap(mapName);
	}
	if (gameState == GameState::GAME_STATE_ATTRACT)
	{
		g_game->SetGameState(GameState::GAME_STATE_PLAYING);
		World* world = g_game->GetWorld();
		world->LoadMap(mapName);
	}

	return true;
}


void Game::Startup()
{
	DebugRenderSetVisible();

	LoadAssets();

	m_gameClock = new Clock();
	m_gameStateClock = new Clock(m_gameClock);
	g_rng = new RandomNumberGenerator();
	SetGameState(GameState::GAME_STATE_ATTRACT);

	g_eventSystem->SubscribeEventCallbackFunction("GameSetTimeScale",		OnSetGameTimeScale);
	g_eventSystem->SubscribeEventCallbackFunction("LoadMap",				OnLoadMap);
	g_eventSystem->SubscribeEventCallbackFunction("ToggleEditorMode",OnToggleEditorMode);
	g_eventSystem->SubscribeEventCallbackFunction("ToggleRaycastDebugging", OnToggleDebugRaycast);
	g_eventSystem->SubscribeEventCallbackFunction("ToggleActorDebugging",	OnToggleDebugActors);

	Vec2 windowDims = g_window->GetWindowDims();
	m_uiCamera.SetScreenViewport(AABB2(0.0f, 0.0f, windowDims.x, windowDims.y));

	m_controllers[0] = &g_inputSystem->GetController(0);
	m_controllers[1] = &g_inputSystem->GetController(1);
	m_controllers[2] = &g_inputSystem->GetController(2);
	m_controllers[3] = &g_inputSystem->GetController(3);
}


void Game::LoadAssets()
{
	//load textures
	g_theRenderer->CreateOrGetTexture("Data/Images/TestUV.png");
	g_theRenderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
}


void Game::ShutDown()
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:
			DeInitAttractMode();
			break;
		case GameState::GAME_STATE_PLAYING:
			DeInitPlayMode();
			break;
		case GameState::GAME_STATE_GAME_WON:
			DeInitGameWonMode();
			break;
		case GameState::GAME_STATE_GAME_OVER:
			DeInitGameOverMode();
			break;
		case GameState::NONE:
		default:
			break;
	}

	delete g_rng;
	g_rng = nullptr;

	g_eventSystem->UnsubscribeEventCallbackFunction("GameSetTimeScale",			OnSetGameTimeScale);
	g_eventSystem->UnsubscribeEventCallbackFunction("LoadMap",					OnLoadMap);
	g_eventSystem->UnsubscribeEventCallbackFunction("ToggleEditorMode",			OnToggleEditorMode);
	g_eventSystem->UnsubscribeEventCallbackFunction("ToggleRaycastDebugging",	OnToggleDebugRaycast);
	g_eventSystem->UnsubscribeEventCallbackFunction("ToggleActorDebugging",		OnToggleDebugActors);
}


void Game::HandleQuitRequest()
{
	g_theApp->HandleQuitRequested();
}


void Game::CheckInputDeveloperCheats()
{
	if (g_inputSystem->IsKeyPressed('O'))
	{
		m_gameClock->StepFrame();
	}

	//pause game
	if (g_inputSystem->WasKeyJustPressed('P'))
	{
		m_gameClock->TogglePause();
	}

	//manipulate time scale
	if (g_inputSystem->IsKeyPressed('R'))
	{
		SetGameTimeScale(SLOW_TIME_SCALE);
	}
	else if (g_inputSystem->IsKeyPressed('T'))
	{
		SetGameTimeScale(1.0f);
	}
	else if (g_inputSystem->IsKeyPressed('Y'))
	{
		SetGameTimeScale(FAST_TIME_SCALE);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		ToggleEditorMode();
	}
}


void Game::Update()
{
	float textHeight = 16.0f;
	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	Clock& systemClock = Clock::GetSystemClock();
	std::string systemClockString = "System -- " + ClockToString(&systemClock);
	std::string gameClockString = "Game   -- " + ClockToString(m_gameClock);
	std::string gameStateClockString = "State  -- " + ClockToString(m_gameStateClock);
	Vec2 cursorPos = Vec2(0.0f, uiCanvasSizeY);
// 	DebugAddScreenText(systemClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);
// 	cursorPos.y -= textHeight;
// 	DebugAddScreenText(gameClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);
// 	cursorPos.y -= textHeight;
// 	DebugAddScreenText(gameStateClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);

	CheckInputDeveloperCheats();
	float deltaSeconds = static_cast<float>(m_gameStateClock->GetFrameDeltaSeconds());
	deltaSeconds = Clamp(deltaSeconds, 0.0f, FRAME_TIME_10_FPS);

	//check input
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		CheckInputAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		CheckInputPlayMode();		break;
		case GameState::GAME_STATE_LOBBY:		CheckInputLobbyMode();		break;
		case GameState::GAME_STATE_GAME_OVER:	CheckInputGameOverMode();	break;
		case GameState::GAME_STATE_GAME_WON:	CheckInputGameWonMode();	break;
		case GameState::NONE: //fallthrough case
		default:
			break;
	}

	//update game mode
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		UpdateAttractMode(deltaSeconds);	break;
		case GameState::GAME_STATE_PLAYING:		UpdatePlayMode(deltaSeconds);		break;
		case GameState::GAME_STATE_LOBBY:		UpdateLobbyMode(deltaSeconds);		break;
		case GameState::GAME_STATE_GAME_OVER:	UpdateGameOverMode(deltaSeconds);	break;
		case GameState::GAME_STATE_GAME_WON:	UpdateGameWonMode(deltaSeconds);	break;
		case GameState::NONE: //fallthrough case
		default:
			break;
	}

	UpdateCameras();
}


void Game::UpdateCameras()
{
	float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, uiCanvasSizeX, uiCanvasSizeY);
}


void Game::Render() const
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		RenderAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		RenderPlayMode();		break;
		case GameState::GAME_STATE_LOBBY:		RenderLobbyMode();		break;
		case GameState::GAME_STATE_GAME_OVER:	RenderGameOverMode();	break;
		case GameState::GAME_STATE_GAME_WON:	RenderGameWonMode();	break;
		case GameState::NONE: //fallthrough case
		default:
			break;
	}
}


void Game::InitAttractMode()
{
	MouseConfig config = {};
	config.m_isHidden = false;
	config.m_isLocked = false;
	config.m_isRelative = false;
	config.m_priority = 0;
	g_inputSystem->PushMouseConfig(config);

	std::string attractMusicPath = g_gameConfigBlackboard.GetValue("attractMusicPath", "");
	float attractMusicVolume = g_gameConfigBlackboard.GetValue("attractMusicVolume", 0.0f);
	SoundID attractMusic = g_audioSystem->CreateOrGetSound(attractMusicPath);
	m_attractMusicID = g_audioSystem->StartSound(attractMusic, true, attractMusicVolume);
}


void Game::DeInitAttractMode()
{
	g_inputSystem->PopMouseConfigOfPriority(0);
	g_audioSystem->StopSound(m_attractMusicID);

	if (m_world)
	{
		m_world->Destroy();
		delete m_world;
		m_world = nullptr;
	}
}


void Game::CheckInputAttractMode()
{
	XboxController const& controller = g_inputSystem->GetController(0);
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_B))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR))
	{
		m_firstPlayerControllerIndex = -1;
		SetGameState(GameState::GAME_STATE_LOBBY);
	}

	for (int controllerIndex = 0; controllerIndex < 4; controllerIndex++)
	{
		XboxController const* c = m_controllers[controllerIndex];
		if (c->IsConnected() && c->WasButtonJustPressed(XboxController::XBOX_BUTTON_A))
		{
			m_firstPlayerControllerIndex = controllerIndex;
			SetGameState(GameState::GAME_STATE_LOBBY);
		}
	}
}


void Game::UpdateAttractMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Game::RenderAttractMode() const
{
 	g_theRenderer->ClearScreen(Rgba8::BLACK);

	std::vector<Vertex_PCU> text;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 textBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
	font->AddVertsForTextInAABB2(text, textBox, 800.0f, "DOOMENSTEIN", Rgba8::WHITE);

	AABB2 textBox2 = AABB2(Vec2(900.0f, 200.0f), 900.0f, 450.0f);
	font->AddVertsForTextInAABB2(text, textBox2, 400.0f, "Keep the number of monsters below 70 for 30 seconds", Rgba8::WHITE);

	g_theRenderer->BeginCamera(m_uiCamera);
	Texture const& fontTexture = font->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) text.size(), text.data());
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::InitLobbyMode()
{
	if (m_world)
	{
		m_world->Destroy();
		delete m_world;
		m_world = nullptr;
	}

	m_gameClock->Unpause();

	m_world = new World();
	m_world->Create();

	m_world->AddPlayerToGame(m_firstPlayerControllerIndex);
}


void Game::DeInitLobbyMode()
{

}


void Game::CheckInputLobbyMode()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR) && m_world->IsControllerOccupied(-1))
	{
		SetGameState(GameState::GAME_STATE_PLAYING);
	}
	
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) && m_world->GetNumPlayers() == 1)
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}
	
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) && m_world->IsControllerOccupied(-1))
	{
		m_world->RemovePlayerFromGame(-1);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR) && !m_world->IsControllerOccupied(-1))
	{
		m_world->AddPlayerToGame(-1);
	}


	for (int controllerIndex = 0; controllerIndex < 4; controllerIndex++)
	{
		XboxController const* controller = m_controllers[controllerIndex];
		if (controller->IsConnected() && controller->WasButtonJustPressed(XboxController::XBOX_BUTTON_A) && m_world->IsControllerOccupied(controllerIndex))
		{
			SetGameState(GameState::GAME_STATE_PLAYING);
		}
		
		if (controller->IsConnected() && controller->WasButtonJustPressed(XboxController::XBOX_BUTTON_B) && m_world->IsControllerOccupied(controllerIndex))
		{
			m_world->RemovePlayerFromGame(controllerIndex);
		}
		
		if (controller->IsConnected() && controller->WasButtonJustPressed(XboxController::XBOX_BUTTON_A) && !m_world->IsControllerOccupied(controllerIndex))
		{
			m_world->AddPlayerToGame(controllerIndex);
		}
	}
}


void Game::UpdateLobbyMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_world->GetNumPlayers() == 0)
		SetGameState(GameState::GAME_STATE_ATTRACT);
}


void Game::RenderLobbyMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	int numPlayers = m_world->GetNumPlayers();
	BitmapFont* textFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	for (int playerIndex = 0; playerIndex < numPlayers; playerIndex++)
	{
		PlayerController* player = m_world->GetPlayer(playerIndex);
		if (player)
		{
			Camera playerScreenCamera = player->GetScreenCamera();
			int playerControllerIndex = player->GetControllerIndex();
			Vec2 hudDims = player->GetHUDDims();

			g_theRenderer->BeginCamera(playerScreenCamera);
			{
				std::vector<Vertex_PCU> lobbyBgVerts;
				std::vector<Vertex_PCU> lobbyTextVerts;
  
				Vec2 hudCenter = hudDims * 0.5f;
				AABB2 lobbyBgBox = AABB2(hudCenter, hudDims.x * 0.8f, hudDims.y * 0.8f);
				AddVertsForAABB2ToVector(lobbyBgVerts, lobbyBgBox, Rgba8::GREY);
				g_theRenderer->BindTexture(0, nullptr);
				g_theRenderer->DrawVertexArray((int) lobbyBgVerts.size(), lobbyBgVerts.data());

				Vec2 lobbyBgBoxDims = lobbyBgBox.GetDimensions();
				std::string playerText = Stringf("Player %i", playerIndex);
				std::string playerControlsText = playerControllerIndex >= 0 ? "Controller (A to start / B to leave)" : "Keyboard/Mouse (Space to start / Esc to leave)";
				AABB2 playerTextBox = AABB2(hudCenter, lobbyBgBoxDims.x * 0.6f, lobbyBgBoxDims.y * 0.2f);
				//DrawRing(hudCenter, 60.0f, 5.0f, Rgba8::MAGENTA);
				Vec2 playerControlsTextBoxCenter = lobbyBgBox.m_mins + Vec2(lobbyBgBoxDims.x * 0.5f, lobbyBgBoxDims.y * 0.2f);
				AABB2 playerControlsTextBox = AABB2(playerControlsTextBoxCenter, lobbyBgBoxDims.x * 0.4f, lobbyBgBoxDims.y * 0.2f);

				textFont->AddVertsForTextInAABB2(lobbyTextVerts, playerTextBox, lobbyBgBoxDims.y * 0.2f, playerText);
				textFont->AddVertsForTextInAABB2(lobbyTextVerts, playerControlsTextBox, lobbyBgBoxDims.y * 0.1f, playerControlsText);
				g_theRenderer->BindTexture(0, &textFont->GetTexture());
				g_theRenderer->DrawVertexArray((int) lobbyTextVerts.size(), lobbyTextVerts.data());
			}
			g_theRenderer->EndCamera(playerScreenCamera);
		}
	}

	g_theRenderer->BeginCamera(m_uiCamera);
	{
		std::vector<Vertex_PCU> textVerts;
		float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
		float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);

		std::string globalInstructions = m_world->IsControllerOccupied(-1) ? "Press A to join" : "Press Space or A to join";
		AABB2 textBox = AABB2(0.0f, 0.0f, uiCanvasSizeX * 0.4f, uiCanvasSizeY * 0.2f);
		textFont->AddVertsForTextInAABB2(textVerts, textBox, uiCanvasSizeY * 0.05f, globalInstructions, Rgba8::WHITE, 1.0f, BitmapFont::ALIGNED_BOTTOM_LEFT);
		g_theRenderer->BindTexture(0, &textFont->GetTexture());
		g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
	}
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::InitPlayMode()
{
	m_gameClock->Unpause();

	MouseConfig config = {};
	config.m_isHidden = true;
	config.m_isLocked = true;
	config.m_isRelative = true;
	config.m_priority = 0;
	g_inputSystem->PushMouseConfig(config);
}


void Game::DeInitPlayMode()
{
	g_inputSystem->PopMouseConfigOfPriority(0);
}


void Game::CheckInputPlayMode()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) && m_world->IsControllerOccupied(-1))
	{
		SetGameState(GameState::GAME_STATE_LOBBY);
		m_firstPlayerControllerIndex = -1;
	}

	for (int controllerIndex = 0; controllerIndex < 4; controllerIndex++)
	{
		XboxController const* controller = m_controllers[controllerIndex];
		if (controller->IsConnected() && controller->WasButtonJustPressed(XboxController::XBOX_BUTTON_B) && m_world->IsControllerOccupied(controllerIndex))
		{
			SetGameState(GameState::GAME_STATE_LOBBY);
			m_firstPlayerControllerIndex = controllerIndex;
		}
	}
}


void Game::UpdatePlayMode(float deltaSeconds)
{
	static Stopwatch s_playerPosTimer = Stopwatch(*m_gameClock, 1.0);
	m_world->Update(deltaSeconds);
}


void Game::RenderPlayMode() const
{
	m_world->Render();
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
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || g_inputSystem->GetController(0).WasButtonJustPressed(XboxController::XBOX_BUTTON_SELECT))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}
}


void Game::RenderGameWonMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	std::vector<Vertex_PCU> text;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 textBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
	font->AddVertsForTextInAABB2(text, textBox, 800.0f, "YOU WON!!!", Rgba8::WHITE);

	g_theRenderer->BeginCamera(m_uiCamera);
	Texture const& fontTexture = font->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) text.size(), text.data());
	g_theRenderer->EndCamera(m_uiCamera);
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
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || g_inputSystem->GetController(0).WasButtonJustPressed(XboxController::XBOX_BUTTON_SELECT))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}
}


void Game::RenderGameOverMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	std::vector<Vertex_PCU> text;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 textBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
	font->AddVertsForTextInAABB2(text, textBox, 800.0f, "YOU LOST :(", Rgba8::WHITE);

	g_theRenderer->BeginCamera(m_uiCamera);
	Texture const& fontTexture = font->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) text.size(), text.data());
	g_theRenderer->EndCamera(m_uiCamera);
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
		case GameState::GAME_STATE_LOBBY:		DeInitLobbyMode();		break;
		case GameState::GAME_STATE_GAME_WON:	DeInitGameWonMode();	break;
		case GameState::GAME_STATE_GAME_OVER:	DeInitGameOverMode();	break;
		case GameState::NONE: //fall through
		default:
			break;
	}

	switch (gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		InitAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		InitPlayMode();		break;
		case GameState::GAME_STATE_LOBBY:		InitLobbyMode();	break;
		case GameState::GAME_STATE_GAME_WON:	InitGameWonMode();	break;
		case GameState::GAME_STATE_GAME_OVER:	InitGameOverMode();	break;
		case GameState::NONE: //fall through
		default:
			break;
	}
	m_gameState = gameState;
	m_gameStateClock->Reset(false);
}


void Game::SetGameTimeScale(float scale)
{
	m_gameClock->SetTimeScale((double) scale);
}


void Game::ToggleEditorMode()
{
	m_inEditorMode = !m_inEditorMode;
}


void Game::ToggleDebugRaycasts()
{
	m_debugRaycasts = !m_debugRaycasts;
}


void Game::ToggleDebugActors()
{
	m_debugActors = !m_debugActors;
}


World* Game::GetWorld() const
{
	return m_world;
}


GameState Game::GetGameState() const
{
	return m_gameState;
}


Clock& Game::GetGameStateClock() const
{
	return *m_gameStateClock;
}


Clock& Game::GetGameClock() const
{
	return *m_gameClock;
}


bool Game::IsEditorModeActive() const
{
	return m_inEditorMode;
}


bool Game::ShouldDebugRaycasts() const
{
	return m_debugRaycasts;
}


bool Game::ShouldDebugActors() const
{
	return m_debugActors;
}
