#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/BezierCurve2D.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Clock.hpp"
#include "Game/SplineEditor.hpp"
#include "Game/Entity.hpp"
#include "Engine/Audio/AudioSystem.hpp"

enum class GameState
{
	NONE,
	GAME_STATE_LEVEL_EDITOR,
	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_GAME_OVER
};

class Player;
class Bullet;
class Enemy;

struct BulletDefinition;
struct EnemyDefinition;

typedef std::vector<Entity*> EntityList;


struct EnemyWave
{
public:
	int m_numEnemies = 0;
	EnemyDefinition const* m_enemyDef = nullptr;
	float m_enemyWaveCooldownSeconds = 0.0f;
	float m_enemySpawnCooldownSeconds = 0.0f;
};


struct SpawnInfo
{
	Vec2 m_position;
	float m_orientation = 0.0f;
	EntityType m_entityType;
	BulletDefinition const* m_bulletDefinition = nullptr;
	EnemyDefinition const* m_enemyDefinition = nullptr;
};

//class representing an instance of our game
//it is owned by App
class Game
{
public: //methods
	Game();
	~Game();

	void Startup();
	void ShutDown();
	void LoadAssets();
	void HandleQuitRequest();
	void CheckInputDeveloperCheats();
	void Update();
	void UpdateCameras(float deltaSeconds);
	void Render() const;

	void InitAttractMode();
	void DeInitAttractMode();
	void CheckInputAttractMode();
	void UpdateAttractMode(float deltaSeconds);
	void RenderAttractMode() const;

	void InitLevelEditor();
	void DeInitLevelEditor();
	void CheckInputLevelEditor();
	void UpdateLevelEditor(float deltaSeconds);
	void RenderLevelEditor() const;

	void InitPlayMode();
	void DeInitPlayMode();
	void CheckInputPlayMode();
	void UpdatePlayMode(float deltaSeconds);
	void RenderPlayMode() const;
	void RenderPlayModeUI() const;
	void CheckCollisionsBetweenAllEntities();
	void CheckCollisionBetweenEntityLists(EntityList& listA, EntityList& listB);
	bool CheckAndResolveCollisionBetweenEntities(Entity*& entityA, Entity*& entityB);
	void CollectGarbage();
	Entity* SpawnEntity(SpawnInfo const& spawnInfo);
	Entity* AddEntityToGame(Entity* entityToAdd);
	Entity* RemoveEntityFromGame(Entity* entityToRemove);
	int		AddPointsToPlayerScore(int pointsToAdd);

	EnemyWave GenerateRandomEnemyWave();
	void SpawnEnemyWave();

	void InitGameOverMode();
	void UpdateGameOverMode(float deltaSeconds);
	void CheckInputGameOverMode();
	void RenderGameOverMode() const;
	void DeInitGameOverMode();

	//mutators
	void SetGameState(GameState gameState);
	void SetTimeScale(float timeScale);

	void AddScreenShakeToWorld(float magnitude, float duration);
	//void AddScreenShakeToUI(float magnitude, float duration);

	void SetLatestSpawnedEntity(Enemy* enemy);
	Enemy* GetLatestSpawnedEntity() const;
	bool IsDebugModeActive() const;
	Player* GetPlayer() const;
	Clock const& GetClock() const;
private:
	Entity* AddEntityToList(Entity* entityToAdd, EntityList& list);
	Entity* RemoveEntityFromList(Entity* entityToRemove, EntityList& list);
	void SpawnBgStars();

private: //methods
	Camera m_worldCamera;
	Camera m_uiCamera;
	
	//world screen shake
	float m_worldCamScreenShakeMagnitude = 0.0f;
	float m_worldCamScreenShakeDuration = 0.0f;

	//ui screen shake
	float m_uiCamScreenShakeMagnitude = 0.0f;
	float m_uiCamScreenShakeDuration = 0.0f;

	bool m_debugMode = false;
	GameState m_gameState = GameState::NONE;

	//play
	int m_playerScore = 0;
	Player* m_player = nullptr;
	EntityList m_allEntities;
	EntityList m_entitiesByFaction[ENTITY_FACTION_COUNT];

	Enemy* m_latestSpawnedEntityForWave = nullptr;
	EnemyWave m_currentEnemyWave;
	Stopwatch m_enemyWaveCooldownTimer;
	Stopwatch m_enemySpawnCooldownTimer;
	bool m_debugCooldownTimer = false;

	SplineEditor m_splineEditor;

	Stopwatch m_timer;
	Clock m_gameClock;

	SoundPlaybackID m_attractScreenMusic = 0;
	SoundPlaybackID m_gameBgMusic = 0;
};
