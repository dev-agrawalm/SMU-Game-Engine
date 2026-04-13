#include "Game/Asteroid.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Game.hpp"
#include "Engine/Audio/AudioSystem.hpp"

const Rgba8 Asteroid::ASTEROID_GREY(100, 100, 100, 255);

Asteroid::Asteroid(Game* game, Vec2 const& position, float orientationAngle, Vec2 const& velocity) : Entity(game, position)
{
	m_orientationDegrees = orientationAngle;
	m_velocity = velocity;
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_health = ASTEROID_HEALTH;

	InitAngularVelocity();
	InitLocalVertexes();
}

Asteroid::Asteroid()
{
	//empty
}

Asteroid::~Asteroid()
{
	//empty
}

void Asteroid::Init(Game* game, Vec2 const& position, float orientationAngle, Vec2 const& velocity)
{
	m_game = game;
	m_position = position;
	m_orientationDegrees = orientationAngle;
	m_velocity = velocity;
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_health = ASTEROID_HEALTH;

	InitAngularVelocity();
	InitLocalVertexes();
}

void Asteroid::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		m_isGarbage = true;
		return;
	}

	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
	m_wasHit = false;

	if (IsOffScreen())
	{
		WarpAroundTheScreen();
	}
}

void Asteroid::Render() const
{
	if (m_isDead)
		return;

	Vertex_PCU tempVerts[NUM_ASTEROID_VERTS];

	for (int index = 0; index < NUM_ASTEROID_VERTS; index++)
	{
		tempVerts[index] = m_localVertexes[index];
		if (m_wasHit)
		{
			tempVerts[index].m_color = Rgba8::WHITE;
		}
	}

	TransformVertexArrayXY3D(NUM_ASTEROID_VERTS, tempVerts, 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_ASTEROID_VERTS, tempVerts);
}

void Asteroid::TakeDamage(int damage)
{
	m_health -= damage;
	m_wasHit = true;

	if (m_health <= 0)
	{
		Die();
		size_t asteroidDeathSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyDied.wav");
		g_audioSystem->StartSound(asteroidDeathSfxId, false, 0.25f);
	}
}

Rgba8 Asteroid::GetDebrisColor() const
{
	return Asteroid::ASTEROID_GREY;
}

void Asteroid::InitLocalVertexes()
{
	m_localVertexes = new Vertex_PCU[NUM_ASTEROID_VERTS];
	const int numTriangles = NUM_ASTEROID_VERTS / 3;
	float thetaIntervalDegrees = 360.0f / static_cast<float>(numTriangles);
	float startThetaDegrees = 0.0f;
	float endThetaDegrees = thetaIntervalDegrees;
	float startVertLength = 0.0f;
	float endVertLength = 0.0f;
	Vec2 centerPosition(0.0f, 0.0f);

	startVertLength = g_rng->GetRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
	for (int count = 0; count < numTriangles; count++)
	{
		Vec2 position;

		m_localVertexes[count * 3 + 0].m_position = Vec2(centerPosition).GetVec3();
		m_localVertexes[count * 3 + 0].m_color = Asteroid::ASTEROID_GREY;
		
		m_localVertexes[count * 3 + 1].m_position = Vec2(Vec2::MakeFromPolarDegrees(startThetaDegrees, startVertLength)).GetVec3();
		m_localVertexes[count * 3 + 1].m_color = Asteroid::ASTEROID_GREY;
		
		endVertLength = g_rng->GetRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
		m_localVertexes[count * 3 + 2].m_position = Vec2(Vec2::MakeFromPolarDegrees(endThetaDegrees, endVertLength)).GetVec3();
		m_localVertexes[count * 3 + 2].m_color = Asteroid::ASTEROID_GREY;

		startVertLength = endVertLength;
		startThetaDegrees += thetaIntervalDegrees;
		endThetaDegrees += thetaIntervalDegrees;
	}
}

void Asteroid::InitAngularVelocity()
{
	float angularVelocity = g_rng->GetRandomFloatInRange(MIN_ASTEROID_ANGULAR_VELOCITY, MAX_ASTEROID_ANGULAR_VELOCITY);
	m_angularVelocity = angularVelocity;
}

void Asteroid::WarpAroundTheScreen()
{
	if (m_position.y > WORLD_SIZE_Y + m_cosmeticRadius) 
	{
		m_position.y = 0.0f - m_cosmeticRadius;
	}
	else if(m_position.y < 0.0f - m_cosmeticRadius)
	{
		m_position.y = WORLD_SIZE_Y + m_cosmeticRadius;
	}

	if (m_position.x > WORLD_SIZE_X + m_cosmeticRadius)
	{
		m_position.x = 0.0f - m_cosmeticRadius;
	}
	else if(m_position.x < 0.0f - m_cosmeticRadius)
	{
		m_position.x = WORLD_SIZE_X + m_cosmeticRadius;
	}
}

