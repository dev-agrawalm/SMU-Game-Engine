#include"Game/Debris.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

Debris::Debris()
{
}

Debris::Debris(Game* game, Vec2 const& position, Rgba8 color, Entity* fromEntity) : Entity(game, position)
{
	m_color = color;
	m_color.a = static_cast<unsigned char>(DEBRIS_MAX_ALPHA);
	m_cosmeticRadius = g_rng->GetRandomFloatInRange(DEBRIS_MIN_COSMETIC_RADIUS, fromEntity->GetPhysicsRadius());
	m_fadeOutTime = DEBRIS_FADE_OUT_TIME;

	InitOrientationAngle(fromEntity);
	InitLocalVertexes();

	m_velocity = GetForwardNormal() * g_rng->GetRandomFloatInRange(DEBRIS_MIN_SPEED, fromEntity->GetSpeed());
	m_angularVelocity = g_rng->GetRandomFloatInRange(-DEBRIS_MAX_ANGULAR_VELOCITY, DEBRIS_MAX_ANGULAR_VELOCITY);
}


Debris::Debris(Game* game, Vec2 const& position, Rgba8 color, Vec2 const& direction, float speed, float fadeoutTime, float minCosmetic, float maxCosmetic)
	: Entity(game, position)
{
	m_color = color;
	m_cosmeticRadius = g_rng->GetRandomFloatInRange(minCosmetic, maxCosmetic);
	m_fadeOutTime = fadeoutTime;
	m_orientationDegrees = direction.GetOrientationDegrees();
	m_velocity = direction * speed;
	m_angularVelocity = g_rng->GetRandomFloatInRange(-DEBRIS_MAX_ANGULAR_VELOCITY, DEBRIS_MAX_ANGULAR_VELOCITY);
	m_canBeOffscreen = true;

	InitLocalVertexes();
}


Debris::~Debris()
{
}

void Debris::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		m_isGarbage = true;
		return;
	}

	if (m_timeAlive >= m_fadeOutTime || (!m_canBeOffscreen && IsOffScreen()))
	{
		m_isGarbage = true;
		m_isDead = true;
		return;
	}

	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
	m_timeAlive += deltaSeconds;
}

void Debris::Render() const
{
	if (m_isDead)
		return;

	Vertex_PCU tempVerts[NUM_DEBRIS_VERTS];

	for (int index = 0; index < NUM_DEBRIS_VERTS; index++)
	{
		tempVerts[index] = m_localVertexes[index];
		
		if (m_timeAlive > 2.0f)
		{
			int a = 0;
			a += 10;
		}

		int newAlpha = RoundDownToInt(RangeMapClamped(m_timeAlive, 0.0f, m_fadeOutTime, DEBRIS_MAX_ALPHA, 0.0f));
		unsigned char newAlphaChar = static_cast<unsigned char> (newAlpha);
		tempVerts[index].m_color.a = newAlphaChar;
	}

	TransformVertexArrayXY3D(NUM_DEBRIS_VERTS, tempVerts, 1.0f, m_orientationDegrees, m_position);

	g_theRenderer->DrawVertexArray(NUM_DEBRIS_VERTS, tempVerts);
}

Rgba8 Debris::GetDebrisColor() const
{
	return m_color;
}

void Debris::InitLocalVertexes()
{
	const int numVerts = g_rng->GetRandomIntInRange(3, NUM_DEBRIS_VERTS + 1); //3 is the minimum number of vertices required to render a triangle
	m_localVertexes = new Vertex_PCU[NUM_DEBRIS_VERTS];

	const int numTriangles = NUM_DEBRIS_VERTS / 3;

	float thetaIntervalDegrees = 360.0f / static_cast<float>(numTriangles);
	float startThetaDegrees = 0.0f;
	float endThetaDegrees = thetaIntervalDegrees;
	float startVertLength = 0.0f;
	float endVertLength = 0.0f;
	Vec2 centerPosition(0.0f, 0.0f);

	startVertLength = g_rng->GetRandomFloatInRange(DEBRIS_MIN_COSMETIC_RADIUS, m_cosmeticRadius);
	for (int count = 0; count < numTriangles; count++)
	{
		Vec2 position;

		m_localVertexes[count * 3 + 0].m_position = Vec2(centerPosition).GetVec3();
		m_localVertexes[count * 3 + 0].m_color = m_color;

		m_localVertexes[count * 3 + 1].m_position = Vec2(Vec2::MakeFromPolarDegrees(startThetaDegrees, startVertLength)).GetVec3();
		m_localVertexes[count * 3 + 1].m_color = m_color;

		endVertLength = g_rng->GetRandomFloatInRange(DEBRIS_MIN_COSMETIC_RADIUS, m_cosmeticRadius);
		m_localVertexes[count * 3 + 2].m_position = Vec2(Vec2::MakeFromPolarDegrees(endThetaDegrees, endVertLength)).GetVec3();
		m_localVertexes[count * 3 + 2].m_color = m_color;

		startVertLength = endVertLength;
		startThetaDegrees += thetaIntervalDegrees;
		endThetaDegrees += thetaIntervalDegrees;
	}
}

void Debris::InitOrientationAngle(Entity* fromEntity)
{
	float fromEntityOrientationDegree = fromEntity->GetOrientationDegrees();
	float fromEntityAcceleration = fromEntity->GetAcceleration();
	const float maxAcceleration = 40.0f;

	float thetaDeviation = RangeMap(fromEntityAcceleration, 0.0f, maxAcceleration, DEGREES_180, DEGREES_0);
	m_orientationDegrees = g_rng->GetRandomFloatInRange(fromEntityOrientationDegree - thetaDeviation, fromEntityOrientationDegree + thetaDeviation);
}
