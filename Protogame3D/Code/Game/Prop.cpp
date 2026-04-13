#include "Game/Prop.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/DevConsole.hpp"

Prop::Prop()
{
}


Prop::Prop(Vec3 const& position) : Entity(position)
{
}


Prop::~Prop()
{

}


void Prop::Update(float deltaSeconds)
{
	m_orientation.m_yaw += deltaSeconds * m_angularVelocity.x;
	m_orientation.m_pitch += deltaSeconds * m_angularVelocity.y;
	m_orientation.m_roll += deltaSeconds * m_angularVelocity.z;
}


void Prop::Render() const
{
	Mat44 modelMat = GetModelMatrix();
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetCullMode(CullMode::BACK);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(0, m_texture);

	g_theRenderer->SetModelMatrix(modelMat, m_tint);
	g_theRenderer->DrawVertexArray((int) m_localVertexes.size(), m_localVertexes.data());
}


void Prop::DebugRender() const
{
}


void Prop::SetVertexArray(std::vector<Vertex_PCU> const& vertices)
{
	m_localVertexes = vertices;
}


void Prop::SetAngularVelocity(Vec3 const& angularVelocity)
{
	m_angularVelocity = angularVelocity;
}


void Prop::SetTint(Rgba8 const& tint)
{
	m_tint = tint;
}


void Prop::SetTexture(Texture* texture)
{
	m_texture = texture;
}
