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


void Game::Startup()
{
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
	DebugAddScreenText(systemClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);
	cursorPos.y -= textHeight;
	DebugAddScreenText(gameClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);
	cursorPos.y -= textHeight;
	DebugAddScreenText(gameStateClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);

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

	UpdateCameras();
}


void Game::UpdateCameras()
{
	float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, uiCanvasSizeX, uiCanvasSizeY);

	if (m_player)
	{
		Mat44 persProjMat = Mat44::CreatePerspectiveProjectionMatrix(60.0f, g_window->GetWindowConfig().m_aspectRatio, 0.1f, 100.0f);
		m_worldCamera.SetProjectionMatrix(persProjMat);

		Mat44 camModelMat = m_player->GetModelMatrix();
		m_worldCamera.SetCameraModelMatrix(camModelMat);
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
	config.m_isLocked = true;
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
 	g_theRenderer->ClearScreen(Rgba8::CYAN);

	std::vector<Vertex_PCU> text;
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	AABB2 textBox = AABB2(Vec2(900.0f, 450.0f), 900.0f, 450.0f);
	font->AddVertsForTextInAABB2(text, textBox, 800.0f, "ATTRACT", Rgba8::WHITE);

	g_theRenderer->BeginCamera(m_uiCamera);
	Texture const& fontTexture = font->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) text.size(), text.data());
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::InitPlayMode()
{
	m_gameClock->Unpause();
	m_player = new Player(Vec3(0.0f, 0.0f, 5.0f));
	m_cube = new Prop(Vec3(10.0f, 5.0f, 0.0f));
	Prop* sphere = new Prop(Vec3(10.0, -5.0f, 1.0f));

	std::vector<Vertex_PCU> vertices;
	AABB3 cube = AABB3(Vec3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f, 1.0f);
	CubeFaceTints tints = {};
	tints.eastTint = Rgba8::GREEN;
	tints.northTint = Rgba8::MAGENTA;
	tints.westTint = Rgba8::RED;
	tints.southTint = Rgba8::CYAN;
	tints.skywardTint = Rgba8::BLUE;
	tints.hellwardTint = Rgba8::YELLOW;
	AddVertsForAABBZ3DToVector(vertices, cube, tints);
	m_cube->SetVertexArray(vertices);
	m_cube->SetAngularVelocity(Vec3(10.0f, 0.0f, 0.0f));

	vertices.clear();
	AddVertsForUVSphereZ3DToVector(vertices, Vec3(0.0f, 0.0f, 0.0f), 0.5f, 16.0f, 8.0f, Rgba8::WHITE);
	sphere->SetVertexArray(vertices);
	Texture* texture = g_theRenderer->CreateOrGetTexture("Data/Images/TestUV.png");
	sphere->SetTexture(texture);
	sphere->SetAngularVelocity(Vec3(4.0f, 8.0f, 0.0f));

	m_entities.push_back(m_player);
	m_entities.push_back(m_cube);
	m_entities.push_back(sphere);

	m_worldCamera.DefineGameSpace(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f));

	MouseConfig config = {};
	config.m_isHidden = true;
	config.m_isLocked = true;
	config.m_isRelative = true;
	config.m_priority = 0;
	g_inputSystem->PushMouseConfig(config);

	Mat44 identity;
	DebugAddWorldBasis(identity, -1.0f);
	Mat44 xAxisTextModel = Mat44(Vec3(0.0f, -1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.3f, 0.0f, 0.3f));
	Mat44 yAxisTextModel = Mat44::CreateTranslation3D(Vec3(0.0f, 0.3f, 0.3f));
	Mat44 zAxisTextModel = Mat44(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(-0.3f, 0.0f, 0.3f));
	DebugAddWorldText("X Axis", 0.25f, Vec2(0.0f, 0.0f), Rgba8::RED, xAxisTextModel, -1.0f, DebugRenderMode::USE_DEPTH);
	DebugAddWorldText("Y Axis", 0.25f, Vec2(0.0f, 0.0f), Rgba8::GREEN, yAxisTextModel, -1.0f, DebugRenderMode::USE_DEPTH);
	DebugAddWorldText("Z Axis", 0.25f, Vec2(0.0f, 0.0f), Rgba8::BLUE, zAxisTextModel, -1.0f, DebugRenderMode::USE_DEPTH);
}


void Game::DeInitPlayMode()
{
	for (int entityIndex = 0; entityIndex < (int) m_entities.size(); entityIndex++)
	{
		delete m_entities[entityIndex];
		m_entities[entityIndex] = nullptr;
	}

	m_entities.clear();
	g_inputSystem->PopMouseConfigOfPriority(0);
}


void Game::CheckInputPlayMode()
{
	XboxController const& controller = g_inputSystem->GetController(0);
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) || controller.WasButtonJustPressed(XboxController::XBOX_BUTTON_B))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}

	if (g_inputSystem->WasMouseButtonJustPressed(0))
	{
		Vec3 playerForward = m_player->GetEulerAngles().GetFoward_XFwd_YLeft_ZUp();;
		Vec3 playerPos = m_player->GetPosition();
		Vec3 spherePos = playerPos + playerForward * 2.0f;
		DebugAddWorldWireSphere(spherePos, 1.0f, -5.0f, Rgba8::GREEN, Rgba8::RED, DebugRenderMode::ALWAYS);
	}

	if (g_inputSystem->WasMouseButtonJustPressed(1))
	{
		Vec3 playerPos = m_player->GetPosition();
		float playerCylinderHeight = 3.0f;
		Vec3 playerCylinderBasePos = Vec3(playerPos.x, playerPos.y, playerPos.z - playerCylinderHeight * 0.5f);
		Vec3 playerCylinderTopPos = Vec3(playerPos.x, playerPos.y, playerPos.z + playerCylinderHeight * 0.5f);
		float playerCylinderRadius = 1.0f;
		std::string playerPosString = Stringf("(%.2f, %.2f, %.2f)", playerPos.x, playerPos.y, playerPos.z);
		DebugAddWorldBillboardText(playerPos, playerPosString, 0.5f, Vec2(0.5f, 0.5f), Rgba8::WHITE, Rgba8::RED, 10.0f, DebugRenderMode::USE_DEPTH);
		DebugAddWorldWireCylinder(playerCylinderBasePos, playerCylinderTopPos, playerCylinderRadius, 10.0f, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USE_DEPTH);
	}

	if (g_inputSystem->WasKeyJustPressed('L'))
	{
		Vec3 playerPos = m_player->GetPosition();
		DebugAddWorldLine(Vec3(0.0f, 0.0f, 0.0f), playerPos, Rgba8::BLUE, 0.125f, 4.0f, DebugRenderMode::XRAY);
	}

	if (g_inputSystem->WasKeyJustPressed('B'))
	{
		Mat44 camModelMat = m_player->GetModelMatrix();
		DebugAddWorldBasis(camModelMat, -1.0f);
	}

	if (g_inputSystem->WasKeyJustPressed('M'))
	{
		EulerAngles playerAngles = m_player->GetEulerAngles();
		std::string playerAnglesString = Stringf("Player Angles:: Yaw: %f, Pitch: %f, Roll: %f", playerAngles.m_yaw, playerAngles.m_pitch, playerAngles.m_roll);
		DebugAddScreenMessage(playerAnglesString, 5.0f, 16.0f, Rgba8::YELLOW);
	}
}


void Game::UpdatePlayMode(float deltaSeconds)
{
	static Stopwatch s_playerPosTimer = Stopwatch(*m_gameClock, 1.0);

	Rgba8 startColor = Rgba8::WHITE;
	Rgba8 endColor = Rgba8::BLUE;
	float time = (float) GetCurrentTimeSeconds();
	float lerpValue = SinRadians(time) * 0.5f + 0.5f;
	Rgba8 tint = Lerp(startColor, endColor, lerpValue);
	m_cube->SetTint(tint);

	UpdateEntities(deltaSeconds);

	Vec3 playerPos = m_player->GetPosition();
	DebugAddScreenMessage(Stringf("Player Pos: x = %f  y = %f  z = %f", playerPos.x, playerPos.y, playerPos.z), 0.0f, 20.0f);
	if (s_playerPosTimer.HasElapsed())
	{
		DebugAddWorldPoint(playerPos, 1.0f, Rgba8::WHITE, Rgba8::BLACK, 2.0f, DebugRenderMode::USE_DEPTH);
		s_playerPosTimer.Restart();
	}
}


void Game::RenderPlayMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	Texture* defaultDepthStencil = g_theRenderer->GetDefaultDepthStencil();
	g_theRenderer->ClearDepth(defaultDepthStencil, 1.0f, 0);

	g_theRenderer->BeginCamera(m_worldCamera);
	{
		g_theRenderer->SetDepthOptions(DepthTest::LESS, true);
		RenderGrid();
		RenderEntities();
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


void Game::RenderGrid() const
{
	float lineLength = 1000.0f;
	float otherDims = 0.05f;

	std::vector<Vertex_PCU> gridVerts;

	Vec3 camPosition = m_player->GetPosition();
	int camGridX = RoundDownToInt(camPosition.x);
	int camGridY = RoundDownToInt(camPosition.y);
	IntVec2 camGridPosition2D = IntVec2(camGridX, camGridY);

	for (int lineIndex = -100; lineIndex < 100; lineIndex++)
	{
		float xAxisGridPos = (float) (camGridX + lineIndex);
		float yAxisGridPos = (float) (camGridY + lineIndex);
		float xLineLength = abs(yAxisGridPos) + lineLength;
		float yLineLength = abs(xAxisGridPos) + lineLength;
		AABB3 lineAlongXAxis = AABB3(Vec3(xAxisGridPos, 0.0f, 0.0f), otherDims, xLineLength, otherDims);
		AABB3 lineAlongYAxis = AABB3(Vec3(0.0f, yAxisGridPos, 0.0f), yLineLength, otherDims, otherDims);

		if (xAxisGridPos == 0)
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongXAxis, Rgba8::WHITE);
		}
		else
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongXAxis, Rgba8::RED);
		}

		if (yAxisGridPos == 0)
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongYAxis, Rgba8::WHITE);
		}
		else
		{
			AddVertsForAABBZ3DToVector(gridVerts, lineAlongYAxis, Rgba8::GREEN);
		}
	}


	g_theRenderer->SetCullMode(CullMode::NONE);
	g_theRenderer->DrawVertexArray((int) gridVerts.size(), gridVerts.data());
}


void Game::RenderEntities() const
{
	for (int entityIndex = 0; entityIndex < (int) m_entities.size(); entityIndex++)
	{
		m_entities[entityIndex]->Render();
	}
}


void Game::UpdateEntities(float deltaSeconds)
{
	for (int entityIndex = 0; entityIndex < (int) m_entities.size(); entityIndex++)
	{
		m_entities[entityIndex]->Update(deltaSeconds);
	}
}


bool Game::Static_SetPlayerSpeed(EventArgs& args)
{
	Player* player = g_game->GetPlayer();
	float playerMoveSpeed		 = player->GetMoveSpeed();
	float playerRollSpeed		 = player->GetRollSpeed();
	float playerSprintSpeed		 = player->GetSprintSpeed();
	float playerRotationSpeed	 = player->GetRotationSpeed();

	float move	 = args.GetProperty("move", playerMoveSpeed);
	float roll	 = args.GetProperty("roll", playerRollSpeed);
	float sprint = args.GetProperty("sprint", playerSprintSpeed);
	float rotate = args.GetProperty("rotate", playerRotationSpeed);

	player->SetMovementSpeed(move);
	player->SetRollSpeed(roll);
	player->SetSprintSpeed(sprint);
	player->SetRotationSpeed(rotate);

	return true;
}
