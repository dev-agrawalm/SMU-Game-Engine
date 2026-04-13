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
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/Block.hpp"
#include "Game/World.hpp"
#include "Game/GameCamera.hpp"

RandomNumberGenerator* g_rng = nullptr;
World* g_world = nullptr;

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
	float timeScale = 0.0f;
	args.GetProperty("scale", timeScale, 1.0f);
	g_game->SetGameTimeScale(timeScale);
	return true;
}


void Game::Startup()
{
	float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, uiCanvasSizeX, uiCanvasSizeY);

	DebugRenderSetVisible();
	LoadAssets();

	m_gameClock = new Clock();
	m_gameStateClock = new Clock(m_gameClock);
	g_rng = new RandomNumberGenerator();
	SetGameState(GameState::GAME_STATE_ATTRACT);

	g_eventSystem->SubscribeEventCallbackFunction("GameSetPlayerSpeed", Static_SetPlayerSpeed);
	g_eventSystem->SubscribeEventCallbackFunction("GameSetTimeScale", OnSetGameTimeScale);
}


void Game::LoadAssets()
{
	//load audio
	g_audioSystem->CreateOrGetSound("Data/Audio/Click.mp3");

	//load textures
	g_theRenderer->CreateOrGetTexture("Data/Images/TestUV.png");
	g_theRenderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
}


void Game::ShutDown()
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		DeInitAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		DeInitPlayMode();		break;
		case GameState::GAME_STATE_GAME_WON:	DeInitGameWonMode();	break;
		case GameState::GAME_STATE_GAME_OVER:	DeInitGameOverMode();	break;
		case GameState::NONE:					//fallthrough
		default: break;
	}

	delete g_rng;
	g_rng = nullptr;

	g_eventSystem->UnsubscribeEventCallbackFunction("SetPlayerSpeed", Static_SetPlayerSpeed);
	g_eventSystem->UnsubscribeEventCallbackFunction("GameSetTimeScale", OnSetGameTimeScale);

// 	for (int threadIndex = 0; threadIndex < NUM_THREADS; threadIndex++)
// 	{
// 		if (g_threads[threadIndex] != nullptr)
// 		{
// 			delete g_threads[threadIndex];
// 			g_threads[threadIndex] = nullptr;
// 		}
// 	}
}


void Game::HandleQuitRequest()
{
	g_theApp->HandleQuitRequested();
}


void Game::CheckInputDeveloperCheats()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_debugMode = !m_debugMode;
	}

	if (g_inputSystem->IsKeyPressed('O'))
	{
		m_gameClock->StepFrame();
	}

	if (g_inputSystem->WasKeyJustPressed('P'))
	{
		m_gameClock->TogglePause();
	}

	SetGameTimeScale(1.0f);
	static bool runAtFastSpeed = g_gameConfigBlackboard.GetValue("debugRunAtFastSpeed", false);
	if (g_inputSystem->WasKeyJustPressed('K'))
	{
		runAtFastSpeed = false;
	}
	if (g_inputSystem->IsKeyPressed('Y') || runAtFastSpeed)
	{
		float timeModifier = g_gameConfigBlackboard.GetValue("worldTimeScaleFastModifier", 20000.0f);
		SetGameTimeScale(timeModifier);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}
}


void Game::Update()
{
// 	float textHeight = 16.0f;
// 	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
// 	Clock& systemClock = Clock::GetSystemClock();
// 	std::string systemClockString = "System -- " + ClockToString(&systemClock);
// 	std::string gameClockString = "Game   -- " + ClockToString(m_gameClock);
// 	std::string gameStateClockString = "State  -- " + ClockToString(m_gameStateClock);
// 	Vec2 cursorPos = Vec2(0.0f, uiCanvasSizeY);
// 	DebugAddScreenText(systemClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);
// 	cursorPos.y -= textHeight;
// 	DebugAddScreenText(gameClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);
// 	cursorPos.y -= textHeight;
// 	DebugAddScreenText(gameStateClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);

	CheckInputDeveloperCheats();
	float deltaSeconds = static_cast<float>(m_gameClock->GetFrameDeltaSeconds());
	deltaSeconds = Clamp(deltaSeconds, 0.0f, FRAME_TIME_10_FPS);

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
}


void Game::Render() const
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:		RenderAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		RenderPlayMode();		break;
		case GameState::GAME_STATE_GAME_OVER:	RenderGameOverMode();	break;
		case GameState::GAME_STATE_GAME_WON:	RenderGameWonMode();	break;
		case GameState::NONE: //fallthrough case
		default: break;
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
}


void Game::DeInitAttractMode()
{
	g_inputSystem->PopMouseConfigOfPriority(0);
}


void Game::CheckInputAttractMode()
{
	XboxController const& controller = g_inputSystem->GetController(0);
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_B))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_START))
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
 	g_theRenderer->ClearScreen(Rgba8::BLACK);

	std::vector<Vertex_PCU> text;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 textBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
	font->AddVertsForTextInAABB2(text, textBox, 800.0f, "SIMPLE MINER", Rgba8::WHITE);

	g_theRenderer->BeginCamera(m_uiCamera);
	Texture const& fontTexture = font->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) text.size(), text.data());
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::InitPlayMode()
{
	m_gameClock->Unpause();
	m_worldCamera.Initialise();

	MouseConfig config = {};
	config.m_isHidden = true;
	config.m_isLocked = false;
	config.m_isRelative = true;
	config.m_priority = 0;
	g_inputSystem->PushMouseConfig(config);
	
	BlockDef::GenerateAllBlockDefs();

	m_player = new Player(Vec3(0.0f, 0.0f, 100.0f));
	m_player->SetOrientation(EulerAngles(0.0f, 90.0f, 0.0f));
	m_worldCamera.AttachToEntity(m_player);

	g_world = new World(m_player);
	g_world->Create();

	Mat44 identity;
	DebugAddWorldBasis(identity, -1.0f);
}


void Game::DeInitPlayMode()
{
	if (m_player)
	{
		delete m_player;
		m_player = nullptr;
	}

	if (g_world)
	{
		g_world->Destroy();
		delete g_world;
		g_world = nullptr;
	}

	g_inputSystem->PopMouseConfigOfPriority(0);
}


void Game::CheckInputPlayMode()
{
	XboxController const& controller = g_inputSystem->GetController(0);
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_B))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}
}


void Game::UpdatePlayMode(float deltaSeconds)
{
// 	Vec3 playerPos = m_player->GetPosition();
// 	DebugAddScreenMessage(Stringf("Player Pos: x = %f  y = %f  z = %f", playerPos.x, playerPos.y, playerPos.z), 0.0f, 20.0f);

	g_world->Update(deltaSeconds);
	m_worldCamera.Update(deltaSeconds);
}


void Game::RenderPlayMode() const
{
	g_theRenderer->BeginCamera(m_worldCamera);
	{
		g_theRenderer->SetDepthOptions(DepthTest::LESS, true);
		g_world->Render();
	}
	g_theRenderer->EndCamera(m_worldCamera);

	DebugRenderWorldToCamera(m_worldCamera);
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
		case GameState::GAME_STATE_ATTRACT:		InitAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:		InitPlayMode();		break;
		case GameState::GAME_STATE_GAME_WON:	InitGameWonMode();	break;
		case GameState::GAME_STATE_GAME_OVER:	InitGameOverMode(); break;
		case GameState::NONE:					//fall through
		default: break;
	}
	m_gameStateClock->Reset(false);
}


void Game::SetGameTimeScale(float scale)
{
	m_gameClock->SetTimeScale((double) scale);
}


Player* Game::GetPlayer() const
{
	return m_player;
}


World* Game::GetWorld() const
{
	return g_world;
}


bool Game::Static_SetPlayerSpeed(EventArgs& args)
{
	Player* player = g_game->GetPlayer();
	float playerMoveSpeed			= player->GetMoveSpeed();
	float playerSprintSpeed			= player->GetSprintSpeed();
	float playerMouseSensitivity	= player->GetMouseSensitivity();

	float move = 0.0f;
	args.GetProperty("move", move, playerMoveSpeed);
	float sprint = 0.0f;
	args.GetProperty("sprint", sprint, playerSprintSpeed);
	float mouse = 0.0f;
	args.GetProperty("mouse", mouse, playerMouseSensitivity);

	player->SetMovementSpeed(move);
	player->SetSprintSpeed(sprint);
	player->SetMouseSensitivity(mouse);

	return true;
}
