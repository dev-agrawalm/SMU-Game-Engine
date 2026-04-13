#include "Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Entity::Entity(Game* game, Vec2 const&  startingPosition) :
	m_game(game), 
	m_position(startingPosition) 
{
	//empty
}

Entity::Entity()
{
	//empty
}

Entity::~Entity()
{
	delete[] m_localVertexes;
	m_localVertexes = nullptr;
}

void Entity::DebugRender() const
{
	const Rgba8 VELOCITY_LINE_COLOR(255, 255, 0);
	const Rgba8 PHYSICS_RING_COLOR(0, 255, 255);
	const Rgba8 COSMETIC_RING_COLOR(255, 0, 255);
	const Rgba8 X_BASIS_COLOR(255, 0, 0, 255);
	const Rgba8 Y_BASIS_COLOR(0, 255, 0, 255);

	//draw line for velocity
	DrawLine(m_position, m_velocity, DEBUG_LINE_WIDTH, VELOCITY_LINE_COLOR);
	//draw physics circle
	DrawRing(m_position, m_physicsRadius, DEBUG_LINE_WIDTH, PHYSICS_RING_COLOR);
	//draw cosmetic circle
	DrawRing(m_position, m_cosmeticRadius, DEBUG_LINE_WIDTH, COSMETIC_RING_COLOR);
	//draw +x basis
	DrawLine(m_position, GetForwardNormal() * 1.5f, DEBUG_LINE_WIDTH, X_BASIS_COLOR);
	//draw +y basis
	DrawLine(m_position, GetForwardNormal().GetRotated90Degrees() * 1.5f, DEBUG_LINE_WIDTH, Y_BASIS_COLOR);
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

Vertex_PCU* const Entity::GetLocalVertexes() const
{
	return m_localVertexes;
}