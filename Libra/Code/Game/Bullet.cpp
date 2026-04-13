#include "Game/Bullet.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Aries.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include<vector>
#include<cmath>

Bullet::Bullet()
{
}


Bullet::~Bullet()
{
}


Bullet::Bullet(Vec2 const& position, float orientationDegrees, EntityFaction faction, EntityType type, Map* map)
	: Entity(position, orientationDegrees, faction, type, map)
{
	switch (type)
	{
		case ENTITY_TYPE_GOOD_BULLET:
		{
			m_health		= g_gameConfigBlackboard.GetValue("goodBulletHealth", m_health);
			m_damage		= g_gameConfigBlackboard.GetValue("goodBulletDamage", m_damage);
			m_texturePath	= g_gameConfigBlackboard.GetValue("goodBulletTexturePath", "");
			m_velocity		= g_gameConfigBlackboard.GetValue("goodBulletSpeed", 0.0f) * GetForwardNormal();
			break;
		}
		case ENTITY_TYPE_EVIL_BULLET:
		{
			m_health		= g_gameConfigBlackboard.GetValue("evilBulletHealth", m_health);
			m_damage		= g_gameConfigBlackboard.GetValue("evilBulletDamage", m_damage);
			m_texturePath	= g_gameConfigBlackboard.GetValue("evilBulletTexturePath", "");
			m_velocity		= g_gameConfigBlackboard.GetValue("evilBulletSpeed", 0.0f) * GetForwardNormal();
			break;
		}
		case ENTITY_TYPE_EVIL_GUIDED_MISSILE:
		{
			m_health			= g_gameConfigBlackboard.GetValue("evilGuidedMissileHealth", m_health);
			m_damage			= g_gameConfigBlackboard.GetValue("evilGuidedMissileDamage", m_damage);
			m_texturePath		= g_gameConfigBlackboard.GetValue("evilGuidedMissileTexturePath", "");
			m_velocity			= g_gameConfigBlackboard.GetValue("evilGuidedMissileSpeed", 0.0f) * GetForwardNormal();
			m_angularVelocity	= g_gameConfigBlackboard.GetValue("evilGuidedMissileAngularVelocity", 0.0f);
			break;
		}
		default:
			ERROR_RECOVERABLE("Invalid entity type given for bullet");
			break;
	}

	CheckAndCorrectBulletIfSpawnedInSolidObject();
}


void Bullet::Update(float deltaSeconds)
{
	if(m_isDead)
		return;

	switch (m_type)
	{
		case ENTITY_TYPE_GOOD_BULLET: // good bullet and evil bullet will both function like bullets
		case ENTITY_TYPE_EVIL_BULLET:
			UpdateBullet(deltaSeconds);
			break;
		case ENTITY_TYPE_EVIL_GUIDED_MISSILE:
			UpdateGuidedMissle(deltaSeconds);
			break;
	}
}


void Bullet::UpdateBullet(float deltaSeconds)
{
	CheckCollisionsWithSolidObjects(deltaSeconds);
	m_position += m_velocity * deltaSeconds;
	CheckCollisionsWithNonBulletEntitiesOfTargetFaction();
}


void Bullet::UpdateGuidedMissle(float deltaSeconds)
{
	CheckCollisionsWithSolidObjects(deltaSeconds);

	m_position += m_velocity * deltaSeconds;

	float lineOfSightRange = g_gameConfigBlackboard.GetValue("evilGuidedMissileLineOfSightRange", 0.0f);
	Vec2 targetPosition = m_targetEntity->GetPosition();
	if (m_map->HasLineOfSight(m_position, targetPosition, lineOfSightRange))
	{
		Vec2 targetDir = (targetPosition - m_position).GetNormalized();
		float targetOrientationDegrees = targetDir.GetOrientationDegrees();
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, targetOrientationDegrees, m_angularVelocity * deltaSeconds);
		m_velocity = m_velocity.GetLength() * GetForwardNormal();
	}
	
	CheckCollisionsWithNonBulletEntitiesOfTargetFaction();
}


void Bullet::Render() const
{
	if (g_debugMode)
	{
		DebugRender();
	}

	if(m_isDead)
		return;

	AABB2 localSpaceBoundingBox = AABB2(-0.05f, -0.025f, 0.05f, 0.025f);
	std::vector<Vertex_PCU> bulletVertexes;
 	Texture* bulletTexture = g_theRenderer->CreateOrGetTexture(m_texturePath.c_str());
	AddVertsForAABB2ToVector(bulletVertexes, localSpaceBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) bulletVertexes.size(), bulletVertexes.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(0, bulletTexture);
	g_theRenderer->DrawVertexArray((int) bulletVertexes.size(), bulletVertexes.data());

}


void Bullet::Die()
{
	SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/BulletRicochet2.wav");
	static int s_bulletFireCount = 0;
	if (s_bulletFireCount % 3 == 0)
		g_audioSystem->StartSound(sfxId, false, 0.05f, 0.0f, 0.7f);
	s_bulletFireCount++;
	m_isDead = true;
	m_isGarbage = true;

	float randomOrientation = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
	m_map->SpawnEntityOfType(ENTITY_TYPE_BULLET_EXPLOSION, m_position, randomOrientation);
}


void Bullet::TakeDamage(int damage)
{
	char const* sfxPaths[2] = {	"Data/Audio/BulletBounce.wav",
								"Data/Audio/BulletRicochet.wav"
	};

	int soundIndex = g_rng->GetRandomIntLessThan(2);
	SoundID sfxId = g_audioSystem->CreateOrGetSound(sfxPaths[soundIndex]);
	static int s_bulletFireCount = 0;
	if (s_bulletFireCount % 3 == 0)
		g_audioSystem->StartSound(sfxId, false, 0.05f);
	s_bulletFireCount++;

	m_health -= damage;

	if (m_health <= 0)
	{
		Die();
	}
}


void Bullet::SetTargetEntity(Entity* target)
{
	m_targetEntity = target;
}


void Bullet::CheckAndCorrectBulletIfSpawnedInSolidObject()
{
	CheckCollisionsWithNonBulletEntitiesOfTargetFaction();
	
	if (m_isDead)
		return;

	Tile tile = m_map->GetTile(m_position);
	AABB2 tileBoundingBox = tile.GetBoundingBox();
	TileDefinition const*& tileDef = tile.m_tileDefinition;
	if (tileDef->m_isSolid && tileBoundingBox.IsPointInside(m_position))
	{
		Die();
	}
}


void Bullet::CheckCollisionsWithNonBulletEntitiesOfTargetFaction()
{
	EntityFaction targetFaction = (EntityFaction) ((ENTITY_FACTION_EVIL + ENTITY_FACTION_GOOD) - m_faction);
	EntityList targetEntities	= m_map->GetNonBulletEntitiesByFaction(targetFaction);

	for (int targetEntityIndex = 0; targetEntityIndex < targetEntities.size(); targetEntityIndex++)
	{
		Entity*& targetEntity = targetEntities[targetEntityIndex];
		if (targetEntity && targetEntity->IsAlive())
		{
			Vec2 targetPosition			= targetEntity->GetPosition();
			float targetPhysicsRadius	= targetEntity->GetPhysicsRadius();
			if (IsPointInsideDisk2D(m_position, targetPosition, targetPhysicsRadius))
			{
				targetEntity->TakeDamage(m_damage);
				Die();
				return;
			}
		}
	}
}


void Bullet::CheckCollisionsWithSolidObjects(float deltaSeconds)
{
	Vec2 forwardNormal		= GetForwardNormal();
	Vec2 displacement		= (m_velocity * deltaSeconds);
	Vec2 newPosition		= m_position + displacement;

	CheckAndCorrectCollisionsWithTiles(newPosition, forwardNormal, displacement.GetLength());
	CheckAndCorrectCollisionsWithAries(newPosition, forwardNormal);
}


void Bullet::CheckAndCorrectCollisionsWithTiles(Vec2 const& newPosition, Vec2 const& forwardNormal, float distanceToNewPosition)
{
	Tile previousPositionTile = m_map->GetTile(m_position);
	Tile newPositionTile = m_map->GetTile(newPosition);
	AABB2 tileBoundingBox = newPositionTile.GetBoundingBox();
	TileDefinition const*& tileDef = newPositionTile.m_tileDefinition;

	if (tileDef->m_isSolid && tileBoundingBox.IsPointInside(newPosition))
	{
		RayCastResult raycastResult = m_map->RayCast(m_position, forwardNormal, distanceToNewPosition, false);
		if (raycastResult.m_didImpact)
		{
			m_position						= raycastResult.m_impactPosition;
			Vec2 impactSurfaceNormal		= raycastResult.m_impactSurfaceNormal;
			float randomAngularDeviation	= g_rng->GetRandomFloatInRange(-10.0f, 10.0f);
			m_velocity.Reflect(impactSurfaceNormal);
			m_velocity.RotateDegrees(randomAngularDeviation);
			m_orientationDegrees = m_velocity.GetOrientationDegrees();
			TakeDamage(1);
		}
	}
}


void Bullet::CheckAndCorrectCollisionsWithAries(Vec2 const& newPosition, Vec2 const& forwardNormal)
{
	if (m_faction == ENTITY_FACTION_EVIL)
		return;

	EntityList ariesList = m_map->GetEntitiesByType(ENTITY_TYPE_EVIL_ARIES);
	for (int ariesIndex = 0; ariesIndex < ariesList.size(); ariesIndex++)
	{
		Entity*& aries				= ariesList[ariesIndex];
		Vec2 ariesCenter			= aries->GetPosition();
		float ariesPhysicsRadius	= aries->GetPhysicsRadius();
		if (IsPointInsideDisk2D(newPosition, ariesCenter, ariesPhysicsRadius))
		{
			Vec2 pointOfImpact;
			//Manually raycasting using step and sample
			float stepDistance		= 0.01f;
			int	stepNumber			= 0;
			float stepDisplacement	= stepDistance * stepNumber;
			Vec2 stepPoint			= m_position + (forwardNormal * stepDisplacement);
			while (!IsPointInsideDisk2D(stepPoint, ariesCenter, ariesPhysicsRadius))
			{
				stepNumber++;
				stepDisplacement = stepNumber * stepDistance;
				stepPoint = m_position + forwardNormal * stepDisplacement;
			}
			
			pointOfImpact						 = stepPoint;
			Vec2 pointOfImpactToAriesPosVec		 = pointOfImpact - ariesCenter;
			Vec2& vector1						 = pointOfImpactToAriesPosVec;
			Vec2 ariesForwardNormal				 = aries->GetForwardNormal();
			Vec2& vector2						 = ariesForwardNormal;
			float angleBetweenVectors1And2		 = GetAngleDegreesBetweenVectors2D(vector1, vector2);
			float ariesShieldHalfApertureDegrees = g_gameConfigBlackboard.GetValue("ariesFrontShieldHalfApertureDegrees", 0.0f);
			if (angleBetweenVectors1And2	<= ariesShieldHalfApertureDegrees)
			{
				m_position			= pointOfImpact;
				Vec2 impactNormal	= pointOfImpactToAriesPosVec.GetNormalized();
				m_velocity.Reflect(impactNormal);
				m_orientationDegrees = m_velocity.GetOrientationDegrees();
				TakeDamage(1);
			}
		}
	}
}
