#include "Game/EnemyWave.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Wasp.hpp"
#include "Game/Beetle.hpp"
#include "Game/SpaceShip.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Bullet.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Bomb.hpp"
#include "Engine/Audio/AudioSystem.hpp"

EnemyWave::EnemyWave()
{

}

EnemyWave::EnemyWave(int const numAsteroids, int const numWasps, int const numBeetles)
{
	m_numAsteroids = numAsteroids;
	m_numBeetles = numBeetles;
	m_numWasps = numWasps;
}

EnemyWave::~EnemyWave()
{

}

void EnemyWave::Init( SpaceShip* spaceShip, Game* game, char const* bgMusicString /*= ""*/)
{
	SoundID enemyWaveMusic = g_audioSystem->CreateOrGetSound(bgMusicString);
	m_bgMusicPlaybackId = g_audioSystem->StartSound(enemyWaveMusic, true, 1.0f, 0.0f, 1.0f);

	m_spaceShip = spaceShip;
	m_game = game;

	m_asteroids = new Asteroid[m_numAsteroids];
	m_beetles = new Beetle[m_numBeetles];
	m_wasps = new Wasp[m_numWasps];
	m_enemyCount = m_numWasps + m_numBeetles + m_numAsteroids;

	SpawnEnemies();
}

void EnemyWave::Deinit()
{
	g_audioSystem->StopSound(m_bgMusicPlaybackId);

	delete[] m_asteroids;
	m_asteroids = nullptr;

	delete[] m_wasps;
	m_wasps = nullptr;

	delete[] m_beetles;
	m_beetles = nullptr;
}

void EnemyWave::Update(float deltaSeconds)
{
	UpdateAsteroids(deltaSeconds);
	UpdateBeetles(deltaSeconds);
	UpdateWasps(deltaSeconds);
}

void EnemyWave::Render() const
{
	RenderAsteroids();
	RenderBeetles();
	RenderWasps();
}

void EnemyWave::DebugRenderEnemyDetails() const
{
	DebugRenderAsteroidDetails();
	DebugRenderWaspDetails();
	DebugRenderBeetleDetails();
}

bool EnemyWave::IsWaveOver() const
{
	if (m_enemyCount == 0)
	{
		return true;
	}

	return false;
}

void EnemyWave::SpawnEnemies()
{
	SpawnAsteroids();
	SpawnBeetles();
	SpawnWasps();
}

void EnemyWave::CheckSpaceshipEnemyCollisions()
{
	CheckSpaceshipAsteroidCollisions();
	CheckSpaceshipBeetleCollision();
	CheckSpaceshipWaspCollisions();
}

void EnemyWave::CheckBulletEnemyCollisions(Bullet* bullet)
{
	CheckBulletAsteroidCollisions(bullet);
	CheckBulletBeetleCollision(bullet);
	CheckBulletWaspCollisions(bullet);
}

void EnemyWave::CheckBombEnemyCollisions(Bomb* bomb)
{
	for (int asteroidIndex = 0; asteroidIndex < m_numAsteroids; asteroidIndex++)
	{
		Asteroid& asteroid = m_asteroids[asteroidIndex];
		if (asteroid.IsAlive() && bomb->IsExploded())
		{
			if(DoDiscsOverlap2D(asteroid.GetPosition(), asteroid.GetPhysicsRadius(), bomb->GetPosition(), bomb->GetExplosionRadius()))
			{
				int damage = bomb->GetDamage();
				asteroid.TakeDamage(damage);
				if (!asteroid.IsAlive())
				{
					m_game->SpawnDebris(&asteroid, DEBRIS_COUNT_ASTEROID, asteroid.GetDebrisColor());
					m_enemyCount--;
				}
			}
		}
	}

	for (int beetleIndex = 0; beetleIndex < m_numBeetles; beetleIndex++)
	{
		Beetle& beetle = m_beetles[beetleIndex];
		if (beetle.IsAlive() && bomb->IsExploded())
		{
			if (DoDiscsOverlap2D(beetle.GetPosition(), beetle.GetPhysicsRadius(), bomb->GetPosition(), bomb->GetExplosionRadius()))
			{
				int damage = bomb->GetDamage();
				beetle.TakeDamage(damage);
				if (!beetle.IsAlive())
				{
					m_game->SpawnDebris(&beetle, DEBRIS_COUNT_ASTEROID, beetle.GetDebrisColor());
					m_enemyCount--;
				}
			}
		}
	}

	for (int waspIndex = 0; waspIndex < m_numWasps; waspIndex++)
	{
		Wasp& wasp = m_wasps[waspIndex];
		if (wasp.IsAlive() && bomb->IsExploded())
		{
			if (DoDiscsOverlap2D(wasp.GetPosition(), wasp.GetPhysicsRadius(), bomb->GetPosition(), bomb->GetExplosionRadius()))
			{
				int damage = bomb->GetDamage();
				wasp.TakeDamage(damage);
				if (!wasp.IsAlive())
				{
					m_game->SpawnDebris(&wasp, DEBRIS_COUNT_ASTEROID, wasp.GetDebrisColor());
					m_enemyCount--;
				}
			}
		}
	}
}

SoundPlaybackID EnemyWave::GetBgMusicPlaybackId() const
{
	return m_bgMusicPlaybackId;
}

void EnemyWave::DebugRenderWaspDetails() const
{
	for (int index = 0; index < m_numWasps; index++)
	{
		Wasp& wasp = m_wasps[index];
		if (wasp.IsAlive())
		{
			wasp.DebugRender();
		}
	}
}

void EnemyWave::DebugRenderAsteroidDetails() const
{
	for (int index = 0; index < m_numAsteroids; index++)
	{
		Asteroid& asteroid = m_asteroids[index];
		if (asteroid.IsAlive())
		{
			asteroid.DebugRender();
		}
	}
}

void EnemyWave::DebugRenderBeetleDetails() const
{
	for (int index = 0; index < m_numBeetles; index++)
	{
		Beetle& beetle = m_beetles[index];
		if (beetle.IsAlive())
		{
			beetle.DebugRender();
		}
	}
}

void EnemyWave::DebugRenderEnemySpaceshipDistance() const
{
	if (!m_spaceShip->IsAlive())
		return;

	DebugRenderWaspSpaceshipDistance();
	DebugRenderBeetleSpaceshipDistance();
	DebugRenderAsteroidSpaceshipDistance();
}

void EnemyWave::DebugRenderWaspSpaceshipDistance() const
{
	const Rgba8 DARK_GREY(50, 50, 50, 255);
	Vec2 spaceshipPos = m_spaceShip->GetPosition();

	for (int index = 0; index < m_numWasps; index++)
	{
		Wasp& wasp = m_wasps[index];

		if (wasp.IsAlive())
		{
			Vec2 waspPos = wasp.GetPosition();
			Vec2 distanceVec = spaceshipPos - waspPos;

			DebugDrawLine(waspPos, distanceVec, DEBUG_LINE_WIDTH, DARK_GREY);
		}
	}
}

void EnemyWave::DebugRenderBeetleSpaceshipDistance() const
{
	const Rgba8 DARK_GREY(50, 50, 50, 255);
	Vec2 spaceshipPos = m_spaceShip->GetPosition();

	for (int index = 0; index < m_numBeetles; index++)
	{
		Beetle& beetle = m_beetles[index];

		if (beetle.IsAlive())
		{
			Vec2 beetlePos = beetle.GetPosition();
			Vec2 distanceVec = spaceshipPos - beetlePos;

			DebugDrawLine(beetlePos, distanceVec, DEBUG_LINE_WIDTH, DARK_GREY);
		}
	}
}

void EnemyWave::DebugRenderAsteroidSpaceshipDistance() const
{
	const Rgba8 DARK_GREY(50, 50, 50, 255);
	Vec2 spaceshipPos = m_spaceShip->GetPosition();

	for (int index = 0; index < m_numAsteroids; index++)
	{
		Asteroid& asteroid = m_asteroids[index];

		if (asteroid.IsAlive())
		{
			Vec2 asteroidPos = asteroid.GetPosition();
			Vec2 distanceVec = spaceshipPos - asteroidPos;

			DebugDrawLine(asteroidPos, distanceVec, DEBUG_LINE_WIDTH, DARK_GREY);
		}
	}
}

void EnemyWave::CheckBulletWaspCollisions(Bullet* bullet)
{
	for (int index = 0; index < m_numWasps; index++)
	{
		Wasp& wasp = m_wasps[index];
		if (m_game->AreEntitiesColliding(&wasp, bullet))
		{
			int damage = bullet->GetDamage();
			wasp.TakeDamage(damage);
			bullet->Die();
			m_game->SpawnDebris(bullet, DEBRIS_COUNT_BULLET, bullet->GetDebrisColor());
			size_t bulletHitSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/PlayerHit.wav");
			float sfxSpeed = g_rng->GetRandomFloatInRange(0.9f, 1.1f);
			g_audioSystem->StartSound(bulletHitSfxId, false, 0.5f, 0.0f, sfxSpeed);

			if (!wasp.IsAlive())
			{
				m_game->SpawnDebris(&wasp, DEBRIS_COUNT_WASP, wasp.GetDebrisColor());
				m_enemyCount--;
			}
		}
	}
}

void EnemyWave::CheckBulletAsteroidCollisions(Bullet* bullet)
{
	for (int index = 0; index < m_numAsteroids; index++)
	{
		Asteroid& asteroid = m_asteroids[index];
		if (m_game->AreEntitiesColliding(&asteroid, bullet))
		{
			int damage = bullet->GetDamage();
			asteroid.TakeDamage(damage);
			bullet->Die();
			m_game->SpawnDebris(bullet, DEBRIS_COUNT_BULLET, bullet->GetDebrisColor());
			size_t bulletHitSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/PlayerHit.wav");
			float sfxSpeed = g_rng->GetRandomFloatInRange(0.9f, 1.1f);
			g_audioSystem->StartSound(bulletHitSfxId, false, 0.5f, 0.0f, sfxSpeed);

			if (!asteroid.IsAlive())
			{
				m_game->SpawnDebris(&asteroid, DEBRIS_COUNT_ASTEROID, asteroid.GetDebrisColor());
				m_enemyCount--;
			}
		}
	}
}

void EnemyWave::CheckBulletBeetleCollision(Bullet* bullet)
{
	for (int index = 0; index < m_numBeetles; index++)
	{
		Entity& beetle = m_beetles[index];
		if (m_game->AreEntitiesColliding(&beetle, bullet))
		{
			int damage = bullet->GetDamage();
			beetle.TakeDamage(damage);
			bullet->Die();
			m_game->SpawnDebris(bullet, DEBRIS_COUNT_BULLET, bullet->GetDebrisColor());
			size_t bulletHitSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/PlayerHit.wav");
			float sfxSpeed = g_rng->GetRandomFloatInRange(0.9f, 1.1f);
			g_audioSystem->StartSound(bulletHitSfxId, false, 0.5f, 0.0f, sfxSpeed);

			if (!beetle.IsAlive())
			{
				m_game->SpawnDebris(&beetle, DEBRIS_COUNT_BEETLE, beetle.GetDebrisColor());
				m_enemyCount--;
			}
		}
	}
}

void EnemyWave::CheckSpaceshipWaspCollisions()
{
	for (int index = 0; index < m_numWasps; index++)
	{
		Entity* wasp = &m_wasps[index];
		if (m_game->AreEntitiesColliding(wasp, m_spaceShip))
		{
			m_spaceShip->Die();
			if (!m_spaceShip->IsAlive())
			{
				m_game->SpawnDebris(m_spaceShip, DEBRIS_COUNT_SPACESHIP, m_spaceShip->GetDebrisColor());
			}
		}
	}
}

void EnemyWave::CheckSpaceshipBeetleCollision()
{
	for (int index = 0; index < m_numBeetles; index++)
	{
		Entity* beetle = &m_beetles[index];
		if (m_game->AreEntitiesColliding(beetle, m_spaceShip))
		{
			m_spaceShip->Die();
			if (!m_spaceShip->IsAlive())
			{
				m_game->SpawnDebris(m_spaceShip, DEBRIS_COUNT_SPACESHIP, m_spaceShip->GetDebrisColor());
			}
		}
	}
}

void EnemyWave::CheckSpaceshipAsteroidCollisions()
{
	for (int index = 0; index < m_numAsteroids; index++)
	{
		Entity* asteroid = &m_asteroids[index];
		if (m_game->AreEntitiesColliding(asteroid, m_spaceShip))
		{
			m_spaceShip->Die();
			if (!m_spaceShip->IsAlive())
			{
				m_game->SpawnDebris(m_spaceShip, DEBRIS_COUNT_SPACESHIP, m_spaceShip->GetDebrisColor());
			}
		}
	}
}

void EnemyWave::UpdateAsteroids(float deltaSeconds)
{
	for (int index = 0; index < m_numAsteroids; index++)
	{
		m_asteroids[index].Update(deltaSeconds);
	}
}

void EnemyWave::UpdateBeetles(float deltaSeconds)
{
	for (int index = 0; index < m_numBeetles; index++)
	{
		m_beetles[index].Update(deltaSeconds);
	}
}

void EnemyWave::UpdateWasps(float deltaSeconds)
{
	for (int index = 0; index < m_numWasps; index++)
	{
		m_wasps[index].Update(deltaSeconds);
	}
}

void EnemyWave::RenderWasps() const
{
	for (int index = 0; index < m_numWasps; index++)
	{
		m_wasps[index].Render();
	}
}

void EnemyWave::RenderBeetles() const
{
	for (int index = 0; index < m_numBeetles; index++)
	{
		m_beetles[index].Render();
	}
}

void EnemyWave::RenderAsteroids() const
{
	for (int index = 0; index < m_numAsteroids; index++)
	{
		m_asteroids[index].Render();
	}
}

void EnemyWave::SpawnAsteroids()
{
	for (int index = 0; index < m_numAsteroids; index++)
	{
		Vec2 position = m_game->GetLocationOffScreen(ASTEROID_COSMETIC_RADIUS);

		float theta = g_rng->GetRandomFloatZeroToOne() * 360.0f;
		Vec2 forwardDirection = Vec2::MakeFromPolarDegrees(theta);
		Vec2 velocity = Vec2::ZERO;/*forwardDirection * ASTEROID_SPEED*/;

		m_asteroids[index].Init(m_game, position, theta, velocity);
	}
}

void EnemyWave::SpawnWasps()
{
	for (int index = 0; index < m_numWasps; index++)
	{
		Vec2 position = m_game->GetLocationOffScreen(WASP_COSMETIC_RADIUS);
		m_wasps[index].Init(m_game, position, m_spaceShip);
	}
}

void EnemyWave::SpawnBeetles()
{
	for (int index = 0; index < m_numBeetles; index++)
	{
		Vec2 position = m_game->GetLocationOffScreen(BEETLE_COSMETIC_RADIUS);
		m_beetles[index].Init(m_game, position, m_spaceShip);
	}
}