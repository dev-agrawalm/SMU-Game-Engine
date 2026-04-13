#include "Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/AABB2.hpp"

Entity::Entity(Vec2 const&  startingPosition, float orientationDegrees, EntityFaction faction, EntityType type, Map* map) :
	m_position(startingPosition),
	m_orientationDegrees(orientationDegrees),
	m_faction(faction),
	m_type(type),
	m_map(map)
{
	//empty
}


Entity::Entity()
{
	//empty
}


Entity::~Entity()
{
}


void Entity::DebugRender() const
{
	//draw line for velocity
	DrawLine(m_position, m_velocity, Rgba8::YELLOW, DEFAULT_LINE_WIDTH + 0.03f);
	//draw physics circle
	DrawRing(m_position, m_physicsRadius, Rgba8::CYAN, DEFAULT_LINE_WIDTH);
	//draw cosmetic circle
	DrawRing(m_position, m_cosmeticRadius, Rgba8::MAGENTA, DEFAULT_LINE_WIDTH);
	//draw +x basis
	DrawLine(m_position, GetForwardNormal() * 0.75f, Rgba8::RED, DEFAULT_LINE_WIDTH);
	//draw +y basis
	DrawLine(m_position, GetForwardNormal().GetRotated90Degrees() * 0.75, Rgba8::GREEN, DEFAULT_LINE_WIDTH);
}


void Entity::Die()
{
	m_isDead = true;
}


bool Entity::IsAlive() const
{
	return !m_isDead;
}


bool Entity::IsGarbage() const
{
	return m_isGarbage;
}


void Entity::TakeDamage(int damage)
{
	m_health -= damage;

	if (m_health <= 0)
	{
		Die();
	}
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


float Entity::GetAcceleration() const
{
	return m_acceleration;
}


float Entity::GetOrientationDegrees() const
{
	return m_orientationDegrees;
}


float Entity::GetSpeed() const
{
	return m_velocity.GetLength();
}


EntityType Entity::GetType() const
{
	return m_type;
}


EntityFaction Entity::GetFaction() const
{
	return m_faction;
}


void Entity::SetPosition(Vec2 const& position)
{
	m_position = position;
}


void Entity::SetMap(Map* map)
{
	m_map = map;
}


void Entity::SetLifeStatus(bool isAlive)
{
	m_isDead = !isAlive;
}


void Entity::SetHealth(int health)
{
	m_health = health;
}
