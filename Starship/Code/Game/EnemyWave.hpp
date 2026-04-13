#pragma once
#include "Engine/Audio/AudioSystem.hpp"

class Asteroid;
class Beetle;
class Wasp;
class SpaceShip;
class Game;
class Bullet;
class Bomb;

class EnemyWave
{
public:
	EnemyWave();
	EnemyWave(int const numAsteroids, int const numWasps, int const numBeetles);
	~EnemyWave();
	void Init(SpaceShip* spaceShip, Game* game, char const* bgMusicString = nullptr);
	void Deinit();
	void Update(float deltaSeconds);
	void Render() const;
	void DebugRenderEnemyDetails() const;
	void DebugRenderEnemySpaceshipDistance() const;
	bool IsWaveOver() const;
	void SpawnEnemies();
	void CheckSpaceshipEnemyCollisions();
	void CheckBulletEnemyCollisions(Bullet* bullet);
	void CheckBombEnemyCollisions(Bomb* bomb);

	SoundPlaybackID GetBgMusicPlaybackId() const;
private:
	void DebugRenderWaspDetails() const;
	void DebugRenderAsteroidDetails() const;
	void DebugRenderBeetleDetails() const;

	void DebugRenderWaspSpaceshipDistance() const;
	void DebugRenderBeetleSpaceshipDistance() const;
	void DebugRenderAsteroidSpaceshipDistance() const;

	void CheckBulletWaspCollisions(Bullet* bullet);
	void CheckBulletAsteroidCollisions(Bullet* bullet);
	void CheckBulletBeetleCollision(Bullet* bullet);

	void CheckSpaceshipWaspCollisions();
	void CheckSpaceshipBeetleCollision();
	void CheckSpaceshipAsteroidCollisions();

	void UpdateAsteroids(float deltaSeconds);
	void UpdateBeetles(float deltaSeconds);
	void UpdateWasps(float deltaSeconds);

	void RenderWasps() const;
	void RenderBeetles() const;
	void RenderAsteroids() const;

	void SpawnAsteroids();
	void SpawnWasps();
	void SpawnBeetles();

private:
	Asteroid* m_asteroids = nullptr;
	int m_numAsteroids = 0;
	
	Wasp* m_wasps = nullptr;
	int m_numWasps = 0;
	
	Beetle* m_beetles = nullptr;
	int m_numBeetles = 0;
	
	int m_enemyCount = 0;

	SoundPlaybackID m_bgMusicPlaybackId = MISSING_SOUND_ID;
	SpaceShip* m_spaceShip = nullptr;
	Game* m_game = nullptr;
};