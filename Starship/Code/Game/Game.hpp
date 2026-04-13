#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"

enum class GameMode
{
	INVALID,
	NONE,
	ATTRACT,
	PLAY
};

enum class GameplayState
{
	INVALID,
	NONE,
	PLAYING,
	GAME_WON,
	GAME_LOST
};

class Entity;
class SpaceShip;
class Bullet;
class Debris;
class EnemyWave;
class XboxController;
class RandomNumberGenerator;
class Bomb;

extern RandomNumberGenerator* g_rng;

//class representing an instance of our game
//it is owned by App
class Game
{
public: //methods
	Game();
	~Game();

	void Startup();
	void Update(float deltaSeconds);
	void Render() const;
	void ShutDown();
	void HandleQuitRequest();


	void InitAttractMode();
	void UpdateAttractMode(float deltaSeconds);
	void CheckInputAttractMode();
	void RenderAttractMode() const;
	void DeInitAttractMode();

	void InitPlayMode();
	void RenderPlayModeUI() const;
	void UpdatePlayMode(float deltaSeconds);

	void SpawnStars(int numStars);

	void CheckInputPlayMode();
	void CheckForGameOver();
	void RenderPlayMode() const;
	void DeInitPlayMode();

	void RespawnShip();
	void FireBullet(Vec2 const& firePosition, float orientationDegree);
	void SpawnBomb(Vec2 const& bombPosition);
	void SpawnDebris(Entity* forEntity, int numberOfPieces, Rgba8 color);
	void SpawnParticles(Vec2 const position, int numberOfPieces, Rgba8 color, Vec2 const& direction, float speed, float fadeoutTime, float minCosmetic = 0.05f, float maxCosmetic = 0.2f);
	
	bool CheckCollisionWithWorldBoundary(Entity* entity);
	bool AreEntitiesColliding(Entity* entityA, Entity* entityB);
	void DeleteGarbage();

	//mutators
	void SetGameMode(GameMode gameMode);
	void SetWorldCamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration);
	void SetUICamScreenShakeParameters(float screenShakeMagnitude, float screenShakeDuration);
	//getters
	Vec2 const GetLocationOffScreen(float maxDist) const;
	Vec2 const GetRandomUICanvasLocation() const;
private: //methods
	void CheckEntityCollisions();
	void CheckCollisionsBetweenBulletAndEnemies();
	void DeleteGarbageBullets();
	void DeleteGarbageDebris();
	void RenderDebris() const;
	void RenderBullets() const;
	void DebugRenderEntityDetails() const; // visualizes entity information (velocity, hit box, etc) for debugging
	void DebugRenderEntitySpaceshipDistance() const; //draws a line from the center of the space ship to the center of every other entity on screen
	void PlayGameOverMusic();
private: //methods
	//spaceship variables
	SpaceShip* m_spaceShip = nullptr;
	XboxController const* m_playerController = nullptr;
	//Bomb* m_spaceshipBombs[PLAYER_SHIP_NUM_BOMBS] = {};

	//bullet variables
	Bullet* m_bullets[MAX_BULLETS] = {};
	int m_bulletCount = 0;
	
	//debris variables
	Debris* m_debris[MAX_DEBRIS] = {};
	int m_debrisCount = 0;
	
	//enemy wave variables
	EnemyWave* m_enemyWaves[NUM_WAVES] = {};
	int m_currentWaveIndex = 0;
	
	//cameras
	Camera m_worldCamera;
	Camera m_uiCamera;
	
	//World Cam Shake
	bool m_shouldShakeWorldCam = false;
	float m_worldCamScreenShakeMagnitude = 0.0f;
	float m_worldCamScreenShakeDuration = 0.0f;
	float m_worldCamTimeSinceScreenShakeStart = 0.0f;
	
	//UI Cam Shake
	bool m_shouldShakeUICam = false;
	float m_uiCamScreenShakeMagnitude = 0.0f;
	float m_uiCamScreenShakeDuration = 0.0f;
	float m_uiCamTimeSinceScreenShakeStart = 0.0f;
	
	//game variables
	bool m_isGameOver = false;
	bool m_isPaused = false;
	bool m_debugRender = false;
	float m_timeSinceGameOver = 0.0f;
	GameMode m_gameMode = GameMode::NONE;
	GameplayState m_gameplayState = GameplayState::NONE;

	SoundPlaybackID m_attractModePlaybackId = MISSING_SOUND_ID;
};