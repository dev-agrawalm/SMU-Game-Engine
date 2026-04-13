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
#include "Game/Entity.hpp"
#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/DevConsole.hpp"

RandomNumberGenerator* g_rng	= nullptr;
BitmapFont* g_bitmapFont		= nullptr;
bool g_debugMode				= false;

Game::Game()
{
}


Game::~Game()
{
}


void Game::Startup()
{
	LoadAssets();
	g_rng = new RandomNumberGenerator();
	m_gameState = GameState::GAME_STATE_ATTRACT;
	InitAttractMode();

	g_console->AddLine(DevConsole::MINOR_INFO, "Game Started");
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
}


void Game::HandleQuitRequest()
{
	g_theApp->HandleQuitRequested();
}


void Game::LoadAssets()
{
	LoadAudio();
	LoadTextures();
}

void Game::LoadAudio()
{
	g_audioSystem->CreateOrGetSound("Data/Audio/AttractMusic.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/GameplayMusic.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/Click.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/GameOver.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/Victory.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/BulletBounce.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/BulletRicochet.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/BulletRicochet2.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
	g_audioSystem->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/ExitMap.wav");

}

void Game::LoadTextures()
{
	g_theRenderer->CreateOrGetTexture("Data/Images/PlayerTankBase.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/PlayerTankTop.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTurretBase.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/EnemyCannon.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank4.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank3.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank2.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/Terrain_8x8.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/FriendlyBullet.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/EnemyBullet.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/FriendlyTank3.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/FriendlyTank2.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/FriendlyTank4.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/EnemyShell.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank0.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/EnemyGatling.png");
	g_theRenderer->CreateOrGetTexture("Data/Images/Explosion_5x5.png");

	g_bitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
}

void Game::CheckInputDeveloperCheats()
{
	//enable debug renderer
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		g_debugMode = !g_debugMode;
	}

	if (g_inputSystem->IsKeyPressed('O'))
	{
		m_runSingleStep = true;
		m_isPaused = false;
	}

	//manipulate time scale
	if (g_inputSystem->IsKeyPressed('T'))
	{
		m_timeScale = 0.1f;
	}
	else if (g_inputSystem->IsKeyPressed('Y'))
	{
		m_timeScale = 4.0f;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F4))
	{
		m_isDebuggerCameraEnabled = !m_isDebuggerCameraEnabled;
	}

	if (g_inputSystem->WasKeyJustPressed('M'))
	{
		g_audioSystem->SetSoundPlaybackVolume(m_attractModeMusicId, 0.0f);
		g_audioSystem->SetSoundPlaybackVolume(m_gameplayModeMusicId, 0.0f);
	}
	
	if (g_inputSystem->WasKeyJustPressed('N'))
	{
		g_audioSystem->SetSoundPlaybackVolume(m_attractModeMusicId, 0.3f);
		g_audioSystem->SetSoundPlaybackVolume(m_gameplayModeMusicId, 0.1f);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}
}


void Game::Update(float deltaSeconds)
{
	m_timeScale = 1.0f;
	CheckInputDeveloperCheats();
	deltaSeconds *= m_timeScale;

	//check input
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:
			CheckInputAttractMode();
			break;
		case GameState::GAME_STATE_PLAYING:
			CheckInputPlayMode();
			break;
		case GameState::GAME_STATE_GAME_OVER:
			CheckInputGameOverMode();
			break;
		case GameState::GAME_STATE_GAME_WON:
			CheckInputGameWonMode();
			break;
		case GameState::NONE: //fallthrough case
		default:
			break;
	}

	//update game mode
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:
			UpdateAttractMode(deltaSeconds);
			break;
		case GameState::GAME_STATE_PLAYING:
			UpdatePlayMode(deltaSeconds);
			break; 
		case GameState::GAME_STATE_GAME_OVER:
			UpdateGameOverMode(deltaSeconds);
			break;
		case GameState::GAME_STATE_GAME_WON:
			UpdateGameWonMode(deltaSeconds);
			break;
		case GameState::NONE: //fallthrough case
		default:
			break;
	}

	UpdateCameras(deltaSeconds);

	if (m_runSingleStep)
	{
		m_runSingleStep = false;
		m_isPaused = true;
	}
}


void Game::UpdateCameras(float deltaSeconds)
{
	float windowAspectRatio		= g_window->GetMainWindowInstance()->GetWindowConfig().m_aspectRatio;
	float numTilesVertically	= g_gameConfigBlackboard.GetValue("numTilesVertically", 0.0f);
	float numTilesHorizontally	= numTilesVertically / windowAspectRatio;
	float tileWidth				= g_gameConfigBlackboard.GetValue("tileWidth", 1.0f);
	m_worldCamera.SetOrthoView(0.0f, 0.0f, numTilesVertically * tileWidth, numTilesHorizontally * tileWidth);

	float uiOrthoSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiOrthoSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, uiOrthoSizeX, uiOrthoSizeY);

	if (m_shouldShakeWorldCam)
	{
		m_worldCamTimeSinceScreenShakeStart += deltaSeconds;
		float shakeMagnitude	= RangeMapClamped(m_worldCamTimeSinceScreenShakeStart, 0.0f, m_worldCamScreenShakeDuration, m_worldCamScreenShakeMagnitude, 0.0f);
		float shakeX			= g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		float shakeY			= g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		m_worldCamera.TranslateCamera2D(Vec2(shakeX, shakeY));
		m_shouldShakeWorldCam = shakeMagnitude > 0.0f ? true : false;
	}

	if (m_shouldShakeUICam)
	{
		m_uiCamTimeSinceScreenShakeStart += deltaSeconds;
		float shakeMagnitude	= RangeMapClamped(m_uiCamTimeSinceScreenShakeStart, 0.0f, m_uiCamScreenShakeDuration, m_uiCamScreenShakeMagnitude, 0.0f);
		float shakeX			= g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		float shakeY			= g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		m_uiCamera.TranslateCamera2D(Vec2(shakeX, shakeY));
		m_shouldShakeUICam = shakeMagnitude > 0.0f ? true : false;
	}

	if (m_isDebuggerCameraEnabled)
	{
		if (m_world)
		{
			IntVec2 currentMapDims	= m_world->GetCurrentMapGridDimensions();
			float camTop			= currentMapDims.y * tileWidth;
			float camRight			= camTop * windowAspectRatio;
			m_worldCamera.SetOrthoView(0.0f, 0.0f, camRight, camTop);
		}
	}

	if (m_world)
	{
		Entity* player = m_world->GetPlayer();
		if (player)
		{
			SetWorldCameraPosition(player->GetPosition());
		}
	}
}


void Game::Render() const
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:
			RenderAttractMode();
			break;
		case GameState::GAME_STATE_PLAYING:
			RenderPlayMode();
			break;
		case GameState::GAME_STATE_GAME_OVER:
			RenderGameOverMode();
			break;
		case GameState::GAME_STATE_GAME_WON:
			RenderGameWonMode();
			break;
		case GameState::NONE: //fallthrough case
		default:
			break;
	}
}


void Game::InitAttractMode()
{
	SoundID musicId = g_audioSystem->CreateOrGetSound("Data/Audio/AttractMusic.mp3");
	m_attractModeMusicId = g_audioSystem->StartSound(musicId, true, 0.3f);
}


void Game::DeInitAttractMode()
{
	g_audioSystem->StopSound(m_attractModeMusicId);

	if (m_world)
	{
		m_world->DestroyWorld();
		delete m_world;
		m_world = nullptr;
	}
}


void Game::CheckInputAttractMode()
{
	XboxController playerController = g_inputSystem->GetController(0);

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE)
		|| playerController.WasButtonJustPressed(XboxController::XBOX_BUTTON_SELECT))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed('P')
		|| playerController.WasButtonJustPressed(XboxController::XBOX_BUTTON_START))
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
	Texture* tank1 = g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank2.png");
	Texture* tank2 = g_theRenderer->CreateOrGetTexture("Data/Images/FriendlyTank2.png");
	AABB2 spriteBoundingBox = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	std::vector<Vertex_PCU> tank1Vertexes;
	AddVertsForAABB2ToVector(tank1Vertexes, spriteBoundingBox,Rgba8::WHITE);
	TransformVertexArrayXY3D((int) tank1Vertexes.size(), tank1Vertexes.data(), 100.0f, 180.0f, Vec2(1200.0f, 400.0f));
	
	std::vector<Vertex_PCU> tank2Vertexes;
	AddVertsForAABB2ToVector(tank2Vertexes, spriteBoundingBox,Rgba8::WHITE);
	TransformVertexArrayXY3D( (int) tank2Vertexes.size(), tank2Vertexes.data(), 100.0f, 0.0f, Vec2(400.0f, 400.0f));

	Rgba8 teal = Rgba8(50, 168, 147, 255);
	g_theRenderer->ClearScreen(teal);

	g_theRenderer->BeginCamera(m_uiCamera);

	g_theRenderer->BindTexture(0, tank1);
	g_theRenderer->DrawVertexArray( (int) tank1Vertexes.size(), tank1Vertexes.data());
	
	g_theRenderer->BindTexture(0, tank2);
	g_theRenderer->DrawVertexArray((int) tank2Vertexes.size(), tank2Vertexes.data());
	
	//Testing the bitmapfont
	std::vector<Vertex_PCU> textVertexes;
	float uiOrthoSizeX		= g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiOrthoSizeY		= g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	float uiOrthoCenterX	= uiOrthoSizeX * 0.5f;
	float uiOrthoCenterY	= uiOrthoSizeY * 0.5f;
	AABB2 textBox = AABB2(Vec2(uiOrthoCenterX, uiOrthoCenterY), 100.0f, 50.0f);
	g_bitmapFont->AddVertsForTextInAABB2(textVertexes, textBox, 45.0f, "LIBRA");
	Texture const& fontTexture = g_bitmapFont->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) textVertexes.size(), textVertexes.data());

	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::InitPlayMode()
{
	m_isPaused = false;

	if (m_world == nullptr)
	{
		m_world = new World();
	}
	m_postPlayerDeathTimer = 0.0f;

	SoundID musicId	= g_audioSystem->CreateOrGetSound("Data/Audio/GameplayMusic.mp3");
	m_gameplayModeMusicId = g_audioSystem->StartSound(musicId, true, 0.1f);
}


void Game::DeInitPlayMode()
{
	g_audioSystem->StopSound(m_gameplayModeMusicId);
}


void Game::CheckInputPlayMode()
{
	XboxController playerController = g_inputSystem->GetController(0);

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE)
		|| playerController.WasButtonJustPressed(XboxController::XBOX_BUTTON_SELECT))
	{
		if (m_isPaused)
		{
			SetGameState(GameState::GAME_STATE_ATTRACT);
		}
		else
		{
			m_isPaused = true;
			SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/Click.mp3");
			g_audioSystem->StartSound(sfxId, false, 0.3f);
		}
	}

	//pause game
	if (g_inputSystem->WasKeyJustPressed('P')
		|| playerController.WasButtonJustPressed(XboxController::XBOX_BUTTON_START))
	{
		m_isPaused = !m_isPaused;
		SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/Click.mp3");
		g_audioSystem->StartSound(sfxId, false, 0.3f);
	}
}


void Game::UpdatePlayMode(float deltaSeconds)
{
	if (m_isPaused)
	{
		g_audioSystem->SetSoundPlaybackSpeed(m_gameplayModeMusicId, 0.0f);
		return;
	}
	else
	{
		g_audioSystem->SetSoundPlaybackSpeed(m_gameplayModeMusicId, 1.0f * m_timeScale);
	}

	if (m_world)
	{
		m_world->Update(deltaSeconds);

		Entity* player = m_world->GetPlayer();
		if (player && !player->IsAlive())
		{
			m_postPlayerDeathTimer += deltaSeconds;
			if (m_postPlayerDeathTimer >= m_playerDeathIdleTime)
			{
				SetGameState(GameState::GAME_STATE_GAME_OVER);
			}
		}
	}
}


void Game::RenderPlayMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	//World Camera
	{
		g_theRenderer->BeginCamera(m_worldCamera);
		if (m_world)
		{
			m_world->Render();
		}

		if (g_debugMode)
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
	SoundID musicId = g_audioSystem->CreateOrGetSound("Data/Audio/Victory.mp3");
	m_gameWonModeMusicId = g_audioSystem->StartSound(musicId, false, 0.3f);
}


void Game::UpdateGameWonMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Game::CheckInputGameWonMode()
{
	XboxController playerController = g_inputSystem->GetController(0);

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE)
		|| playerController.WasButtonJustPressed(XboxController::XBOX_BUTTON_SELECT)
		|| playerController.WasButtonJustPressed(XboxController::XBOX_BUTTON_START)
		|| g_inputSystem->WasKeyJustPressed('P'))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}
}


void Game::RenderGameWonMode() const
{
	Texture* tank1 = g_theRenderer->CreateOrGetTexture("Data/Images/FriendlyTank3.png");
	Texture* tank2 = g_theRenderer->CreateOrGetTexture("Data/Images/FriendlyTank2.png");
	Texture* tank3 = g_theRenderer->CreateOrGetTexture("Data/Images/FriendlyTank4.png");
	AABB2 spriteBoundingBox = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	std::vector<Vertex_PCU> tank1Vertexes;
	AddVertsForAABB2ToVector(tank1Vertexes, spriteBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) tank1Vertexes.size(), tank1Vertexes.data(), 100.0f, 60.0f, Vec2(400.0f, 200.0f));

	std::vector<Vertex_PCU> tank2Vertexes;
	AddVertsForAABB2ToVector(tank2Vertexes, spriteBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) tank2Vertexes.size(), tank2Vertexes.data(), 100.0f, 270.0f, Vec2(800.0f, 600.0f));

	std::vector<Vertex_PCU> tank3Vertexes;
	AddVertsForAABB2ToVector(tank3Vertexes, spriteBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) tank3Vertexes.size(), tank3Vertexes.data(), 100.0f, 120.0f, Vec2(1200.0f, 200.0f));

	Rgba8 teal = Rgba8(50, 168, 147, 255);
	g_theRenderer->ClearScreen(teal);

	g_theRenderer->BeginCamera(m_uiCamera);
	g_theRenderer->BindTexture(0, tank1);
	g_theRenderer->DrawVertexArray((int) tank1Vertexes.size(), tank1Vertexes.data());

	g_theRenderer->BindTexture(0, tank2);
	g_theRenderer->DrawVertexArray((int) tank2Vertexes.size(), tank2Vertexes.data());

	g_theRenderer->BindTexture(0, tank3);
	g_theRenderer->DrawVertexArray((int) tank3Vertexes.size(), tank3Vertexes.data());

	std::vector<Vertex_PCU> textVertexes;
	float uiOrthoSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiOrthoSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	float uiOrthoCenterX = uiOrthoSizeX * 0.5f;
	float uiOrthoCenterY = uiOrthoSizeY * 0.5f;
	AABB2 textBox = AABB2(Vec2(uiOrthoCenterX, uiOrthoCenterY), 100.0f, 50.0f);
	g_bitmapFont->AddVertsForTextInAABB2(textVertexes, textBox, 45.0f, "YOU WON");
	Texture const& fontTexture = g_bitmapFont->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) textVertexes.size(), textVertexes.data());

	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::DeInitGameWonMode()
{
	g_audioSystem->StopSound(m_gameWonModeMusicId);
}


void Game::InitGameOverMode()
{
	SoundID musicId = g_audioSystem->CreateOrGetSound("Data/Audio/GameOver.mp3");
	m_gameOverModeMusicId = g_audioSystem->StartSound(musicId, false, 0.3f);
}


void Game::UpdateGameOverMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Game::CheckInputGameOverMode()
{
	XboxController playerController = g_inputSystem->GetController(0);

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE)
		|| playerController.WasButtonJustPressed(XboxController::XBOX_BUTTON_SELECT))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}

	if (playerController.WasButtonJustPressed(XboxController::XBOX_BUTTON_START)
		|| g_inputSystem->WasKeyJustPressed('P'))
	{
		if (m_world)
		{
			m_world->RespawnPlayerAtLastLocation();
			SetGameState(GameState::GAME_STATE_PLAYING);
		}
	}
}


void Game::RenderGameOverMode() const
{
	Texture* tank1 = g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank3.png");
	Texture* tank2 = g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank2.png");
	Texture* tank3 = g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank4.png");
	AABB2 spriteBoundingBox = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	std::vector<Vertex_PCU> tank1Vertexes;
	AddVertsForAABB2ToVector(tank1Vertexes, spriteBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) tank1Vertexes.size(), tank1Vertexes.data(), 100.0f, 60.0f, Vec2(400.0f, 200.0f));

	std::vector<Vertex_PCU> tank2Vertexes;
	AddVertsForAABB2ToVector(tank2Vertexes, spriteBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) tank2Vertexes.size(), tank2Vertexes.data(), 100.0f, 270.0f, Vec2(800.0f, 600.0f));

	std::vector<Vertex_PCU> tank3Vertexes;
	AddVertsForAABB2ToVector(tank3Vertexes, spriteBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) tank3Vertexes.size(), tank3Vertexes.data(), 100.0f, 120.0f, Vec2(1200.0f, 200.0f));

	Rgba8 teal = Rgba8(50, 168, 147, 255);
	g_theRenderer->ClearScreen(teal);

	g_theRenderer->BeginCamera(m_uiCamera);
	g_theRenderer->BindTexture(0, tank1);
	g_theRenderer->DrawVertexArray((int) tank1Vertexes.size(), tank1Vertexes.data());

	g_theRenderer->BindTexture(0, tank2);
	g_theRenderer->DrawVertexArray((int) tank2Vertexes.size(), tank2Vertexes.data());

	g_theRenderer->BindTexture(0, tank3);
	g_theRenderer->DrawVertexArray((int) tank3Vertexes.size(), tank3Vertexes.data());

	std::vector<Vertex_PCU> textVertexes;
	float uiOrthoSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	float uiOrthoSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	float uiOrthoCenterX = uiOrthoSizeX * 0.5f;
	float uiOrthoCenterY = uiOrthoSizeY * 0.5f;
	AABB2 textBox = AABB2(Vec2(uiOrthoCenterX, uiOrthoCenterY), 100.0f, 50.0f);
	g_bitmapFont->AddVertsForTextInAABB2(textVertexes, textBox, 45.0f, "YOU LOST");
	Texture const& fontTexture = g_bitmapFont->GetTexture();
	g_theRenderer->BindTexture(0, &fontTexture);
	g_theRenderer->DrawVertexArray((int) textVertexes.size(), textVertexes.data());
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::DeInitGameOverMode()
{
	g_audioSystem->StopSound(m_gameOverModeMusicId);
}


void Game::RenderPlayModeUI() const
{
	if (m_isPaused)
	{
		AABB2 camBox = m_uiCamera.GetOrthoCamBoundingBox();
		Rgba8 translucentBlack = Rgba8(0, 0, 0, 155);
		std::vector<Vertex_PCU> vertexes;
		AddVertsForAABB2ToVector(vertexes, camBox, translucentBlack);
		g_theRenderer->SetDiffuseTexture(nullptr);
		g_theRenderer->DrawVertexArray((int) vertexes.size(),vertexes.data());
	}
}


void Game::SetGameState(GameState gameMode)
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
		case GameState::NONE: //fall through
		default:
			break;
	}

	m_gameState = gameMode;

	switch (gameMode)
	{
		case GameState::GAME_STATE_ATTRACT:
			InitAttractMode();
			break;
		case GameState::GAME_STATE_PLAYING:
			InitPlayMode();
			break;
		case GameState::GAME_STATE_GAME_WON:
			InitGameWonMode();
			break;
		case GameState::GAME_STATE_GAME_OVER:
			InitGameOverMode();
			break;
		case GameState::NONE: //fall through
		default:
			break;
	}
}


void Game::SetWorldCamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration)
{
	m_worldCamScreenShakeMagnitude = screenShakeMagnitude;
	m_worldCamScreenShakeDuration = screenShakeDuration;
	m_shouldShakeWorldCam = true;
	m_worldCamTimeSinceScreenShakeStart = 0.0f;
}


void Game::SetUICamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration)
{
	m_uiCamScreenShakeMagnitude = screenShakeMagnitude;
	m_uiCamScreenShakeDuration = screenShakeDuration;
	m_shouldShakeUICam = true;
	m_uiCamTimeSinceScreenShakeStart = 0.0f;
}


void Game::SetWorldCameraPosition(Vec2 const& position)
{
	float tileWidth			= g_gameConfigBlackboard.GetValue("tileWidth", 1.0f);
	AABB2 camBox			= m_worldCamera.GetOrthoCamBoundingBox();
	Vec2 camBoxHalfDims		= camBox.GetDimensions() * 0.5f;
	IntVec2 currentMapDims	= m_world->GetCurrentMapGridDimensions();

	float minX	= 0.0f + camBoxHalfDims.x;
	float maxX	= (currentMapDims.x * tileWidth) - camBoxHalfDims.x;
	float x		= Clamp(position.x, minX, maxX);

	float minY	= 0.0f + camBoxHalfDims.y;
	float maxY	= (currentMapDims.y * tileWidth) - camBoxHalfDims.y;
	float y		= Clamp(position.y, minY, maxY);
	
	Vec2 worldCamPos = Vec2(x, y);
	m_worldCamera.SetPosition(worldCamPos);
}


void Game::SetTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}


bool Game::AreEntitiesColliding(Entity* entityA, Entity* entityB) const
{
	if (!entityA->IsAlive() || !entityB->IsAlive())
		return false;

	Vec2 entityPosA = entityA->GetPosition();
	Vec2 entityPosB = entityB->GetPosition();
	float entityRadiusA = entityA->GetPhysicsRadius();
	float entityRadiusB = entityB->GetPhysicsRadius();

	return DoDiscsOverlap2D(entityPosA, entityRadiusA, entityPosB, entityRadiusB);
}
