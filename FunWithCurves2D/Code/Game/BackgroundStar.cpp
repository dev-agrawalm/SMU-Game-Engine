#include "Game/BackgroundStar.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"

constexpr int NUM_VERTS_BG_STAR = 8 * 3;

Vec2 BackgroundStar::s_velocity = Vec2::ZERO;

BackgroundStar::BackgroundStar(Vec2 const& position)
	: Entity(position, ENTITY_FACTION_NEUTRAL)
{
	static FloatRange cosmeticRadiusRange = g_gameConfigBlackboard.GetValue("bgStarCosmeticRadiusRange", FloatRange::ZERO_TO_ONE);
	m_cosmeticRadius = g_rng->GetRandomFloatInRange(cosmeticRadiusRange.m_min, cosmeticRadiusRange.m_max);
	static FloatRange starAngularVelocityRange = g_gameConfigBlackboard.GetValue("bgStarAngularVelocityRange", FloatRange::ZERO_TO_ONE);
	m_angularVelocity = g_rng->GetRandomFloatInRange(starAngularVelocityRange.m_min, starAngularVelocityRange.m_max);
	InitLocalVerts();
}


void BackgroundStar::Update(float deltaSeconds)
{
	static FloatRange rangeX = g_gameConfigBlackboard.GetValue("bgStarXRange", FloatRange::ZERO_TO_ONE);
	static FloatRange rangeY = g_gameConfigBlackboard.GetValue("bgStarYRange", FloatRange::ZERO_TO_ONE);
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
	m_position += s_velocity * deltaSeconds;

	if (m_position.x > rangeX.m_max + m_cosmeticRadius)
	{
		m_position.x = rangeX.m_min - m_cosmeticRadius;
	}

	if (m_position.x < rangeX.m_min - m_cosmeticRadius)
	{
		m_position.x = rangeX.m_max + m_cosmeticRadius;
	}

	if (m_position.y < rangeX.m_min - m_cosmeticRadius)
	{
		m_position.y = rangeY.m_max + m_cosmeticRadius;
	}
}


void BackgroundStar::Render() const
{
	Vertex_PCU m_tempVerts[NUM_VERTS_BG_STAR] = {};
	for (int vertIndex = 0; vertIndex < NUM_VERTS_BG_STAR; vertIndex++)
	{
		m_tempVerts[vertIndex] = m_localVertexes[vertIndex];
	}
	TransformVertexArrayXY3D(NUM_VERTS_BG_STAR, m_tempVerts, 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray(NUM_VERTS_BG_STAR, m_tempVerts);
}


void BackgroundStar::InitLocalVerts()
{
	const int NUM_SECTIONS_STAR = 12;
	const int NUM_VERTS_SECTION = 3;
	const int TOTAL_VERT_COUNT = NUM_SECTIONS_STAR * NUM_VERTS_SECTION;
	m_localVertexes = new Vertex_PCU[TOTAL_VERT_COUNT];

	float thetaInterval = (360.0f / static_cast<float>(NUM_SECTIONS_STAR)); //the angle at the center of the circle for each individual quad segment of the ring

	float startThetaDegrees = 0.0f; //angle of the starting line of our quad segment
	float endThetaDegrees = thetaInterval; //angle of the ending line of our quad segment
	int initVertexCount = 0; //number of vertexes in the vertex array that have been initialized

	while (initVertexCount < TOTAL_VERT_COUNT)
	{
		float cosStartTheta = CosDegrees(startThetaDegrees);
		float sinStartTheta = SinDegrees(startThetaDegrees);
		float cosEndTheta = CosDegrees(endThetaDegrees);
		float sinEndTheta = SinDegrees(endThetaDegrees);

		float outCircleRadius1 = g_rng->GetRandomFloatInRange(m_cosmeticRadius * 0.2f, m_cosmeticRadius);
		Vec2 outerPoint1 = Vec2(outCircleRadius1 * cosStartTheta, outCircleRadius1 * sinStartTheta);
	
		float outCircleRadius2 = g_rng->GetRandomFloatInRange(m_cosmeticRadius * 0.2f, m_cosmeticRadius);
		Vec2 outerPoint2 = Vec2(outCircleRadius2 * cosEndTheta, outCircleRadius2 * sinEndTheta);

		for (int index = initVertexCount; index < initVertexCount + NUM_VERTS_SECTION; index++)
		{
			m_localVertexes[index].m_color = Rgba8::WHITE;
		}

		m_localVertexes[initVertexCount].m_position = Vec2::ZERO.GetVec3();
		m_localVertexes[initVertexCount + 1].m_position = initVertexCount < 3 ? Vec2(outerPoint1).GetVec3() : m_localVertexes[initVertexCount -1].m_position;
		m_localVertexes[initVertexCount + 2].m_position = initVertexCount + 3 != TOTAL_VERT_COUNT ? Vec2(outerPoint2).GetVec3() : m_localVertexes[1].m_position;

		initVertexCount += NUM_VERTS_SECTION;
		startThetaDegrees += thetaInterval;
		endThetaDegrees += thetaInterval;
	}
}
