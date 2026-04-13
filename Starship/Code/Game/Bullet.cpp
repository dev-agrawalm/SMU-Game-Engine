#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Game.hpp"

Bullet::Bullet(Game* game, Vec2 const&  position, float orientationDegrees) : Entity(game,position)
{
	m_orientationDegrees = orientationDegrees;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_damage = BULLET_DAMAGE;

	InitLocalVertexes();
}

Bullet::Bullet()
{
	//empty
}

Bullet::~Bullet()
{
	//empty
}

void Bullet::Update(float deltaSeconds)
{
	if (m_isDead)
	{
		m_isGarbage = true;
		return;
	}

	Vec2 forwardDir = GetForwardNormal();
	m_velocity = forwardDir * BULLET_SPEED;
	m_position += m_velocity * deltaSeconds;

	m_age += deltaSeconds;
	
	if (IsOffScreen() || m_age >= BULLET_LIFETIME_SECONDS)
	{
		m_isDead = true;
		m_isGarbage = true;
	}
}

void Bullet::Render() const
{
	if (m_isDead)
		return;

	Vertex_PCU tempVerts[NUM_BULLET_VERTS];

	for (int index = 0; index < NUM_BULLET_VERTS; index++)
	{
		tempVerts[index] = m_localVertexes[index];
	}

	TransformVertexArrayXY3D(NUM_BULLET_VERTS, tempVerts, 1.0f, m_orientationDegrees, m_position);

	g_theRenderer->DrawVertexArray(NUM_BULLET_VERTS, tempVerts);
}

void Bullet::Die()
{
	m_isDead = true;
	m_game->SetWorldCamScreenShakeParameters(BULLET_IMPACT_SCREEN_SHAKE_MAGNITUDE, BULLET_IMPACT_SCREEN_SHAKE_DURATION);
}

int Bullet::GetDamage() const
{
	return m_damage;
}

Rgba8 Bullet::GetDebrisColor() const
{
	return Rgba8::YELLOW;
}

void Bullet::InitLocalVertexes()
{
	m_localVertexes = new Vertex_PCU[NUM_BULLET_VERTS];

	//Front triangle
	m_localVertexes[0].m_position = Vec3(0.5f, 0.0f);
	m_localVertexes[0].m_color = Rgba8::YELLOW;
	m_localVertexes[1].m_position = Vec3(0.0f, 0.5f);
	m_localVertexes[1].m_color = Rgba8::YELLOW;
	m_localVertexes[2].m_position = Vec3(0.0f, -0.5f);
	m_localVertexes[2].m_color = Rgba8::YELLOW;
	//back triangle
	m_localVertexes[3].m_position = Vec3(0.0f, 0.5f);
	m_localVertexes[3].m_color = Rgba8::RED;
	m_localVertexes[4].m_position = Vec3(-2.0f, 0.0f);
	m_localVertexes[4].m_color = Rgba8::RED_TRANSPARENT; //transparent red vertex for a fading effect
	m_localVertexes[5].m_position = Vec3(0.0f, -0.5f);
	m_localVertexes[5].m_color = Rgba8::RED;

}
