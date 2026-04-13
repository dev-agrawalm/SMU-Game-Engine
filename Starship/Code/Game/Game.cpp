#include "Game/Game.hpp"
#include "Game/SpaceShip.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/Bullet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Debris.hpp"
#include "Game/GameCommon.hpp"
#include "Game/EnemyWave.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Bomb.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

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
	//load audio
	g_audioSystem->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/PlayerDied.wav");
 	g_audioSystem->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
 	g_audioSystem->CreateOrGetSound("Data/Audio/PlayerHit.wav");
 	g_audioSystem->CreateOrGetSound("Data/Audio/Explosion.wav");
 	g_audioSystem->CreateOrGetSound("Data/Audio/Bounce.wav");
 	g_audioSystem->CreateOrGetSound("Data/Audio/Respawn.wav");
 	g_audioSystem->CreateOrGetSound("Data/Audio/YouLose.mp3");
 	g_audioSystem->CreateOrGetSound("Data/Audio/YouWin.mp3");
 	g_audioSystem->CreateOrGetSound("Data/Audio/Attract Mode Music.wav");
	g_audioSystem->CreateOrGetSound(WAVE_ONE_BG_MUSIC);
	g_audioSystem->CreateOrGetSound(WAVE_TWO_BG_MUSIC);
	g_audioSystem->CreateOrGetSound(WAVE_THREE_BG_MUSIC);
	g_audioSystem->CreateOrGetSound(WAVE_FOUR_BG_MUSIC);
	g_audioSystem->CreateOrGetSound(WAVE_FIVE_BG_MUSIC);


	g_rng = new RandomNumberGenerator();
	m_gameMode = GameMode::ATTRACT;
	m_playerController = &(g_inputSystem->GetController(0));

	InitAttractMode();
}

void Game::Update(float deltaSeconds)
{
	switch (m_gameMode)
	{
		case GameMode::ATTRACT:
			if (!g_theApp->IsPaused())
			{
				UpdateAttractMode(deltaSeconds);
			}
			CheckInputAttractMode();
			break;
		case GameMode::PLAY:
			if (!g_theApp->IsPaused())
			{
				UpdatePlayMode(deltaSeconds);
			}
			CheckInputPlayMode();
			break;
		case GameMode::NONE:
		default:
			break;
	}

	m_worldCamera.SetOrthoView(0.0f, 0.0f, WORLD_SIZE_X, WORLD_SIZE_Y);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, UI_CANVAS_SIZE_X, UI_CANVAS_SIZE_Y);
	

	if (m_shouldShakeWorldCam)
	{
		m_worldCamTimeSinceScreenShakeStart += deltaSeconds;
		float shakeMagnitude = RangeMapClamped(m_worldCamTimeSinceScreenShakeStart, 0.0f, m_worldCamScreenShakeDuration, m_worldCamScreenShakeMagnitude, 0.0f);
		float shakeX = g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		float shakeY = g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		m_worldCamera.TranslateCamera2D(Vec2(shakeX, shakeY));
		m_shouldShakeWorldCam = shakeMagnitude > 0.0f ? true : false;
	}

	if (m_shouldShakeUICam)
	{
		m_uiCamTimeSinceScreenShakeStart += deltaSeconds;
		float shakeMagnitude = RangeMapClamped(m_uiCamTimeSinceScreenShakeStart, 0.0f, m_uiCamScreenShakeDuration, m_uiCamScreenShakeMagnitude, 0.0f);
		float shakeX = g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		float shakeY = g_rng->GetRandomFloatInRange(-shakeMagnitude, shakeMagnitude);
		m_uiCamera.TranslateCamera2D(Vec2(shakeX, shakeY));
		m_shouldShakeUICam = shakeMagnitude > 0.0f ? true : false;
	}
	
	m_uiCamera.SetProjectionMatrix(m_uiCamera.GetOrthoProjectionMatrix());
	m_worldCamera.SetProjectionMatrix(m_worldCamera.GetOrthoProjectionMatrix());
}

void Game::Render() const
{
	g_theRenderer->BindTexture(0, nullptr);
	switch (m_gameMode)
	{
		case GameMode::ATTRACT:
			RenderAttractMode();
			break;
		case GameMode::PLAY:
			RenderPlayMode();
			break;
		case GameMode::NONE:
		default:
			break;
	}
}

void Game::InitAttractMode()
{
	SoundID attractModeMusicId = g_audioSystem->CreateOrGetSound("Data/Audio/CelesteMusic2.wav");
	m_attractModePlaybackId = g_audioSystem->StartSound(attractModeMusicId, true, 1.0f, 0.0f, 1.0f);

	SpawnStars(2000);
}

void Game::UpdateAttractMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i])
		{
			m_debris[i]->Update(deltaSeconds);
		}
	}

	SpawnStars(1);
	//SetUICamScreenShakeParameters(ATTRACT_MODE_SCREEN_SHAKE_MAGNITUDE, 1.0f);
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
		SetGameMode(GameMode::PLAY);
	}

	
	if (m_playerController->IsConnected())
	{
		if (m_playerController->WasButtonJustPressed(XboxController::XBOX_BUTTON_LEFT_BUMPER))
		{
			HandleQuitRequest();
		}

		if (m_playerController->WasButtonJustPressed(XboxController::XBOX_BUTTON_START))
		{
			SetGameMode(GameMode::PLAY);
		}
	}
}

void Game::RenderAttractMode() const
{

	//Render the stars
	{
// 		Vec2 starPosition;
// 		float starRotation;
// 		Vertex_PCU starVertexes[ATTRACT_MODE_NUM_STARS * NUM_STAR_TRIANGLES * 3];
// 		for (int starIndex = 0; starIndex < ATTRACT_MODE_NUM_STARS; starIndex++)
// 		{
// 			Vertex_PCU tempStarVerts[NUM_STAR_TRIANGLES * 3];
// 			starPosition = GetRandomUICanvasLocation();
// 			starRotation = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
// 			//left triangle
// 			tempStarVerts[0].m_position = Vec3(0.0f, 0.1f);
// 			tempStarVerts[1].m_position = Vec3(-20.0f, 0.0f);
// 			tempStarVerts[2].m_position = Vec3(0.0f, -0.1f);
// 			//down triangle
// 			tempStarVerts[3].m_position = Vec3(-0.1f, 0.0f);
// 			tempStarVerts[4].m_position = Vec3(0.0f, -20.0f);
// 			tempStarVerts[5].m_position = Vec3(0.1f, 0.0f);
// 			//right triangle
// 			tempStarVerts[6].m_position = Vec3(0.0f, -0.1f);
// 			tempStarVerts[7].m_position = Vec3(20.0f, 0.0f);
// 			tempStarVerts[8].m_position = Vec3(0.0f, 0.1f);
// 			//up triangle
// 			tempStarVerts[9].m_position = Vec3(0.1f, 0.0f);
// 			tempStarVerts[10].m_position = Vec3(0.0f, 20.0f);
// 			tempStarVerts[11].m_position = Vec3(-0.1f, 0.0f);
// 
// 			TransformVertexArrayXY3D(NUM_STAR_TRIANGLES * 3, tempStarVerts, 1.0f, starRotation, starPosition);
// 			for (int vertIndex = 0; vertIndex < NUM_STAR_TRIANGLES * 3; vertIndex++)
// 			{
// 				starVertexes[starIndex * NUM_STAR_TRIANGLES * 3 + vertIndex] = tempStarVerts[vertIndex];
// 				starVertexes[starIndex * NUM_STAR_TRIANGLES * 3 + vertIndex].m_color = Rgba8::WHITE;
// 			}
// 		}
// 
// 		g_theRenderer->DrawVertexArray(ATTRACT_MODE_NUM_STARS * NUM_STAR_TRIANGLES * 3,
//									   starVertexes);
	}

	g_theRenderer->BeginCamera(m_worldCamera);
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i])
		{
			m_debris[i]->Render();
		}
	}
	g_theRenderer->EndCamera(m_worldCamera);

	g_theRenderer->BeginCamera(m_uiCamera);
	//Render the play button
	{
		Vertex_PCU playButtonVertexes[3];
		Rgba8 DARK_GREEN = Rgba8(0, 150, 0, 255);
		Vec3 uiCanvasCenterPosition = Vec3(UI_CANVAS_CENTER_X, UI_CANVAS_CENTER_Y);

		playButtonVertexes[0].m_position = Vec3(60.0f, 0.0f) + uiCanvasCenterPosition;
		playButtonVertexes[1].m_position = Vec3(-60.0f, 60.0f) + uiCanvasCenterPosition;
		playButtonVertexes[2].m_position = Vec3(-60.0f, -60.0f) + uiCanvasCenterPosition;
		playButtonVertexes[0].m_color = DARK_GREEN;
		playButtonVertexes[1].m_color = DARK_GREEN;
		playButtonVertexes[2].m_color = DARK_GREEN;
		g_theRenderer->DrawVertexArray(3, playButtonVertexes);
	}

	g_theRenderer->EndCamera(m_uiCamera);
}

void Game::DeInitAttractMode()
{
	g_audioSystem->StopSound(m_attractModePlaybackId);
}

void Game::InitPlayMode()
{
	m_gameplayState = GameplayState::PLAYING;
	m_isGameOver = false;
	g_theApp->SetPause(false);
	Vec2 shipStartingPosition = Vec2(PLAYER_SHIP_STARTING_X, PLAYER_SHIP_STARTING_Y);
	m_spaceShip = new SpaceShip(this, shipStartingPosition, m_playerController);
	m_timeSinceGameOver = -1.0f;

	m_enemyWaves[0] = new EnemyWave(WAVE_ONE_ENEMY_COUNT[0], WAVE_ONE_ENEMY_COUNT[1], WAVE_ONE_ENEMY_COUNT[2]);
	m_enemyWaves[1] = new EnemyWave(WAVE_TWO_ENEMY_COUNT[0], WAVE_TWO_ENEMY_COUNT[1], WAVE_TWO_ENEMY_COUNT[2]);
	m_enemyWaves[2] = new EnemyWave(WAVE_THREE_ENEMY_COUNT[0], WAVE_THREE_ENEMY_COUNT[1], WAVE_THREE_ENEMY_COUNT[2]);
	m_enemyWaves[3] = new EnemyWave(WAVE_FOUR_ENEMY_COUNT[0], WAVE_FOUR_ENEMY_COUNT[1], WAVE_FOUR_ENEMY_COUNT[2]);
	m_enemyWaves[4] = new EnemyWave(WAVE_FIVE_ENEMY_COUNT[0], WAVE_FIVE_ENEMY_COUNT[1], WAVE_FIVE_ENEMY_COUNT[2]);
	m_currentWaveIndex = 0;
	m_enemyWaves[m_currentWaveIndex]->Init(m_spaceShip, this, ENEMY_WAVE_BG_MUSIC[m_currentWaveIndex]);

	//SpawnStars(2000);
}

void Game::RenderPlayModeUI() const
{
	const float uiLifeIconScale = 6.0f;
	const Vec2 uiLifeIconPositions[4] = {
										Vec2(20.0f, 775.0f),
										Vec2(55.0f, 775.0f),
										Vec2(90.0f, 775.0f),
										Vec2(125.0f, 775.0f)
	};

	Vertex_PCU* spaceshipVertexes = m_spaceShip->GetLocalVertexes();
	Vertex_PCU tempVerts[NUM_SHIP_VERTS];

	int numLives = m_spaceShip->GetLives();
	for (int lifeIndex = 0; lifeIndex < numLives; lifeIndex++)
	{
		for (int vertexIndex = 0; vertexIndex < NUM_SHIP_VERTS; vertexIndex++)
		{
			tempVerts[vertexIndex] = spaceshipVertexes[vertexIndex];
			tempVerts[vertexIndex].m_color.a = 200;
		}

		TransformVertexArrayXY3D(NUM_SHIP_VERTS, tempVerts, uiLifeIconScale, 0.0f, uiLifeIconPositions[lifeIndex]);
		g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, tempVerts);
	}

	std::vector<Vertex_PCU> textVerts;
	static	BitmapFont* bitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/SquirrelFixedFont");
	static Vec2 gameResultTextMins = Vec2(UI_CANVAS_CENTER_X * 0.75f, UI_CANVAS_CENTER_Y);
	if (m_gameplayState == GameplayState::GAME_LOST)
	{
		bitmapFont->AddVertsForText2D(textVerts, gameResultTextMins, 40.0f, "Game Over", Rgba8::YELLOW);
	}
	else if (m_gameplayState == GameplayState::GAME_WON)
	{
		bitmapFont->AddVertsForText2D(textVerts, gameResultTextMins, 40.0f, "You Won!", Rgba8::YELLOW);
	}
	g_theRenderer->BindTexture(0, &bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray((int) textVerts.size(), textVerts.data());
}

void Game::UpdatePlayMode(float deltaSeconds)
{
	if (m_spaceShip->IsAlive() && m_spaceShip->IsSlowingDown())
	{
		deltaSeconds *= 0.25f;
	}

	if (m_isGameOver)
	{
		m_timeSinceGameOver += deltaSeconds;
		if (m_timeSinceGameOver >= GAME_OVER_IDLE_TIME)
		{
			SetGameMode(GameMode::ATTRACT);
			return;
		}
	}

	//update bomb
// 	for (int bombIndex = 0; bombIndex < PLAYER_SHIP_NUM_BOMBS; bombIndex++)
// 	{
// 		if (m_spaceshipBombs[bombIndex])
// 		{
// 			m_spaceshipBombs[bombIndex]->Update(deltaSeconds);
// 		}
// 	}

	//update spaceShip
	if (m_spaceShip)
	{
		m_spaceShip->Update(deltaSeconds);
	}

	//update bullets
	for (int index = 0; index < m_bulletCount; index++)
	{
		if (m_bullets[index])
		{
			m_bullets[index]->Update(deltaSeconds);
		}
	}

	for (int index = 0; index < m_debrisCount; index++)
	{
		if (m_debris[index])
		{
			m_debris[index]->Update(deltaSeconds);
		}
	}
// 
// 	//update enemies
	if (m_enemyWaves[m_currentWaveIndex] && m_currentWaveIndex < NUM_WAVES)
	{
		m_enemyWaves[m_currentWaveIndex]->Update(deltaSeconds);
	}

	SpawnStars(1);


	CheckEntityCollisions();
	DeleteGarbage();

	CheckForGameOver();
}


void Game::SpawnStars(int numStars)
{
	Vec2 randMins = Vec2(-5.0f, -5.0f);
	Vec2 randMaxs = Vec2(WORLD_SIZE_X + 5.0f, /*WORLD_SIZE_Y + 5.0f*/-1.0f);
	Vec2 pos = g_rng->GetRandomPositionInAABB2(randMins, randMaxs);
	float orientation = 90.0f; // g_rng->GetRandomFloatInRange(0.0f, 360.0f);
	float orientationDelta = g_rng->GetRandomFloatInRange(-10.0f, 10.0f);
	orientation += orientationDelta;
	float speed = g_rng->GetRandomFloatInRange(5.0f, 20.0f);
	float fadeoutTime = g_rng->GetRandomFloatInRange(5.0f, 30.0f);
	SpawnParticles(pos, numStars, g_rng->GetRandomColor(), Vec2::MakeFromPolarDegrees(orientation), speed, fadeoutTime, 0.1f, 0.5f);
}


void Game::CheckForGameOver()
{
	if (m_isGameOver)
		return;

	if (!m_spaceShip->IsAlive() && m_spaceShip->GetLives() == 0)
	{
		m_gameplayState = GameplayState::GAME_LOST;
		m_isGameOver = true;
	}

	if (m_enemyWaves[m_currentWaveIndex]->IsWaveOver())
	{
		m_currentWaveIndex++;
		if (m_currentWaveIndex >= NUM_WAVES)
		{
			m_gameplayState = GameplayState::GAME_WON;
			m_isGameOver = true;
		}
		else
		{
			m_enemyWaves[m_currentWaveIndex - 1]->Deinit();
			m_enemyWaves[m_currentWaveIndex]->Init(m_spaceShip, this, ENEMY_WAVE_BG_MUSIC[m_currentWaveIndex]);
		}
	}

	if (m_isGameOver)
		PlayGameOverMusic();

}


void Game::PlayGameOverMusic()
{
	SoundID gameOverMusicId;
	switch (m_gameplayState)
	{
		case GameplayState::GAME_LOST:
		{
			SoundPlaybackID enemyWaveBgPlaybackId = m_enemyWaves[NUM_WAVES - 1]->GetBgMusicPlaybackId();
			FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) enemyWaveBgPlaybackId;
			channelAssignedToSound->setVolume(0.1f);
			
			gameOverMusicId = g_audioSystem->CreateOrGetSound("Data/Audio/YouLose.mp3");
			g_audioSystem->StartSound(gameOverMusicId);
			break;
		}
		case GameplayState::GAME_WON:
		{
			SoundPlaybackID enemyWaveBgPlaybackId = m_enemyWaves[NUM_WAVES - 1]->GetBgMusicPlaybackId();
			FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) enemyWaveBgPlaybackId;
			channelAssignedToSound->setVolume(0.1f);

			gameOverMusicId = g_audioSystem->CreateOrGetSound("Data/Audio/YouWin.mp3");
			g_audioSystem->StartSound(gameOverMusicId);
			break;
		}
		case GameplayState::PLAYING: //fall through case
		default:
			break;
	}
}

void Game::CheckInputPlayMode()
{
	if (m_gameplayState != GameplayState::PLAYING)
		return;

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		SetGameMode(GameMode::ATTRACT);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_debugRender = !m_debugRender;
	}

	if (m_spaceShip && !m_spaceShip->IsAlive() && g_inputSystem->WasKeyJustPressed('N'))
	{
		if (m_spaceShip->GetLives() > 0)
		{
			RespawnShip();
		}
	}

	if (m_playerController->IsConnected())
	{
		if (m_playerController->WasButtonJustPressed(XboxController::XBOX_BUTTON_Y))
		{
			bool isAppPaused = g_theApp->IsPaused();
			g_theApp->SetPause(!isAppPaused);
		}

		if (m_playerController->WasButtonJustPressed(XboxController::XBOX_BUTTON_SELECT))
		{
			SetGameMode(GameMode::ATTRACT);
		}

		if (m_playerController->WasButtonJustPressed(XboxController::XBOX_BUTTON_Y))
		{
			m_debugRender = !m_debugRender;
		}

		if (m_playerController->WasButtonJustPressed(XboxController::XBOX_BUTTON_START))
		{
			RespawnShip();
		}
	}
}

void Game::RespawnShip()
{
	if (m_spaceShip->IsAlive())
		return;

	Vec2 shipStartingPosition = Vec2(PLAYER_SHIP_STARTING_X, PLAYER_SHIP_STARTING_Y);
	m_spaceShip->Respawn(shipStartingPosition);
}

void Game::RenderPlayMode() const
{
	g_theRenderer->BeginCamera(m_worldCamera);
	
// 	for (int bombIndex = 0; bombIndex < PLAYER_SHIP_NUM_BOMBS; bombIndex++)
// 	{
// 		if (m_spaceshipBombs[bombIndex])
// 		{
// 			m_spaceshipBombs[bombIndex]->Render();
// 		}
// 	}

	//Render bullets
	RenderBullets();
	//Render Asteroids
	RenderDebris();
	//Render Spaceship
	if (m_spaceShip)
	{
		m_spaceShip->Render();
	}
	//Render enemy waves
	if (m_enemyWaves[m_currentWaveIndex] && m_currentWaveIndex < NUM_WAVES)
	{
		m_enemyWaves[m_currentWaveIndex]->Render();
	}
	//start debug rendering
	if (m_debugRender)
	{
		DebugRenderEntityDetails();
	}
	g_theRenderer->EndCamera(m_worldCamera);

	g_theRenderer->BeginCamera(m_uiCamera);
	RenderPlayModeUI();
	g_theRenderer->EndCamera(m_uiCamera);
}

void Game::DeInitPlayMode()
{
	m_gameplayState = GameplayState::NONE;
	//delete spaceship
	delete m_spaceShip;
	m_spaceShip = nullptr;

	//delete any remaining bullets
	for (int index = 0; index < m_bulletCount; index++)
	{
		delete m_bullets[index];
		m_bullets[index] = nullptr;
	}

	for (int index = 0; index < m_debrisCount; index++)
	{
		delete m_debris[index];
		m_debris[index] = nullptr;
	}
// 
// 	for (int bombIndex = 0; bombIndex < PLAYER_SHIP_NUM_BOMBS; bombIndex++)
// 	{
// 		delete m_spaceshipBombs[bombIndex];
// 		m_spaceshipBombs[bombIndex] = nullptr;
// 	}
// 
	m_currentWaveIndex = Clamp(m_currentWaveIndex, 0, NUM_WAVES - 1);
	m_enemyWaves[m_currentWaveIndex]->Deinit();
	for (int index = 0; index < NUM_WAVES; index++)
	{
		EnemyWave*& enemyWave = m_enemyWaves[index];
		if (enemyWave)
		{
			delete enemyWave;
			enemyWave = nullptr;
		}
	}
}

void Game::RenderDebris() const
{
	for (int index = 0; index < m_debrisCount; index++)
	{
		if (m_debris[index])
		{
			m_debris[index]->Render();
		}
	}
}

void Game::RenderBullets() const
{
	for (int index = 0; index < m_bulletCount; index++)
	{
		if (m_bullets[index])
		{
			m_bullets[index]->Render();
		}
	}
}

void Game::DebugRenderEntityDetails() const
{
	for (int index = 0; index < m_debrisCount; index++)
	{
		if (m_debris[index])
		{
			m_debris[index]->DebugRender();
		}
	}

	//Render bullet information
	for (int index = 0; index < m_bulletCount; index++)
	{
		if (m_bullets[index])
		{
			m_bullets[index]->DebugRender();
		}
	}

	m_enemyWaves[m_currentWaveIndex]->DebugRenderEnemyDetails();

	//render spaceship information
	if (m_spaceShip)
	{
		m_spaceShip->DebugRender();
		DebugRenderEntitySpaceshipDistance();
	}
}

void Game::DebugRenderEntitySpaceshipDistance() const
{
	if (!m_spaceShip->IsAlive())
		return;

	const Rgba8 DARK_GREY(50, 50, 50, 255);
	Vec2 spaceshipPos = m_spaceShip->GetPosition();

	//draw lines for bullets
	for (int index = 0; index < m_bulletCount; index++)
	{
		Bullet* bullet = m_bullets[index];

		if (bullet && bullet->IsAlive())
		{
			Vec2 bulletPos = bullet->GetPosition();
			Vec2 distanceVec = spaceshipPos - bulletPos;

			DebugDrawLine(bulletPos, distanceVec, DEBUG_LINE_WIDTH, DARK_GREY);
		}
	}

	m_enemyWaves[m_currentWaveIndex]->DebugRenderEnemySpaceshipDistance();
}

void Game::ShutDown()
{
	switch (m_gameMode)
	{
		case GameMode::ATTRACT:
			DeInitAttractMode();
			break;
		case GameMode::PLAY:
			DeInitPlayMode();
			break;
		case GameMode::NONE:
		default:
			break;
	}

	m_playerController = nullptr;
	
	delete g_rng;
	g_rng = nullptr;
}

void Game::HandleQuitRequest()
{
	g_theApp->HandleQuitRequested();
}

void Game::SetGameMode(GameMode gameMode)
{
	m_gameMode = gameMode;

	switch (gameMode)
	{
		case GameMode::ATTRACT:
			DeInitPlayMode();
			InitAttractMode();
			break;
		case GameMode::PLAY:
			DeInitAttractMode();
			InitPlayMode();
			break;
		case GameMode::NONE:
		default:
			break;
	}
}

void Game::FireBullet(Vec2 const& firePosition, float orientationDegree)
{
	if (m_bulletCount >= MAX_BULLETS)
	{
		ERROR_RECOVERABLE("Reached Max Limit for bullets on screen");
	}
	else
	{
		m_bullets[m_bulletCount] = new Bullet(this, firePosition, orientationDegree);
		m_bulletCount++;
	}
}

void Game::SpawnBomb(Vec2 const& bombPosition)
{
	UNUSED(bombPosition);
	for (int bombIndex = 0; bombIndex < PLAYER_SHIP_NUM_BOMBS; bombIndex++)
	{
		if (bombIndex >= PLAYER_SHIP_NUM_BOMBS)
			break;

// 		if (m_spaceshipBombs[bombIndex] == nullptr)
// 		{
// 			m_spaceshipBombs[bombIndex] = new Bomb(this, bombPosition);
// 			break;;
// 		}
	}
}

void Game::SpawnDebris(Entity* forEntity, int numberOfPieces, Rgba8 color)
{
	int totalDebrisCount = m_debrisCount + numberOfPieces;

	if (totalDebrisCount >= MAX_DEBRIS)
	{
		ERROR_RECOVERABLE("Reached Max Limit for debris on screen");
	}
	else
	{
		for (int index = m_debrisCount; index < totalDebrisCount; index++)
		{
			m_debris[index] = new Debris(this, forEntity->GetPosition(), color, forEntity);
			m_debrisCount++;
		}
	}
}


void Game::SpawnParticles(Vec2 const position, int numberOfPieces, Rgba8 color, Vec2 const& direction, float speed, float fadeoutTime, float minCosmetic, float maxCosmetic)
{
	int totalDebrisCount = m_debrisCount + numberOfPieces;

	if (totalDebrisCount >= MAX_DEBRIS)
	{
		return;
		//ERROR_RECOVERABLE("Reached Max Limit for debris on screen");
	}
	else
	{
		for (int index = m_debrisCount; index < totalDebrisCount; index++)
		{
			m_debris[index] = new Debris(this, position, color, direction, speed, fadeoutTime, minCosmetic, maxCosmetic);
			m_debrisCount++;
		}
	}
}


void Game::DeleteGarbage()
{
	//check if space is garbage
	if (m_spaceShip && m_spaceShip->IsGarbage())
	{
		delete m_spaceShip;
		m_spaceShip = nullptr;
	}

// 	for (int bombIndex = 0; bombIndex < PLAYER_SHIP_NUM_BOMBS; bombIndex++)
// 	{
// 		Bomb*& bomb = m_spaceshipBombs[bombIndex];
// 		if (bomb && bomb->IsGarbage())
// 		{
// 			delete bomb;
// 			bomb = nullptr;
// 		}
// 	}

	//check for garbage bullets
	DeleteGarbageBullets();

	//Check for garbage debris
	DeleteGarbageDebris();
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

void Game::DeleteGarbageBullets()
{
	for (int index = 0; index < m_bulletCount; index++)
	{
		Bullet*& indexBullet = m_bullets[index];

		if (indexBullet && indexBullet->IsGarbage())
		{
			Bullet*& lastBullet = m_bullets[m_bulletCount - 1];

			//delete bullet at index and replace it with the last bullet fired
			delete indexBullet;
			indexBullet = lastBullet;
			lastBullet = nullptr;
			m_bulletCount--; //reduce the bullet count by 1
			index--; //reduce the index by 1 so that the replaced bullet can be check as well
		}
	}
}

void Game::DeleteGarbageDebris()
{
	for (int index = 0; index < m_debrisCount; index++)
	{
		Debris*& indexDebris = m_debris[index];

		if (indexDebris && indexDebris->IsGarbage())
		{
			Debris*& lastDebris = m_debris[m_debrisCount - 1];

			//delete bullet at index and replace it with the last bullet fired
			delete indexDebris;
			indexDebris = lastDebris;
			lastDebris = nullptr;
			m_debrisCount--; //reduce the bullet count by 1
			index--; //reduce the index by 1 so that the replaced debris can be check as well
		}
	}
}

bool Game::CheckCollisionWithWorldBoundary(Entity* entity)
{
	if (!entity->IsAlive())
		return false;

	Vec2 entityPos = entity->GetPosition();
	float physicsRadius = entity->GetPhysicsRadius();

	if ((WORLD_SIZE_X - entityPos.x) <= physicsRadius || (entityPos.x - 0.0f) <= physicsRadius
	 || (WORLD_SIZE_Y - entityPos.y) <= physicsRadius || (entityPos.y - 0.0f) <= physicsRadius)
	{
		return true;
	}

	return false;
}

bool Game::AreEntitiesColliding(Entity* entityA, Entity* entityB)
{
	if (!entityA->IsAlive() || !entityB->IsAlive())
		return false;

	Vec2 entityPosA = entityA->GetPosition();
	Vec2 entityPosB = entityB->GetPosition();
	float entityRadiusA = entityA->GetPhysicsRadius();
	float entityRadiusB = entityB->GetPhysicsRadius();

	return DoDiscsOverlap2D(entityPosA, entityRadiusA, entityPosB, entityRadiusB);
}

Vec2 const Game::GetLocationOffScreen(float distance) const
{
	float minX = /*0.0f - */distance;
	float maxX = WORLD_SIZE_X - distance;
	
	float minY = /*0.0f - */distance;
	float maxY = WORLD_SIZE_Y - distance;

	Vec2 position = g_rng->GetRandomPositionInAABB2(minX, minY, maxX, maxY);
	Vec2 playerPos = m_spaceShip->GetPosition();// Vec2(PLAYER_SHIP_STARTING_X, PLAYER_SHIP_STARTING_Y);
	float playerCosmeticRadius = m_spaceShip->GetCosmeticRadius();
	float distanceThreshold = distance + playerCosmeticRadius * 2.0f;
	while ((position - playerPos).GetLengthSquared() <= distanceThreshold * distanceThreshold)
	{
		position = g_rng->GetRandomPositionInAABB2(minX, minY, maxX, maxY);
	}


// 	if (g_rng->GetRandomIntLessThan(2) == 0)
// 	{
// 		position.x = g_rng->GetRandomIntLessThan(2) == 0 ? minX : maxX;
// 		position.y = g_rng->GetRandomFloatInRange(minY, maxY);
// 	}
// 	else
// 	{
// 		position.x = g_rng->GetRandomFloatInRange(minX, maxX);
// 		position.y = g_rng->GetRandomIntLessThan(2) == 0 ? minY : maxY;
// 	}

	return position;
}

Vec2 const Game::GetRandomUICanvasLocation() const
{
	float randomX = g_rng->GetRandomFloatInRange(0.0f, UI_CANVAS_SIZE_X);
	float randomY = g_rng->GetRandomFloatInRange(0.0f, UI_CANVAS_SIZE_Y);

	return Vec2(randomX, randomY);
}

void Game::CheckEntityCollisions()
{
	if (m_isGameOver)
		return;

	//check for bullet-asteroid collisions
	CheckCollisionsBetweenBulletAndEnemies();

	if (m_spaceShip)
	{
		m_enemyWaves[m_currentWaveIndex]->CheckSpaceshipEnemyCollisions();
	}
// 
// 	for (int bombIndex = 0; bombIndex < PLAYER_SHIP_NUM_BOMBS; bombIndex++)
// 	{
// 		Bomb*& bomb = m_spaceshipBombs[bombIndex];
// 		if (bomb && bomb->IsAlive())
// 		{
// 			m_enemyWaves[m_currentWaveIndex]->CheckBombEnemyCollisions(bomb);
// 		}
// 	}
}

void Game::CheckCollisionsBetweenBulletAndEnemies()
{
	for (int bulletIndex = 0; bulletIndex < m_bulletCount; bulletIndex++)
	{
		Bullet*& bullet = m_bullets[bulletIndex];

		if (bullet)
		{
			m_enemyWaves[m_currentWaveIndex]->CheckBulletEnemyCollisions(bullet);
		}
	}
}