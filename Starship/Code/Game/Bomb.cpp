#include "Game/Bomb.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"

const Rgba8 Bomb::BOMB_RED = Rgba8(219, 50, 50, 255);
const Rgba8 Bomb::BOMB_WHITE = Rgba8(255, 255, 255, 255);
const Rgba8 Bomb::EXPLOSION_WHITE = Rgba8(248, 228, 218,255);
const Rgba8 Bomb::EXPLOSION_BLACK = Rgba8(0, 0, 0, 255);

Bomb::Bomb()
{

}

Bomb::~Bomb()
{

}

Bomb::Bomb(Game* game, Vec2 const& position) : Entity(game, position)
{
	m_waitTime = BOMB_WAIT_TIMER_SECONDS;
	m_previousWaitTime = m_waitTime;
	m_bombColor = Bomb::BOMB_RED;

	m_explosionLifetime = BOMB_EXPLOSION_DURATION;
	m_previousExplosionLifetime = m_explosionLifetime;
	m_explosionColor = Bomb::EXPLOSION_WHITE;

	m_explosionRadius = BOMB_EXPLOSION_RADIUS;
	m_damage = BOMB_DAMAGE;
	m_cosmeticRadius = BOMB_COSMETIC_RADIUS;

	InitLocalVertexes();
}

void Bomb::Update(float deltaSeconds)
{
	if (!m_isExploded)
	{
		m_waitTime -= deltaSeconds;
		m_deltaWaitTime = m_previousWaitTime - m_waitTime;

		if (m_deltaWaitTime >= m_waitTimeBlinkInterval)
		{
			m_waitTimeBlinkInterval = RangeMap(m_waitTime, BOMB_WAIT_TIMER_SECONDS, 0.0f, 0.75, 0.1f);
			m_bombColor = m_bombColor == Bomb::BOMB_RED ? Bomb::BOMB_WHITE : Bomb::BOMB_RED;
			m_previousWaitTime = m_waitTime;
		}

		if (m_waitTime <= 0.0f)
		{
			m_isExploded = true;
			size_t bombExplosionSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/Explosion.wav");
			g_audioSystem->StartSound(bombExplosionSfxId, false, 0.6f, 0.0f, 1.1f);
			m_game->SetWorldCamScreenShakeParameters(BOMB_EXPLOSION_SCREEN_SHAKE_MAGNITUDE, BOMB_EXPLOSION_SCREEN_SHAKE_DURATION);
		}
	}
	
	if (m_isExploded)
	{
		m_explosionLifetime -= deltaSeconds;
		m_deltaExplosionLifetime = m_previousExplosionLifetime - m_explosionLifetime;

		if (m_deltaExplosionLifetime >= m_explosionBlinkInterval)
		{
			m_explosionColor = m_explosionColor == Bomb::EXPLOSION_WHITE ? Bomb::EXPLOSION_BLACK : Bomb::EXPLOSION_WHITE;
			m_previousExplosionLifetime = m_explosionLifetime;
		}

		if (m_explosionLifetime <= 0.0f)
		{
			m_isDead = true;
			m_isGarbage = true;
		}
	}
}

void Bomb::Render() const
{
	if (m_isDead)
		return;

	Vertex_PCU tempVerts[NUM_BOMB_QUADS * NUM_QUAD_VERTS];
	float renderScale;
	renderScale = m_isExploded ?
		(m_explosionRadius / m_cosmeticRadius) : m_cosmeticRadius;

	for (int index = 0; index < NUM_BOMB_QUADS * NUM_QUAD_VERTS; index++)
	{
		tempVerts[index].m_position = m_localVertexes[index].m_position;

		//flicker animation for the bomb and the bomb explosion
		if (!m_isExploded)
		{
			tempVerts[index].m_color = m_bombColor;
		}
		else
		{
			tempVerts[index].m_color = m_explosionColor;
		}
	}

	TransformVertexArrayXY3D(NUM_BOMB_QUADS * NUM_QUAD_VERTS, tempVerts, renderScale, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_BOMB_QUADS * NUM_QUAD_VERTS, tempVerts);

	if (!m_isExploded)
		DebugDrawRing(m_position, m_cosmeticRadius, 0.2f, Rgba8::GREY);
}

bool Bomb::IsExploded() const
{
	return m_isExploded;
}

int Bomb::GetDamage() const
{
	return m_damage;
}

float Bomb::GetExplosionRadius() const
{
	return m_explosionRadius;
}

void Bomb::InitLocalVertexes()
{
	m_localVertexes = new Vertex_PCU[NUM_BOMB_QUADS * NUM_QUAD_VERTS];
	const int numTriangles = NUM_BOMB_QUADS * NUM_QUAD_VERTS / 3;
	float thetaIntervalDegrees = 360.0f / static_cast<float>(numTriangles);
	float startThetaDegrees = 0.0f;
	float endThetaDegrees = thetaIntervalDegrees;
	Vec2 centerPosition(0.0f, 0.0f);

	for (int count = 0; count < numTriangles; count++)
	{
		Vec2 position;

		m_localVertexes[count * 3 + 0].m_position = Vec2(centerPosition).GetVec3();
		m_localVertexes[count * 3 + 0].m_color = Bomb::BOMB_RED;

		m_localVertexes[count * 3 + 1].m_position = Vec2(Vec2::MakeFromPolarDegrees(startThetaDegrees, BOMB_COSMETIC_RADIUS)).GetVec3();
		m_localVertexes[count * 3 + 1].m_color = Bomb::BOMB_RED;

		m_localVertexes[count * 3 + 2].m_position = Vec2(Vec2::MakeFromPolarDegrees(endThetaDegrees, BOMB_COSMETIC_RADIUS)).GetVec3();
		m_localVertexes[count * 3 + 2].m_color = Bomb::BOMB_RED;

		startThetaDegrees += thetaIntervalDegrees;
		endThetaDegrees += thetaIntervalDegrees;
	}
}

