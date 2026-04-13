#include "Game/Sagittarius.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Tile.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

Sagittarius::Sagittarius(Vec2 const& position, float orientationDegrees, EntityFaction faction, EntityType type, Map* map)
	: Entity(position, orientationDegrees, faction, type, map)
{
	m_targetOrientationDegrees		= g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
	m_targetEntityPosition			= m_position;
	m_cosmeticRadius				= g_gameConfigBlackboard.GetValue("sagittariusCosmeticRadius",					m_cosmeticRadius);
	m_physicsRadius					= g_gameConfigBlackboard.GetValue("sagittariusPhysicsRadius",					m_physicsRadius);
	m_health						= g_gameConfigBlackboard.GetValue("sagittariusHealth",							m_health);
	m_speed							= g_gameConfigBlackboard.GetValue("sagittariusSpeed",							m_speed);
	m_angularVelocity				= g_gameConfigBlackboard.GetValue("sagittariusTankAngularVelocity",				m_angularVelocity);
	m_maxWanderingTime				= g_gameConfigBlackboard.GetValue("sagittariusWanderingTime",					m_maxWanderingTime);
	m_turretLength					= g_gameConfigBlackboard.GetValue("sagittariusTurretLength",					m_turretLength);
	m_turretLocalPosition			= g_gameConfigBlackboard.GetValue("sagittariusTurretLocalPosition",				m_turretLocalPosition);
	m_turretLocalOrientationDegrees = g_gameConfigBlackboard.GetValue("sagittariusTurretLocalOrientationDegrees",	m_turretLocalOrientationDegrees);
	m_turretAngularVelocity			= g_gameConfigBlackboard.GetValue("sagittariusTurretAngularVelocity",			m_turretAngularVelocity);
	m_turretCooldownTimeSeconds		= g_gameConfigBlackboard.GetValue("sagittariusTurretCooldownTimeSeconds",		m_turretCooldownTimeSeconds);
	m_turretCooldownTimer			= 0.0f;
	m_maxHealth						= m_health;
}


Sagittarius::~Sagittarius()
{
}


bool Sagittarius::CheckAndFollowTargetInSight(float deltaSeconds)
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
			m_targetEntityPosition = targetPosition;
			CheckAndFireTurretAtTargetInRange(deltaSeconds);
			break;
		}
	}

	float distanceToTarget = (m_targetEntityPosition - m_position).GetLength();
	if (m_targetEntity && m_targetEntity->IsAlive() && distanceToTarget > 0.1f)
	{
		MoveTowardsTarget(deltaSeconds);
		return true;
	}

	m_targetEntityPosition = m_position;
	return false;
}


void Sagittarius::MoveTowardsTarget(float deltaSeconds)
{
	float distanceToTarget	= (m_targetEntityPosition - m_position).GetLength();
	float distanceFromTarget = g_gameConfigBlackboard.GetValue("sagittariusDistanceFromTarget", 0.0f);
	if (distanceToTarget > distanceFromTarget)
	{
		Vec2 targetDir					= (m_targetEntityPosition - m_position).GetNormalized();
		float targetOrientationDegrees	= targetDir.GetOrientationDegrees();
		m_orientationDegrees			= GetTurnedTowardDegrees(m_orientationDegrees, targetOrientationDegrees, m_angularVelocity * deltaSeconds);
		float angularDisplaceFromTarget = GetShortestAngularDisplacementDegrees(m_orientationDegrees, targetOrientationDegrees);
		
		if (angularDisplaceFromTarget <= 45.0f && angularDisplaceFromTarget >= -45.0f)
		{
			m_velocity = m_speed * GetForwardNormal();
			m_position += m_velocity * deltaSeconds;
		}
	}
}


void Sagittarius::CheckAndFireTurretAtTargetInRange(float deltaSeconds)
{
	float turretWorldOrientationDegrees;
	Vec2 targetPosition				= m_targetEntityPosition + m_targetEntity->GetForwardNormal() * (m_targetEntity->GetPhysicsRadius() + 0.3f);
	Vec2 targetDir					= (targetPosition - m_position).GetNormalized();
	float targetOrientationDegrees	= targetDir.GetOrientationDegrees();
	turretWorldOrientationDegrees	= m_orientationDegrees + m_turretLocalOrientationDegrees;
	turretWorldOrientationDegrees	= GetTurnedTowardDegrees(turretWorldOrientationDegrees, targetOrientationDegrees, m_turretAngularVelocity * deltaSeconds);
	m_turretLocalOrientationDegrees = turretWorldOrientationDegrees - m_orientationDegrees;
	float angularDisplaceFromTarget = GetShortestAngularDisplacementDegrees(turretWorldOrientationDegrees, targetOrientationDegrees);
	if (angularDisplaceFromTarget <= 0.01f && angularDisplaceFromTarget >= -0.01f)
	{
		if (m_turretCooldownTimer <= 0.0f)
		{
			m_turretCooldownTimer			= m_turretCooldownTimeSeconds;
			Vec2 turretWorldPosition		= m_position + g_gameConfigBlackboard.GetValue("sagittariusTurretLocalPosition", Vec2::ZERO);
			Vec2 turretMuzzlePosition		= turretWorldPosition + Vec2::MakeFromPolarDegrees(turretWorldOrientationDegrees, m_turretLength);
			m_map->SpawnEntityOfType(ENTITY_TYPE_EVIL_BULLET, turretMuzzlePosition, turretWorldOrientationDegrees);

			static int s_bulletFireCount = 0;
			SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
			if (s_bulletFireCount % 7 == 0)
				g_audioSystem->StartSound(sfxId, false, 0.05f);
			s_bulletFireCount++;
		}
	}
}


void Sagittarius::Update(float deltaSeconds)
{
	if (m_isDead)
		return;

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


void Sagittarius::Render() const
{
	if (g_debugMode)
	{
		DebugRender();

		Vec2 lineToTarget = m_targetEntityPosition - m_position;
		if (lineToTarget.GetLength() > 0.0f)
		{
			DrawLine(m_position, lineToTarget, Rgba8::BLUE, 0.002f);
		}
	}

	if (m_isDead)
		return;

	AABB2 localSpaceBoundingBox = AABB2(-0.5f, -0.5, 0.5f, 0.5f);

	std::vector<Vertex_PCU> tankVertexes;
	Texture* tankTexture = g_theRenderer->CreateOrGetTexture("Data/Images/EnemyTank0.png");
	AddVertsForAABB2ToVector(tankVertexes, localSpaceBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) tankVertexes.size(), tankVertexes.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(0, tankTexture);
	g_theRenderer->DrawVertexArray((int) tankVertexes.size(), tankVertexes.data());

	std::vector<Vertex_PCU> turretVertexes;
	Texture* turretTexture		 = g_theRenderer->CreateOrGetTexture("Data/Images/EnemyGatling.png");
	Vec2 turretWorldPosition	 = m_position + m_turretLocalPosition;
	float turretWorldOrientation = m_turretLocalOrientationDegrees + m_orientationDegrees;
	AddVertsForAABB2ToVector(turretVertexes, localSpaceBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) turretVertexes.size(), turretVertexes.data(), 1.0f, turretWorldOrientation, turretWorldPosition);
	g_theRenderer->BindTexture(0, turretTexture);
	g_theRenderer->DrawVertexArray((int) turretVertexes.size(), turretVertexes.data());
}


void Sagittarius::Die()
{
	SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	g_audioSystem->StartSound(sfxId, false, 0.05f);
	m_isDead	= true;
	m_isGarbage = true;

	float randomOrientation = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
	m_map->SpawnEntityOfType(ENTITY_TYPE_TANK_EXPLOSION, m_position, randomOrientation);
}


void Sagittarius::TakeDamage(int damage)
{
	SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	g_audioSystem->StartSound(sfxId, false, 0.05f);

	m_health -= damage;
	if (m_health <= 0)
	{
		Die();
	}
}
