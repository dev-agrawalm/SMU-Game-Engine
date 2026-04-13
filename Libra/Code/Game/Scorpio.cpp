#include "Scorpio.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Map.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

Scorpio::Scorpio()
{
}


Scorpio::~Scorpio()
{
}


Scorpio::Scorpio(Vec2 const& position, float orientationDegrees, EntityFaction faction, EntityType type, Map* map)
	: Entity(position, orientationDegrees, faction, type, map)
{
	m_health						= g_gameConfigBlackboard.GetValue("scorpioHealth", m_health);
	m_cosmeticRadius				= g_gameConfigBlackboard.GetValue("scorpioCosmeticRadius", m_cosmeticRadius);
	m_physicsRadius					= g_gameConfigBlackboard.GetValue("scorpioPhysicsRadius", m_physicsRadius);
	m_turretLocalPosition			= g_gameConfigBlackboard.GetValue("scorpioTurretLocalPosition", m_turretLocalPosition);
	m_turretAngularVelocity			= g_gameConfigBlackboard.GetValue("scorpioTurretAngularVelocity", m_turretAngularVelocity);
	m_turretLocalOrientationDegrees = g_gameConfigBlackboard.GetValue("scorpioTurretLocalOrientationDegrees", m_turretLocalOrientationDegrees);
	m_turretLength					= g_gameConfigBlackboard.GetValue("scorpioTurretLength", m_turretLength);
	m_turretCooldownTimeSeconds		= g_gameConfigBlackboard.GetValue("scorpioTurretCooldownTimeSeconds", m_turretCooldownTimeSeconds);
	m_turretCooldownTimer			= 0.0f;
	m_isPushedByEntities			= false;
	m_targetPosition				= m_position;
	m_maxHealth						= m_health;
}


void Scorpio::Update(float deltaSeconds)
{
	if (m_isDead)
		return;

	m_turretCooldownTimer -= deltaSeconds;
	if (CheckAndFollowTargetInSight(deltaSeconds))
	{
		return;
	}

	float turretWorldOrientationDegrees;
	turretWorldOrientationDegrees	= m_orientationDegrees + m_turretLocalOrientationDegrees;
	turretWorldOrientationDegrees	+= m_turretAngularVelocity * deltaSeconds;
	m_turretLocalOrientationDegrees	= turretWorldOrientationDegrees - m_orientationDegrees;
}


bool Scorpio::CheckAndFollowTargetInSight(float deltaSeconds)
{
	EntityFaction targetEntityFaction	= (EntityFaction) ((ENTITY_FACTION_GOOD + ENTITY_FACTION_EVIL) - m_faction);
	EntityList targetEntities			= m_map->GetNonBulletEntitiesByFaction(targetEntityFaction);
	for (int targetIndex = 0; targetIndex < targetEntities.size(); targetIndex++)
	{
		m_targetEntity	= targetEntities[targetIndex];
		if (m_targetEntity)
		{
			Vec2 targetPosition		= m_targetEntity->GetPosition();
			float lineOfSightRange	= g_gameConfigBlackboard.GetValue("enemyLineOfSightRange", 0.0f);
			if (m_targetEntity && m_targetEntity->IsAlive() && m_map->HasLineOfSight(m_position, targetPosition, lineOfSightRange))
			{
				m_targetPosition = targetPosition;
				MoveTurretTowardsTarget(deltaSeconds);
				CheckAndFireTurretAtTarget();
				return true;
			}
		}
	}

	return false;
}


void Scorpio::MoveTurretTowardsTarget(float deltaSeconds)
{
	float turretWorldOrientationDegrees;
	Vec2 targetDir						= (m_targetPosition - m_position).GetNormalized();
	float targetOrientationDegrees		= targetDir.GetOrientationDegrees();
	turretWorldOrientationDegrees		= m_orientationDegrees + m_turretLocalOrientationDegrees;
	turretWorldOrientationDegrees		= GetTurnedTowardDegrees(turretWorldOrientationDegrees,
																 targetOrientationDegrees, m_turretAngularVelocity * deltaSeconds);
	m_turretLocalOrientationDegrees		= turretWorldOrientationDegrees - m_orientationDegrees;
}


void Scorpio::CheckAndFireTurretAtTarget()
{
	float turretWorldOrientationDegrees;
	Vec2 targetDir					= (m_targetPosition - m_position).GetNormalized();
	float targetOrientationDegrees	= targetDir.GetOrientationDegrees();
	turretWorldOrientationDegrees	= m_orientationDegrees + m_turretLocalOrientationDegrees;
	float angularDisplaceFromTarget = GetShortestAngularDisplacementDegrees(turretWorldOrientationDegrees, targetOrientationDegrees);
	if (angularDisplaceFromTarget <= 5.0f && angularDisplaceFromTarget >= -5.0f)
	{
		if (m_turretCooldownTimer <= 0.0f)
		{
			m_turretCooldownTimer			= m_turretCooldownTimeSeconds;
			float turretWorldOrientation	= m_turretLocalOrientationDegrees + m_orientationDegrees;
			Vec2 turretWorldPosition		= m_position + m_turretLocalPosition;
			Vec2 turretMuzzlePosition		= turretWorldPosition + Vec2::MakeFromPolarDegrees(turretWorldOrientation, m_turretLength);
			m_map->SpawnEntityOfType(ENTITY_TYPE_EVIL_BULLET, turretMuzzlePosition, turretWorldOrientation);
			
			SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
			g_audioSystem->StartSound(sfxId, false, 0.05f);
			static int s_bulletFireCount = 0;
			if (s_bulletFireCount % 3 == 0)
				g_audioSystem->StartSound(sfxId, false, 0.05f);
			s_bulletFireCount++;
		}
	}
}


void Scorpio::Render() const
{
	if (g_debugMode)
	{
		DebugRender();

		Vec2 lineToTarget = m_targetPosition - m_position;
		if (lineToTarget.GetLength() > 0.0f)
		{
			DrawLine(m_position, lineToTarget, Rgba8::BLUE, 0.002f);
		}
	}
	
	if (m_isDead)
		return;
	
	AABB2 localSpaceBoundingBox = AABB2(-0.5f, -0.5, 0.5f, 0.5f);

	std::vector<Vertex_PCU> scorpioBaseVertexes;
	Texture* tankTexture = g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTurretBase.png");
	AddVertsForAABB2ToVector(scorpioBaseVertexes, localSpaceBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) scorpioBaseVertexes.size(), scorpioBaseVertexes.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(0, tankTexture);
	g_theRenderer->DrawVertexArray((int) scorpioBaseVertexes.size(), scorpioBaseVertexes.data());


	std::vector<Vertex_PCU> turretVertexes;
	Texture* turretTexture			= g_theRenderer->CreateOrGetTexture("Data/Images/EnemyCannon.png");
	Vec2 turretWorldPosition		= m_position + m_turretLocalPosition;
	float turretWorldOrientation	= m_turretLocalOrientationDegrees + m_orientationDegrees;
	AddVertsForAABB2ToVector(turretVertexes, localSpaceBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) turretVertexes.size(), turretVertexes.data(), 1.0f, turretWorldOrientation, turretWorldPosition);
	g_theRenderer->BindTexture(0, turretTexture);
	g_theRenderer->DrawVertexArray((int) turretVertexes.size(), turretVertexes.data());	
}


void Scorpio::Die()
{
	SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	g_audioSystem->StartSound(sfxId, false, 0.05f);
	m_isDead = true;

	float randomOrientation = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
	m_map->SpawnEntityOfType(ENTITY_TYPE_TANK_EXPLOSION, m_position, randomOrientation);
}


void Scorpio::TakeDamage(int damage)
{
	SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	g_audioSystem->StartSound(sfxId, false, 0.05f);

	m_health -= damage;
	if (m_health <= 0)
	{
		Die();
	}
}
