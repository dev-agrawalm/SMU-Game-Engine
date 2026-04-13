#include "Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Entity::Entity(Vec2 const&  startingPosition) 
	: m_position(startingPosition) 
{
	//empty
}


Entity::Entity()
{
	//empty
}


Entity::Entity(Vec2 const& startingPos, EntityFaction faction)
	: m_position(startingPos)
	, m_faction(faction)
{

}


Entity::~Entity()
{
	if (m_localVertexes)
	{
		delete[] m_localVertexes;
		m_localVertexes = nullptr;
	}
}


void Entity::DebugRender() const
{
	const Rgba8 VELOCITY_LINE_COLOR(255, 255, 0);
	const Rgba8 PHYSICS_RING_COLOR(0, 255, 255);
	const Rgba8 COSMETIC_RING_COLOR(255, 0, 255);
	const Rgba8 X_BASIS_COLOR(255, 0, 0, 255);
	const Rgba8 Y_BASIS_COLOR(0, 255, 0, 255);
	const float DEBUG_LINE_WIDTH = 0.005f;
	const float DEBUG_LINE_LENGTH = 1.0f;

	//draw line for velocity
	DrawLine(m_position, m_velocity, DEBUG_LINE_WIDTH, VELOCITY_LINE_COLOR);
	//draw physics circle
	DrawRing(m_position, m_physicsRadius, DEBUG_LINE_WIDTH, PHYSICS_RING_COLOR);
	//draw cosmetic circle
	DrawRing(m_position, m_cosmeticRadius, DEBUG_LINE_WIDTH, COSMETIC_RING_COLOR);
	//draw +x basis
	DrawLine(m_position, GetForwardNormal() * DEBUG_LINE_LENGTH, DEBUG_LINE_WIDTH, X_BASIS_COLOR);
	//draw +y basis
	DrawLine(m_position, GetForwardNormal().GetRotated90Degrees() * DEBUG_LINE_LENGTH, DEBUG_LINE_WIDTH, Y_BASIS_COLOR);
}


void Entity::Die()
{
	m_isDead = true;
}


bool Entity::IsOffScreen() const
{
	float worldSizeX = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
	float worldSizeY = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);
	if ((m_position.x - worldSizeX) > m_cosmeticRadius || (0.0f - m_position.x) > m_cosmeticRadius
		|| (m_position.y - worldSizeY) > m_cosmeticRadius  || (0.0f - m_position.y) > m_cosmeticRadius)
	{
		return true;
	}

	return false;
}


bool Entity::IsAlive() const
{
	return !m_isDead;
}


bool Entity::IsGarbage() const
{
	return m_isGarbage;
}


void Entity::SetFaction(EntityFaction faction)
{
	m_faction = faction;
}


void Entity::SetPosition(Vec2 const& newPos)
{
	m_position = newPos;
}


void Entity::SetOrientation(float newOrientation)
{
	m_orientationDegrees = newOrientation;
}


void Entity::TakeDamage(int damage)
{
	m_health -= damage;

	if (m_health <= 0)
	{
		Die();
	}
}


void Entity::MakeGarbage()
{
	m_isGarbage = true;
}


Vec2 Entity::GetForwardNormal() const
{
	Vec2 forwardNormal = Vec2(CosDegrees(m_orientationDegrees), SinDegrees(m_orientationDegrees));
	return forwardNormal;
}


Vec2 Entity::GetPosition() const
{
	return m_position;
}


float Entity::GetPhysicsRadius() const
{
	return m_physicsRadius;
}


float Entity::GetCosmeticRadius() const
{
	return m_cosmeticRadius;
}


float Entity::GetOrientationDegrees() const
{
	return m_orientationDegrees;
}


float Entity::GetSpeed() const
{
	return m_velocity.GetLength();
}


EntityFaction Entity::GetFaction() const
{
	return m_faction;
}


bool Entity::IsBullet() const
{
	return m_isBullet;
}


int Entity::GetHealth() const
{
	return m_health;
}


int Entity::GetDamageOnCollision() const
{
	return m_damageOnCollision;
}


bool Entity::IsDead() const
{
	return m_isDead;
}


bool Entity::DieOnImpact() const
{
	return m_dieOnImpact;
}


void Entity::SetLocalVerts(std::vector<Vertex_PCU> verts)
{
	m_localVertexes = verts.data();
}
