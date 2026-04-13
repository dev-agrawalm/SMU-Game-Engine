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
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Game/Player.hpp"
#include "Game/Enemy.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/Bullet.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/BackgroundStar.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB2.hpp"

RandomNumberGenerator* g_rng = nullptr;
SpriteSheet* g_gameSpriteSheet = nullptr;

static bool OnPlayerDeathEvent(EventArgs& args)
{
	UNUSED(args);
	g_game->SetGameState(GameState::GAME_STATE_GAME_OVER);
	return false;
}


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
	Texture* texture = g_theRenderer->CreateOrGetTexture("Data/Images/GameSpriteSheet12x12.png");
	g_gameSpriteSheet = new SpriteSheet(*texture, IntVec2(12, 12));
	m_gameState = GameState::GAME_STATE_LEVEL_EDITOR;
	SetGameState(m_gameState);
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
		case GameState::GAME_STATE_ATTRACT:			DeInitAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:			DeInitPlayMode();		break;
		case GameState::GAME_STATE_GAME_OVER:		DeInitGameOverMode();	break;		
		case GameState::GAME_STATE_LEVEL_EDITOR:	DeInitLevelEditor();	break;
		case GameState::NONE:						//fallthrough
		default: break;
	}

	if (g_gameSpriteSheet != nullptr)
	{
		delete g_gameSpriteSheet;
		g_gameSpriteSheet = nullptr;
	}

	if (g_rng != nullptr)
	{
		delete g_rng;
		g_rng = nullptr;
	}
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
		m_gameClock.StepFrame();
	}

	if (g_inputSystem->WasKeyJustPressed('P') && m_gameState != GameState::GAME_STATE_LEVEL_EDITOR)
	{
		m_gameClock.TogglePause();
	}

	if (g_inputSystem->IsKeyPressed('T'))
	{
		SetTimeScale(0.2f);
	}
	else if (g_inputSystem->IsKeyPressed('Y'))
	{
		SetTimeScale(5.0f);
	}
	else if (g_inputSystem->IsKeyPressed('R'))
	{
		SetTimeScale(1.0f);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPEN_FULL);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_CTRL))
	{
		g_eventSystem->FireEvent("PlayerDeathAnimationFinishedEvent");
	}
}


void Game::Update()
{
// 	float textHeight = 16.0f;
// 	float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
// 	Clock& systemClock = Clock::GetSystemClock();
// 	std::string systemClockString = "System -- " + ClockToString(&systemClock);
// 	std::string gameClockString = "Game   -- " + ClockToString(&m_gameClock);
// 	Vec2 cursorPos = Vec2(0.0f, uiCanvasSizeY);
// 	DebugAddScreenText(systemClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);
// 	cursorPos.y -= textHeight;
// 	DebugAddScreenText(gameClockString, cursorPos, 0.0f, Vec2(0.0f, 1.0f), textHeight);

	CheckInputDeveloperCheats();
	float deltaSeconds = (float) m_gameClock.GetFrameDeltaSeconds();

	//check input
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:			CheckInputAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:			CheckInputPlayMode();		break;
		case GameState::GAME_STATE_GAME_OVER:		CheckInputGameOverMode();	break;
		case GameState::GAME_STATE_LEVEL_EDITOR:	CheckInputLevelEditor();	break;
		case GameState::NONE:						//fallthrough case
		default: break;
	}

	//update game mode
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:			UpdateAttractMode(deltaSeconds);	break;
		case GameState::GAME_STATE_PLAYING:			UpdatePlayMode(deltaSeconds);		break;
		case GameState::GAME_STATE_GAME_OVER:		UpdateGameOverMode(deltaSeconds);	break;
		case GameState::GAME_STATE_LEVEL_EDITOR:	UpdateLevelEditor(deltaSeconds);	break;
		case GameState::NONE: //fallthrough case
		default: break;
	}

	UpdateCameras(deltaSeconds);
}


void Game::UpdateCameras(float deltaSeconds)
{
	static float worldSizeX	= g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
	static float worldSizeY	= g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);
	static float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	static float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);

	m_worldCamera.SetOrthoView(0.0f, 0.0f, worldSizeX, worldSizeY);
	m_uiCamera.SetOrthoView(0.0f, 0.0f, uiCanvasSizeX, uiCanvasSizeY);

	if (m_worldCamScreenShakeDuration > 0.0f)
	{
		m_worldCamScreenShakeDuration -= deltaSeconds;
		float randomX = g_rng->GetRandomFloatInRange(-m_worldCamScreenShakeMagnitude, m_worldCamScreenShakeMagnitude);
		float randomY = g_rng->GetRandomFloatInRange(-m_worldCamScreenShakeMagnitude, m_worldCamScreenShakeMagnitude);
		m_worldCamera.TranslateCamera2D(Vec2(randomX, randomY));
	}
}


void Game::Render() const
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:			RenderAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:			RenderPlayMode();		break;
		case GameState::GAME_STATE_GAME_OVER:		RenderGameOverMode();	break;
		case GameState::GAME_STATE_LEVEL_EDITOR:	RenderLevelEditor();	break;
		case GameState::NONE:						//fallthrough case
		default: break;
	}
}


void Game::InitAttractMode()
{
	SplineDefinition::LoadSplineDefinitionsFromXml();
	BulletDefinition::LoadBulletDefinitionsFromXml();
	WeaponDefinition::LoadWeaponDefinitionsFromXml();
	EnemyDefinition::LoadEnemyDefinitionsFromXml();

	SpawnInfo playerInfo = {};
	playerInfo.m_entityType = ENTITY_TYPE_PLAYER;
	SpawnEntity(playerInfo);

	static SoundID attractScreenMusicID = g_audioSystem->CreateOrGetSound("Data/Audio/AttractScreen.wav");
	static float attractScreenMusicVolume = g_gameConfigBlackboard.GetValue("attractScreenMusicVolume", 0.0f);
	m_attractScreenMusic = g_audioSystem->StartSound(attractScreenMusicID, true, attractScreenMusicVolume);
}


void Game::DeInitAttractMode()
{
	g_audioSystem->StopSound(m_attractScreenMusic);

	for (int entityIndex = 0; entityIndex < (int) m_allEntities.size(); entityIndex++)
	{
		Entity*& e = m_allEntities[entityIndex];
		if (e)
		{
			e = RemoveEntityFromGame(e);
			delete e;
			e = nullptr;
		}
	}
	m_allEntities.clear();

	EnemyDefinition::DeleteAllEnemyDefinitions();
	WeaponDefinition::DeleteAllWeaponDefinitions();
	BulletDefinition::DeleteAllBulletDefinitions();
	SplineDefinition::DeleteAllSplineDefinitions();
}


void Game::CheckInputAttractMode()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		SetGameState(GameState::GAME_STATE_LEVEL_EDITOR);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ENTER))
	{
		SetGameState(GameState::GAME_STATE_PLAYING);
	}
}


void Game::UpdateAttractMode(float deltaSeconds)
{
	for (int entityIndex = 0; entityIndex < (int) m_allEntities.size(); entityIndex++)
	{
		Entity*& e = m_allEntities[entityIndex];
		if (e)
			e->Update(deltaSeconds);
	}

	CollectGarbage();
}


void Game::RenderAttractMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	g_theRenderer->BeginCamera(m_worldCamera);
	{
		for (int entityIndex = 0; entityIndex < (int) m_allEntities.size(); entityIndex++)
		{
			Entity* const& e = m_allEntities[entityIndex];
			if (e)
				e->Render();
		}
	}
	g_theRenderer->EndCamera(m_worldCamera);

	static float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	static float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	g_theRenderer->BeginCamera(m_uiCamera);
	{
		AABB2 titleImageAABB2 = AABB2(Vec2(uiCanvasSizeX * 0.5f, uiCanvasSizeY * 0.5f), uiCanvasSizeX, uiCanvasSizeY);
		RenderMaterial material = {};
		material.m_texture = g_theRenderer->CreateOrGetTexture("Data/Images/titleScreen.png");
		g_theRenderer->DrawAABB2D(titleImageAABB2, material);
	
		static BitmapFont* bitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
		std::vector<Vertex_PCU> attractScreenTextVerts;
		float fontSize = 8.0f;
		
		static std::string howToText = Stringf("\nWASD: move around\n\nSpacebar: fire weapon\n\nQ/E: cycle through weapons");
		Vec2 tutorialTextCursorPos = Vec2(uiCanvasSizeX * 0.6f, uiCanvasSizeY * 0.3f);
		bitmapFont->AddVertsForTextInAABB2(attractScreenTextVerts, AABB2(tutorialTextCursorPos, uiCanvasSizeX * 0.7f, uiCanvasSizeY * 0.6f), fontSize, howToText,
											Rgba8::WHITE, 1.0f, BitmapFont::ALIGNED_CENTER_LEFT);
		
		static std::string optionsText = Stringf("Enter: Start Game\nEsc: Back to the Editor");
		Vec2 optionsTextCursorPos = Vec2(uiCanvasSizeX * 0.6f, uiCanvasSizeY * 0.15f);
		bitmapFont->AddVertsForTextInAABB2(attractScreenTextVerts, AABB2(optionsTextCursorPos, uiCanvasSizeX * 0.7f, uiCanvasSizeY * 0.6f), fontSize, optionsText,
											Rgba8::WHITE, 1.0f, BitmapFont::ALIGNED_CENTER_LEFT);
		g_theRenderer->BindTexture(0, &bitmapFont->GetTexture());
		g_theRenderer->DrawVertexArray((int) attractScreenTextVerts.size(), attractScreenTextVerts.data());
	}
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::InitLevelEditor()
{
	m_splineEditor.Startup();
}


void Game::DeInitLevelEditor()
{
	m_splineEditor.Shutdown();
}


void Game::CheckInputLevelEditor()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE) && !m_splineEditor.IsSaving())
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ENTER) && !m_splineEditor.IsSaving())
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}

	m_splineEditor.CheckInput();
}


void Game::UpdateLevelEditor(float deltaSeconds)
{
	m_splineEditor.Update(deltaSeconds);
}


void Game::RenderLevelEditor() const
{
	m_splineEditor.Render(m_uiCamera);
}


void Game::InitPlayMode()
{
	g_eventSystem->SubscribeEventCallbackFunction("PlayerDeathAnimationFinishedEvent", OnPlayerDeathEvent);

	SplineDefinition::LoadSplineDefinitionsFromXml();
	BulletDefinition::LoadBulletDefinitionsFromXml();
	WeaponDefinition::LoadWeaponDefinitionsFromXml();
	EnemyDefinition::LoadEnemyDefinitionsFromXml();

	SpawnInfo playerInfo = {};
	playerInfo.m_entityType = ENTITY_TYPE_PLAYER;
	SpawnEntity(playerInfo);
	m_playerScore = 0;

	SpawnBgStars();

	m_currentEnemyWave = GenerateRandomEnemyWave();
	m_latestSpawnedEntityForWave = nullptr;

	static SoundID playModeMusicID = g_audioSystem->CreateOrGetSound("Data/Audio/bgMusicSource3.mp3");
	static float gameBgMusicVolume = g_gameConfigBlackboard.GetValue("gameBgMusicVolume", 0.0f);
	m_gameBgMusic = g_audioSystem->StartSound(playModeMusicID, true, gameBgMusicVolume);
}

void Game::DeInitPlayMode()
{
	SetTimeScale(1.0f);
	g_audioSystem->StopSound(m_gameBgMusic);

	EnemyDefinition::DeleteAllEnemyDefinitions();
	WeaponDefinition::DeleteAllWeaponDefinitions();
	BulletDefinition::DeleteAllBulletDefinitions();
	SplineDefinition::DeleteAllSplineDefinitions();

	for (int entityIndex = 0; entityIndex < (int) m_allEntities.size(); entityIndex++)
	{
		Entity*& e = m_allEntities[entityIndex];
		if (e)
		{
			e = RemoveEntityFromGame(e);
			delete e;
			e = nullptr;
		}
	}
	m_allEntities.clear();

	g_eventSystem->UnsubscribeEventCallbackFunction("PlayerDeathAnimationFinishedEvent", OnPlayerDeathEvent);
}


void Game::CheckInputPlayMode()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}
}

extern int g_numChildBullets;

void Game::UpdatePlayMode(float deltaSeconds)
{
	SpawnEnemyWave();
	for (int entityIndex = 0; entityIndex < (int) m_allEntities.size(); entityIndex++)
	{
		Entity*& e = m_allEntities[entityIndex];
		if (e)
		{
			e->Update(deltaSeconds);
		}
	}

	CheckCollisionsBetweenAllEntities();
	CollectGarbage();
}


void Game::RenderPlayMode() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);

	g_theRenderer->BeginCamera(m_worldCamera);
	{
		for (int entityIndex = 0; entityIndex < (int) m_allEntities.size(); entityIndex++)
		{
			Entity* const& e = m_allEntities[entityIndex];
			if (e)
			{
				e->Render();
			}
		}
	}
	g_theRenderer->EndCamera(m_worldCamera);

	g_theRenderer->BeginCamera(m_uiCamera);
	{
		RenderPlayModeUI();
	}
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::InitGameOverMode()
{

}


void Game::UpdateGameOverMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	CheckInputGameOverMode();
}


void Game::CheckInputGameOverMode()
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		HandleQuitRequest();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ENTER))
	{
		SetGameState(GameState::GAME_STATE_ATTRACT);
	}
}


void Game::RenderGameOverMode() const
{
	static float uiCanvasSizeX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	static float uiCanvasSizeY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	g_theRenderer->BeginCamera(m_uiCamera);
	{
		AABB2 titleImageAABB2 = AABB2(Vec2(uiCanvasSizeX * 0.5f, uiCanvasSizeY * 0.5f), uiCanvasSizeX, uiCanvasSizeY);
		RenderMaterial material = {};
		material.m_texture = g_theRenderer->CreateOrGetTexture("Data/Images/GameOver.jpg");
		g_theRenderer->DrawAABB2D(titleImageAABB2, material);

		std::vector<Vertex_PCU> resultsScreenTextVerts;
		static std::string finalScoreText = Stringf("\n\nFinal Score: %i", m_playerScore, m_playerScore);
		static std::string optionsText = Stringf("\n\n\nEsc: Quit Game \nEnter: Main menu");
		static BitmapFont* bitmapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
// 		bitmapFont->AddVertsForText2D(resultsScreenTextVerts, Vec2(uiCanvasSizeX * 0.3f, uiCanvasSizeY * 0.10f), 16.0f, optionsText);
// 		bitmapFont->AddVertsForText2D(resultsScreenTextVerts, Vec2(uiCanvasSizeX * 0.43f, uiCanvasSizeY * 0.35f), 24.0f, finalScoreText);
		float fontSize = 14.0f;
		bitmapFont->AddVertsForTextInAABB2(resultsScreenTextVerts, AABB2(Vec2(uiCanvasSizeX * 0.55f, uiCanvasSizeY * 0.35f), uiCanvasSizeX * 0.6f, uiCanvasSizeY * 0.7f), 
											fontSize, finalScoreText + "\n" + optionsText, Rgba8::WHITE, 1.0f, BitmapFont::ALIGNED_CENTER_LEFT);
		g_theRenderer->BindTexture(0, &bitmapFont->GetTexture());
		g_theRenderer->DrawVertexArray((int) resultsScreenTextVerts.size(), resultsScreenTextVerts.data());
	}
	g_theRenderer->EndCamera(m_uiCamera);
}


void Game::DeInitGameOverMode()
{

}


void Game::RenderPlayModeUI() const
{
	float consoleCameraOrthoX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);

	float fontSize = 8.0f;
	if (g_gameConfigBlackboard.GetValue("debugEntityCount", false))
	{
		DebugAddScreenText(Stringf("m_allEntities Size: %i", (int) m_allEntities.size()), Vec2(consoleCameraOrthoX, 0.0f), 0.0f, Vec2(1.0f, 0.0f), 16.0f);
		DebugAddScreenText(Stringf("Num of child bullets spawned: %i", g_numChildBullets), Vec2(0.0f, 0.0f), 0.0f, Vec2(0.0f, 0.0f), 16.0f, Rgba8::WHITE);
	}

	if (m_debugCooldownTimer)
	{
		double elapsedTime = m_enemyWaveCooldownTimer.GetElapsedSeconds();
		DebugAddScreenText(Stringf("Cooldown Timer Elapsed time: %.3f", elapsedTime), Vec2(consoleCameraOrthoX, 0.0f), 0.0f, Vec2(1.0f, 0.0f), 16.0f);
	}

	if (g_gameConfigBlackboard.GetValue("displayScore", false))
	{
		int playerHealth = m_player->GetHealth();
		float cursorPosY = 0.0f;
		float topPadding = 1.0f;
		playerHealth = playerHealth > 0 ? playerHealth : 0;
		DebugAddScreenText(Stringf("Player health: %i", playerHealth), Vec2(consoleCameraOrthoX, cursorPosY), 0.0f, Vec2(1.0f, 0.0f), fontSize);
		DebugAddScreenText(Stringf("Player score: %i", m_playerScore), Vec2(consoleCameraOrthoX, cursorPosY + fontSize + topPadding), 0.0f, Vec2(1.0f, 0.0f), fontSize);
	}
}


Entity* Game::SpawnEntity(SpawnInfo const& spawnInfo)
{
	switch (spawnInfo.m_entityType)
	{
		case ENTITY_TYPE_INVALID:
			break;
		case ENTITY_TYPE_PLAYER:
		{
			m_player = new Player();
			AddEntityToGame(m_player);
			return m_player;
		}
		case ENTITY_TYPE_ENEMY:
		{
			Entity* enemy = new Enemy(spawnInfo.m_enemyDefinition);
			AddEntityToGame(enemy);
			return enemy;
		}
		case ENTITY_TYPE_ENEMY_BULLET:
		{
			Entity* bullet = new Bullet(spawnInfo.m_position, spawnInfo.m_orientation, ENTITY_FACTION_ENEMY, spawnInfo.m_bulletDefinition);
			AddEntityToGame(bullet);
			return bullet;
		}
		case ENTITY_TYPE_PLAYER_BULLET:
		{
			Entity* bullet = new Bullet(spawnInfo.m_position, spawnInfo.m_orientation, ENTITY_FACTION_PLAYER, spawnInfo.m_bulletDefinition);
			AddEntityToGame(bullet);
			return bullet;
		}
		case ENTITY_TYPE_BG_STAR:
		{
			Entity* star = new BackgroundStar(spawnInfo.m_position);
			AddEntityToGame(star);
			return star;
		}
		default:
			break;
	}

	return nullptr;
}


Entity* Game::AddEntityToGame(Entity* entityToAdd)
{
	AddEntityToList(entityToAdd, m_allEntities);
	EntityFaction faction = entityToAdd->GetFaction();
	AddEntityToList(entityToAdd, m_entitiesByFaction[faction]);
	return entityToAdd;
}


Entity* Game::RemoveEntityFromGame(Entity* entityToRemove)
{
	RemoveEntityFromList(entityToRemove, m_allEntities);
	EntityFaction faction = entityToRemove->GetFaction();
	RemoveEntityFromList(entityToRemove, m_entitiesByFaction[faction]);
	return entityToRemove;
}


int Game::AddPointsToPlayerScore(int pointsToAdd)
{
	m_playerScore += pointsToAdd;
	return m_playerScore;
}


EnemyWave Game::GenerateRandomEnemyWave()
{
	IntRange enemyCountRange = g_gameConfigBlackboard.GetValue("enemyCountRange", IntRange(5, 10));
	int numEnemies = g_rng->GetRandomIntInRange(enemyCountRange.m_min, enemyCountRange.m_max);
	float waveCooldownPeriod = g_gameConfigBlackboard.GetValue("enemyWaveCooldown", 10.0f);
	float spawnCooldownPeriod = g_gameConfigBlackboard.GetValue("enemySpawnCooldown", 0.5f);
	int enemyDefinitionCount = EnemyDefinition::GetEnemyDefinitionCount();
	int enemyDefinitionIndex = g_rng->GetRandomIntLessThan(enemyDefinitionCount);
	EnemyDefinition const* enemyDefinition = EnemyDefinition::GetDefinitionByIndex(enemyDefinitionIndex);

	EnemyWave randomWave = {};
	randomWave.m_numEnemies = numEnemies;
	randomWave.m_enemyDef = enemyDefinition;
	randomWave.m_enemyWaveCooldownSeconds = waveCooldownPeriod;
	randomWave.m_enemySpawnCooldownSeconds = spawnCooldownPeriod;
	return randomWave;
}


void Game::SpawnEnemyWave()
{
	if (m_enemyWaveCooldownTimer.HasElapsed())
	{
		m_latestSpawnedEntityForWave = nullptr;
		m_currentEnemyWave = GenerateRandomEnemyWave();
		m_enemyWaveCooldownTimer.DecrementAllAndPause();
		m_enemyWaveCooldownTimer.SetDuration(m_currentEnemyWave.m_enemyWaveCooldownSeconds);
		m_debugCooldownTimer = false;
	}

	if (m_currentEnemyWave.m_numEnemies > 0)
	{
		if (m_enemySpawnCooldownTimer.HasElapsed())
		{
			SpawnInfo enemyInfo = {};
			enemyInfo.m_enemyDefinition = m_currentEnemyWave.m_enemyDef;
			enemyInfo.m_entityType = ENTITY_TYPE_ENEMY;
			Entity* spawnedEntity = SpawnEntity(enemyInfo);
			if (m_latestSpawnedEntityForWave == nullptr)
			{
				m_latestSpawnedEntityForWave = dynamic_cast<Enemy*>(spawnedEntity);
			}
			else
			{
				Enemy* currentSpawnedEnemy = dynamic_cast<Enemy*>(spawnedEntity);
				if (m_latestSpawnedEntityForWave && currentSpawnedEnemy)
				{
					m_latestSpawnedEntityForWave->SetEnemyBehind(currentSpawnedEnemy);
					currentSpawnedEnemy->SetEnemyInFront(m_latestSpawnedEntityForWave);
					m_latestSpawnedEntityForWave = currentSpawnedEnemy;
				}
			}
			m_currentEnemyWave.m_numEnemies--;
			m_enemySpawnCooldownTimer.Start(m_currentEnemyWave.m_enemySpawnCooldownSeconds);
		}

		if (m_currentEnemyWave.m_numEnemies <= 0)
		{
			m_enemyWaveCooldownTimer.Resume();
			m_debugCooldownTimer = true & g_gameConfigBlackboard.GetValue("debugCooldownTimer", false);
		}
	}
}


void Game::CheckCollisionsBetweenAllEntities()
{
	CheckCollisionBetweenEntityLists(m_entitiesByFaction[ENTITY_FACTION_ENEMY], m_entitiesByFaction[ENTITY_FACTION_PLAYER]);
}


void Game::SetGameState(GameState gameState)
{
	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:			DeInitAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:			DeInitPlayMode();		break;
		case GameState::GAME_STATE_GAME_OVER:		DeInitGameOverMode();	break;
		case GameState::GAME_STATE_LEVEL_EDITOR:	DeInitLevelEditor();	break;
		case GameState::NONE: //fall through
		default: break;
	}

	m_gameState = gameState;

	switch (m_gameState)
	{
		case GameState::GAME_STATE_ATTRACT:			InitAttractMode();	break;
		case GameState::GAME_STATE_PLAYING:			InitPlayMode();		break;
		case GameState::GAME_STATE_GAME_OVER:		InitGameOverMode(); break;
		case GameState::GAME_STATE_LEVEL_EDITOR:	InitLevelEditor();	break;
		case GameState::NONE: //fall through
		default: break;
	}
}


void Game::SetTimeScale(float timeScale)
{
	m_gameClock.SetTimeScale((double) timeScale);
}


void Game::AddScreenShakeToWorld(float magnitude, float duration)
{
	m_worldCamScreenShakeMagnitude = magnitude;
	m_worldCamScreenShakeDuration = duration;
}


void Game::SetLatestSpawnedEntity(Enemy* enemy)
{
	m_latestSpawnedEntityForWave = enemy;
}


Enemy* Game::GetLatestSpawnedEntity() const
{
	return m_latestSpawnedEntityForWave;
}


bool Game::IsDebugModeActive() const
{
	return m_debugMode;
}


Player* Game::GetPlayer() const
{
	return m_player;
}


Clock const& Game::GetClock() const
{
	return m_gameClock;
}


Entity* Game::AddEntityToList(Entity* entityToAdd, EntityList& list)
{
	if (entityToAdd)
	{
		for (int listIndex = 0; listIndex < (int) list.size(); listIndex++)
		{
			Entity*& e = list[listIndex];
			if (e == nullptr)
			{
				e = entityToAdd;
				return e;
			}
		}
	}

	list.push_back(entityToAdd);
	return entityToAdd;
}


Entity* Game::RemoveEntityFromList(Entity* entityToRemove, EntityList& list)
{
	if (entityToRemove)
	{
		for (int listIndex = 0; listIndex < (int) list.size(); listIndex++)
		{
			Entity*& e = list[listIndex];
			if (e == entityToRemove)
			{
				e = nullptr;
				break;
			}
		}
	}

	return entityToRemove;
}


void Game::SpawnBgStars()
{
	static FloatRange rangeX = g_gameConfigBlackboard.GetValue("bgStarXRange", FloatRange::ZERO_TO_ONE);
	static FloatRange rangeY = g_gameConfigBlackboard.GetValue("bgStarYRange", FloatRange::ZERO_TO_ONE);
	static int numStars = g_gameConfigBlackboard.GetValue("numBgStars", 50);

	for (int starIndex = 0; starIndex < numStars; starIndex++)
	{
		SpawnInfo spawnInfo = {};
		spawnInfo.m_entityType = ENTITY_TYPE_BG_STAR;
		spawnInfo.m_position = g_rng->GetRandomPositionInAABB2(Vec2(rangeX.m_min, rangeY.m_min), Vec2(rangeX.m_max, rangeY.m_max));
		SpawnEntity(spawnInfo);
	}
}


void Game::CheckCollisionBetweenEntityLists(EntityList& listA, EntityList& listB)
{
	for (int a = 0; a < (int) listA.size(); a++)
	{
		for (int b = 0; b < (int) listB.size(); b++)
		{
			Entity* entityA = listA[a];
			Entity* entityB = listB[b];
			if (entityA && entityB)
			{
				CheckAndResolveCollisionBetweenEntities(entityA, entityB);
			}
		}
	}
}


bool Game::CheckAndResolveCollisionBetweenEntities(Entity*& entityA, Entity*& entityB)
{
	if (entityA->IsDead() || entityB->IsDead())
		return false;

	if (entityA->IsBullet() && entityB->IsBullet())
		return false;

	Vec2 positionA = entityA->GetPosition();
	float radiusA = entityA->GetPhysicsRadius();
	Vec2 positionB = entityB->GetPosition();
	float radiusB = entityB->GetPhysicsRadius();

	bool didCollide = DoDiscsOverlap2D(positionA, radiusA, positionB, radiusB);
	if (didCollide)
	{
		Vec2 newPositionA = positionA;
		Vec2 newPositionB = positionB;
		PushDisksOutOfEachOther2D(newPositionA, radiusA, newPositionB, radiusB);
		entityA->SetPosition(newPositionA);
		entityB->SetPosition(newPositionB);

		if (entityA->GetFaction() != entityB->GetFaction())
		{
			int damageA = entityA->GetDamageOnCollision();
			entityB->TakeDamage(damageA);

			int damageB = entityB->GetDamageOnCollision();
			entityA->TakeDamage(damageB);
		}

		if (entityA->DieOnImpact())
		{
			entityA->Die();
		}

		if (entityB->DieOnImpact())
		{
			entityB->Die();
		}
	}

	return didCollide;
}


void Game::CollectGarbage()
{
	for (int entityIndex = 0; entityIndex < (int) m_allEntities.size(); entityIndex++)
	{
		Entity*& e = m_allEntities[entityIndex];
		if (e && e->IsGarbage())
		{
			e = RemoveEntityFromGame(e);
			delete e;
			e = nullptr;
		}
	}
}
