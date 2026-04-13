#include "Game/Capricorn.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Bullet.hpp"

Capricorn::Capricorn()
{
}


Capricorn::~Capricorn()
{
}


Capricorn::Capricorn(Vec2 const& position, float orientationDegrees, EntityFaction faction, EntityType type, Map* map)
	: Entity(position, orientationDegrees, faction, type, map)
{
	m_targetOrientationDegrees	 = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
	m_targetPosition			 = m_position;
	m_cosmeticRadius			 = g_gameConfigBlackboard.GetValue("capricornCosmeticRadius",			 m_cosmeticRadius);
	m_physicsRadius				 = g_gameConfigBlackboard.GetValue("capricornPhysicsRadius",			 m_physicsRadius);
	m_speed						 = g_gameConfigBlackboard.GetValue("capricornSpeed",					 m_speed);
	m_angularVelocity			 = g_gameConfigBlackboard.GetValue("capricornTankAngularVelocity",		 m_angularVelocity);
	m_turretLength				 = g_gameConfigBlackboard.GetValue("capricornTurretLength",				 m_turretLength);
	m_turrentCooldownTimeSeconds = g_gameConfigBlackboard.GetValue("capricornTurretCooldownTimeSeconds", m_turrentCooldownTimeSeconds);
	m_health					 = g_gameConfigBlackboard.GetValue("capricornHealth",					 m_health);
	m_maxWanderingTime			 = g_gameConfigBlackboard.GetValue("capricornWanderingTime",			 m_maxWanderingTime);
	m_turretCooldownTimer		 = 0.0f;
	m_wanderingTime				 = 0.0f;
	m_canSwim					 = true;
	m_maxHealth					 = m_health;
}


void Capricorn::Update(float deltaSeconds)
{
	if (m_isDead)
		return;

	m_turretCooldownTimer -= deltaSeconds;
	if (CheckAndFollowTargetInSight(deltaSeconds))
	{
		return;
	}

	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, m_targetOrientationDegrees,
												  m_angularVelocity * deltaSeconds);
	m_velocity		= m_speed * GetForwardNormal();
	m_position		+= m_velocity * deltaSeconds;
	m_wanderingTime += deltaSeconds;
	if (m_wanderingTime >= m_maxWanderingTime)
	{
		m_wanderingTime = 0.0f;
		m_targetOrientationDegrees = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
	}
}


bool Capricorn::CheckAndFollowTargetInSight(float deltaSeconds)
{
	EntityFaction targetEntityFaction = (EntityFaction) ((ENTITY_FACTION_GOOD + ENTITY_FACTION_EVIL) - m_faction);
	EntityList targetEntities = m_map->GetNonBulletEntitiesByFaction(targetEntityFaction);
	for (int targetIndex = 0; targetIndex < targetEntities.size(); targetIndex++)
	{
		m_targetEntity			= targetEntities[targetIndex];
		Vec2 targetPosition		= m_targetEntity->GetPosition();
		float lineOfSightRange	= g_gameConfigBlackboard.GetValue("enemyLineOfSightRange", 0.0f);
		if (m_targetEntity->IsAlive() && m_map->HasLineOfSight(m_position, targetPosition, lineOfSightRange))
		{
			m_targetPosition = targetPosition;
			CheckAndFireTurretAtTargetInRange();
			break;
		}
	}

	float distanceToTarget = (m_targetPosition - m_position).GetLength();
	if (m_targetEntity && m_targetEntity->IsAlive() && distanceToTarget > 0.1f)
	{
		MoveTowardsTarget(deltaSeconds);
		return true;
	}

	m_targetPosition = m_position;
	return false;
}


void Capricorn::MoveTowardsTarget(float deltaSeconds)
{
	Vec2 targetDir					= (m_targetPosition - m_position).GetNormalized();
	float targetOrientationDegrees	= targetDir.GetOrientationDegrees();
	m_orientationDegrees			= GetTurnedTowardDegrees(m_orientationDegrees, targetOrientationDegrees, m_angularVelocity * deltaSeconds);
	float angularDisplaceFromTarget = GetShortestAngularDisplacementDegrees(m_orientationDegrees, targetOrientationDegrees);
	if (angularDisplaceFromTarget <= 45.0f && angularDisplaceFromTarget >= -45.0f)
	{
		m_velocity = m_speed * GetForwardNormal();
		m_position += m_velocity * deltaSeconds;
	}
}


void Capricorn::CheckAndFireTurretAtTargetInRange()
{
	Vec2 targetDir					= (m_targetPosition - m_position).GetNormalized();
	float targetOrientationDegrees	= targetDir.GetOrientationDegrees();
	float angularDisplaceFromTarget = GetShortestAngularDisplacementDegrees(m_orientationDegrees, targetOrientationDegrees);
	if (angularDisplaceFromTarget <= 5.0f && angularDisplaceFromTarget >= -5.0f)
	{
		if (m_turretCooldownTimer <= 0.0f)
		{
			m_turretCooldownTimer			= m_turrentCooldownTimeSeconds;
			float turretWorldOrientation	= m_orientationDegrees;
			Vec2 turretWorldPosition		= m_position + g_gameConfigBlackboard.GetValue("capricornTurretLocalPosition", Vec2::ZERO);
			Vec2 turretMuzzlePosition		= turretWorldPosition + Vec2::MakeFromPolarDegrees(turretWorldOrientation, m_turretLength);
			Entity* firedMissile			= m_map->SpawnEntityOfType(ENTITY_TYPE_EVIL_GUIDED_MISSILE, turretMuzzlePosition, turretWorldOrientation);
			Bullet* guidedMissle			= dynamic_cast<Bullet*>(firedMissile);
			guidedMissle->SetTargetEntity(m_targetEntity);

			SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
			g_audioSystem->StartSound(sfxId, false, 0.05f);
		}
	}
}


void Capricorn::Render() const
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

	std::vector<Vertex_PCU> capricornVertexes;
	Texture* tankTexture = g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank3.png");
	AddVertsForAABB2ToVector(capricornVertexes, localSpaceBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) capricornVertexes.size(), capricornVertexes.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(0, tankTexture);
	g_theRenderer->DrawVertexArray((int) capricornVertexes.size(), capricornVertexes.data());
}


void Capricorn::Die()
{
	SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	g_audioSystem->StartSound(sfxId, false, 0.05f);
	m_isDead = true;
	m_isGarbage = true;

	float randomOrientation = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
	m_map->SpawnEntityOfType(ENTITY_TYPE_TANK_EXPLOSION, m_position, randomOrientation);
}


void Capricorn::TakeDamage(int damage)
{
	SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	g_audioSystem->StartSound(sfxId, false, 0.05f);

	m_health -= damage;
	if (m_health <= 0)
	{
		Die();
	}
}