#include "Game/Wasp.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"

Wasp::Wasp()
{
}

Wasp::Wasp(Game* game, Vec2 const&  position, Entity* targetEntity) : Entity(game, position)
{
	m_cosmeticRadius = WASP_COSMETIC_RADIUS * WASP_SCALE;
	m_physicsRadius = WASP_PHYSICS_RADIUS * WASP_SCALE;
	m_health = WASP_HEALTH;
	m_acceleration = WASP_ACCELERATION;
	m_targetEntity = targetEntity;

	InitLocalVertexes();
}

Wasp::~Wasp()
{
}

void Wasp::Init(Game* game, Vec2 const& position, Entity* targetEntity)
{
	m_game = game;
	m_position = position;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS * WASP_SCALE;
	m_physicsRadius = WASP_PHYSICS_RADIUS * WASP_SCALE;
	m_health = WASP_HEALTH;
	m_acceleration = WASP_ACCELERATION;
	m_targetEntity = targetEntity;

	InitLocalVertexes();
}

void Wasp::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		m_isGarbage = true;
		return;
	}

	if (m_targetEntity->IsAlive())
	{
		Vec2 targetEntityLocalPosition = m_targetEntity->GetPosition() - m_position;
		m_orientationDegrees = targetEntityLocalPosition.GetOrientationDegrees();
	}

	m_velocity += GetForwardNormal() * m_acceleration * deltaSeconds;
	m_velocity.ClampLength(WASP_MAX_SPEED);
	m_position += m_velocity * deltaSeconds;
	m_wasHit = false;

	if (!m_targetEntity->IsAlive() && IsOffScreen())
	{
		WarpAroundTheScreen();
	}
}

void Wasp::Render() const
{
	if (m_isDead)
		return;

	Vertex_PCU tempVerts[NUM_WASP_VERTS];

	for (int index = 0; index < NUM_WASP_VERTS; index++)
	{
		tempVerts[index] = m_localVertexes[index];
		if (m_wasHit)
		{
			tempVerts[index].m_color = Rgba8::WHITE;
		}
	}

	TransformVertexArrayXY3D(NUM_WASP_VERTS, tempVerts, 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_WASP_VERTS, tempVerts);
}

void Wasp::TakeDamage(int damage)
{
	m_health -= damage;
	m_wasHit = true;

	if (m_health <= 0)
	{
		Die();
		size_t waspDeathSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyDied.wav");
		g_audioSystem->StartSound(waspDeathSfxId, false, 0.25f);
	}
}

Rgba8 Wasp::GetDebrisColor() const
{
	return Rgba8::YELLOW;
}

void Wasp::InitLocalVertexes()
{
	m_localVertexes = new Vertex_PCU[NUM_WASP_VERTS];

	for (int index = 0; index < NUM_WASP_VERTS; index++)
	{
		m_localVertexes[index].m_color = Rgba8::YELLOW;
	}

	m_localVertexes[0].m_position = Vec3(1.5f, 0.0f) * WASP_SCALE;
	m_localVertexes[1].m_position = Vec3(-0.5f, 0.5f) * WASP_SCALE;
	m_localVertexes[2].m_position = Vec3(-0.5f, -0.5f) * WASP_SCALE;
	
	m_localVertexes[3].m_position = Vec3(-0.5f, 1.0f) * WASP_SCALE;
	m_localVertexes[3].m_color = Rgba8::BLACK;
	m_localVertexes[4].m_position = Vec3(-1.0f, 0.0f) * WASP_SCALE;
	m_localVertexes[5].m_position = Vec3(-0.5f, -1.0f) * WASP_SCALE;
	m_localVertexes[5].m_color = Rgba8::BLACK;

	m_localVertexes[6].m_position = Vec3(-0.5f, 1.0f) * WASP_SCALE;
	m_localVertexes[7].m_position = Vec3(-1.5f, 1.5f) * WASP_SCALE;
	m_localVertexes[8].m_position = Vec3(-1.0f, 0.0f) * WASP_SCALE;
	
	m_localVertexes[9].m_position = Vec3(-1.0f, 0.0f) * WASP_SCALE;
	m_localVertexes[10].m_position = Vec3(-1.5f, -1.5f) * WASP_SCALE;
	m_localVertexes[11].m_position = Vec3(-0.5f, -1.0f) * WASP_SCALE;
}

void Wasp::WarpAroundTheScreen()
{
	if (m_position.y > WORLD_SIZE_Y + m_cosmeticRadius)
	{
		m_position.y = 0.0f - m_cosmeticRadius;
	}
	else if (m_position.y < 0.0f - m_cosmeticRadius)
	{
		m_position.y = WORLD_SIZE_Y + m_cosmeticRadius;
	}

	if (m_position.x > WORLD_SIZE_X + m_cosmeticRadius)
	{
		m_position.x = 0.0f - m_cosmeticRadius;
	}
	else if (m_position.x < 0.0f - m_cosmeticRadius)
	{
		m_position.x = WORLD_SIZE_X + m_cosmeticRadius;
	}
}