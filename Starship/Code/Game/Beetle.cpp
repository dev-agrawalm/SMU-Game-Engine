#include "Beetle.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"

const Rgba8 Beetle::BEETLE_GREEN(58, 145, 100, 255);

Beetle::Beetle()
{
}

Beetle::Beetle(Game* game, Vec2 const&  position, Entity* targetEntity) : Entity(game, position)
{
	m_cosmeticRadius = BEETLE_COSMETIC_RADIUS * BEETLE_SCALE;
	m_physicsRadius = BEETLE_PHYSICS_RADIUS * BEETLE_SCALE;
	m_targetEntity = targetEntity;
	m_health = BEETLE_HEALTH;
	
	InitLocalVertexes();
}

Beetle::~Beetle()
{
	m_targetEntity = nullptr;
}

void Beetle::Init(Game* game, Vec2 const& position, Entity* targetEntity)
{
	m_game = game;
	m_position = position;
	m_cosmeticRadius = BEETLE_COSMETIC_RADIUS * BEETLE_SCALE;
	m_physicsRadius = BEETLE_PHYSICS_RADIUS * BEETLE_SCALE;
	m_targetEntity = targetEntity;
	m_health = BEETLE_HEALTH;

	InitLocalVertexes();
}

void Beetle::Update(float deltaSeconds)
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

	m_velocity = GetForwardNormal() * BEETLE_SPEED;
	m_position += m_velocity * deltaSeconds;
	m_wasHit = false;

	if (!m_targetEntity->IsAlive() && IsOffScreen())
	{
		WarpAroundTheScreen();
	}
}

void Beetle::Render() const
{
	if (m_isDead)
		return;

	Vertex_PCU tempVerts[NUM_BEETLE_VERTS];

	for (int index = 0; index < NUM_BEETLE_VERTS; index++)
	{
		tempVerts[index] = m_localVertexes[index];
		if (m_wasHit)
		{
			tempVerts[index].m_color = Rgba8::WHITE;
		}
	}

	TransformVertexArrayXY3D(NUM_BEETLE_VERTS, tempVerts, 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_BEETLE_VERTS, tempVerts);
}

void Beetle::TakeDamage(int damage)
{
	m_health -= damage;
	m_wasHit = true;

	if (m_health <= 0)
	{
		Die();
		size_t beetleDeathSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyDied.wav");
		g_audioSystem->StartSound(beetleDeathSfxId, false, 0.25f);
	}
}

Rgba8 Beetle::GetDebrisColor() const
{
	return Beetle::BEETLE_GREEN;
}

void Beetle::InitLocalVertexes()
{
	m_localVertexes = new Vertex_PCU[NUM_BEETLE_VERTS];

	for (int index = 0; index < NUM_BEETLE_VERTS; index++)
	{
		m_localVertexes[index].m_color = Beetle::BEETLE_GREEN;
	}

	//left wing triangle
	m_localVertexes[0].m_position = Vec3(1.0f, 0.5f) * BEETLE_SCALE;
	m_localVertexes[0].m_color = Rgba8::WHITE;
	m_localVertexes[1].m_position = Vec3(1.0f, 1.0f) * BEETLE_SCALE;
	m_localVertexes[1].m_color = Rgba8::WHITE;
	m_localVertexes[2].m_position = Vec3(0.5f, 0.5f) * BEETLE_SCALE;
	m_localVertexes[2].m_color = Rgba8::WHITE;
	//backside top left triangle
	m_localVertexes[3].m_position = Vec3(1.0f, 1.0f) * BEETLE_SCALE;
	m_localVertexes[4].m_position = Vec3(0.5f, 1.0f) * BEETLE_SCALE;
	m_localVertexes[5].m_position = Vec3(0.5f, 0.5f) * BEETLE_SCALE;
	//backside bottom right triangle
	m_localVertexes[6].m_position = Vec3(0.5f, -0.5f) * BEETLE_SCALE;
	m_localVertexes[7].m_position = Vec3(0.5f, 0.5f) * BEETLE_SCALE;
	m_localVertexes[8].m_position = Vec3(-0.5f, 0.0f) * BEETLE_SCALE;
	//right wing triangle
	m_localVertexes[9].m_position = Vec3(1.0f, -1.0f) * BEETLE_SCALE;
	m_localVertexes[9].m_color = Rgba8::WHITE;
	m_localVertexes[10].m_position = Vec3(1.0f, -0.5f) * BEETLE_SCALE;
	m_localVertexes[10].m_color = Rgba8::WHITE;
	m_localVertexes[11].m_position = Vec3(0.5f, -0.5f) * BEETLE_SCALE;
	m_localVertexes[11].m_color = Rgba8::WHITE;

	m_localVertexes[12].m_position = Vec3(0.5f, -0.5f) * BEETLE_SCALE;
	m_localVertexes[13].m_position = Vec3(0.5f, -1.0f) * BEETLE_SCALE;
	m_localVertexes[14].m_position = Vec3(1.0f, -1.0f) * BEETLE_SCALE;
	
	m_localVertexes[15].m_position = Vec3(-0.5f, 0.0f) * BEETLE_SCALE;
	m_localVertexes[16].m_position = Vec3(-0.5f, 0.5f) * BEETLE_SCALE;
	m_localVertexes[17].m_position = Vec3(-1.0f, 0.0f) * BEETLE_SCALE;

	m_localVertexes[18].m_position = Vec3(-0.5f, 0.0f) * BEETLE_SCALE;
	m_localVertexes[19].m_position = Vec3(-1.0f, 0.0f) * BEETLE_SCALE;
	m_localVertexes[20].m_position = Vec3(-0.5f, -0.5f) * BEETLE_SCALE;
}

void Beetle::WarpAroundTheScreen()
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