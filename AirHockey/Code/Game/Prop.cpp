#include "Game/Prop.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "ThirdParty/ImGUI/imgui.h"

constexpr int PROP_NUM_PTS_NEON_TRAIL = 15;

Prop::Prop()
{
}


Prop::Prop(Vec3 const& position) : Entity(position)
{
	InitLocalVerts();
	//m_isDead = true;
}


Prop::~Prop()
{
	g_audioSystem->StopSound(m_puckSlidePlaybackId);
}


void Prop::Update(float deltaSeconds)
{
	if (m_showPuckControls)
	{
		ImGui::Begin("Puck Controls");
		ImGui::Text(Stringf("Current angular velocity degrees: %0.2f", m_angularVelocityDegrees).c_str());
		ImGui::Text("Q/E to modify angular velocity");
		ImGui::Text(Stringf("Velocity to apply: %0.2f, %0.2f, %0.2f", m_velocityToApply.x, m_velocityToApply.y, m_velocityToApply.z).c_str());
		ImGui::Text("Arrow keys to modify the velocity to be applied");
		ImGui::Text("F7 to apply velocity");
		ImGui::End();
	}
	
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F7))
	{
		m_velocity = m_velocityToApply;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ALT))
	{
		m_curPosition = Vec3::ZERO;
		m_velocity = Vec3::ZERO;
		m_orientationDegrees = 0.0f;
		m_angularVelocityDegrees = 0.0f;

		m_velocityToApply = Vec3::ZERO;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_LEFT_ARROW))
	{
		m_velocityToApply.x -= 2.0f;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_RIGHT_ARROW))
	{
		m_velocityToApply.x += 2.0f;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_UP_ARROW))
	{
		m_velocityToApply.y += 2.0f;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_DOWN_ARROW))
	{
		m_velocityToApply.y -= 2.0f;
	}

	if (g_inputSystem->IsKeyPressed('E'))
	{
		m_angularVelocityDegrees += 10.0f;
	}

	if (g_inputSystem->IsKeyPressed('Q'))
	{
		m_angularVelocityDegrees -= 10.0f;
	}

// 	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F9))
// 	{
// 		m_showPuckControls = !m_showPuckControls;
// 	}

	if (!m_isDead)
	{
		m_prevPos = m_curPosition;
		m_angularVelocityDegrees -= m_angularVelocityDegrees * TABLE_SURFACE_FRICTION * deltaSeconds;
		m_velocity -= m_velocity * TABLE_SURFACE_FRICTION * deltaSeconds;
		m_orientationDegrees += m_angularVelocityDegrees * deltaSeconds;
		m_curPosition += m_velocity * deltaSeconds;
		m_neonTrailPositions.Push(m_curPosition);
	}

	m_neonGlowSpline.Clear();
	Vec2 splineVelocities[PROP_NUM_PTS_NEON_TRAIL] = {};
	int numTrailPositions = m_neonTrailPositions.GetSize();
	for (int trailPosIndex = 0; trailPosIndex < numTrailPositions - 2; trailPosIndex++)
	{
		int n = trailPosIndex;
		int nPlus2 = n + 2;
		Vec3 trailPosN = m_neonTrailPositions.GetNthElement(n);
		Vec2 trailPosN_XY = trailPosN.GetVec2();
		Vec3 trailPosNPlus2 = m_neonTrailPositions.GetNthElement(nPlus2);
		Vec2 trailPosNPlus2_XY = trailPosNPlus2.GetVec2();

		int nPlus1 = n + 1;
		Vec2 velocityAtNPlus1 = (trailPosNPlus2_XY - trailPosN_XY) * 0.5f;
		splineVelocities[nPlus1] = velocityAtNPlus1;
	}

	//create the spline
	for (int trailPosIndex = 0; trailPosIndex < numTrailPositions - 1; trailPosIndex++)
	{
		CubicHermiteCurve2D hermiteCurve = {};
		hermiteCurve.m_start = m_neonTrailPositions.GetNthElement(trailPosIndex).GetVec2();
		hermiteCurve.m_end = m_neonTrailPositions.GetNthElement(trailPosIndex + 1).GetVec2();
		hermiteCurve.m_startVelocity = splineVelocities[trailPosIndex];
		hermiteCurve.m_endVelocity = splineVelocities[trailPosIndex + 1];
		m_neonGlowSpline.AddCurveAtEnd(hermiteCurve);
	}

	if (numTrailPositions > 0)
	{
		Vec2 finalPointVelocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_angularVelocityDegrees);
		splineVelocities[numTrailPositions - 1] = finalPointVelocity;
	}

	if (deltaSeconds > 0.0f)
	{
		float puckSlideVolume = RangeMap(m_velocity.GetLength(), 0.0f, 16.0f, 0.0f, g_gameSoundVolumes[SFX_PUCK_SLIDE_ON_TABLE]);
		g_audioSystem->UpdateSoundPositionAndVelocity(m_puckSlidePlaybackId, m_curPosition, m_velocity);
		g_audioSystem->SetSoundPlaybackVolume(m_puckSlidePlaybackId, puckSlideVolume);
	}
}


void Prop::Render()
{
	if (m_isDead)
		return;

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(0, m_texture);

	std::vector<Vertex_PCU> verts;
	verts.reserve(1000);
	if (m_neonGlowSpline.GetNumCurves() > 0)
	{
		float splineStep = 0.01f;
		float splineT = 0.0f;
		while (splineT <= 1.0f)
		{
			float radius = RangeMap(splineT, 0.0f, 1.0f, 0.05f, 0.25f);
			Vec2 splinePt = m_neonGlowSpline.GetPointAtFraction(splineT);
			Vec3 splinePt3D = splinePt.GetVec3();
			Rgba8 trailColor = m_tint;
			trailColor.a = 150;
			AddVertsForDisk3D(verts, splinePt3D, radius, 16.0f, trailColor);
			splineT += splineStep;
		}
	}

	Mat44 modelMat = GetModelMatrix();
	for (int vertIndex = 0; vertIndex < (int) m_localVerts.size(); vertIndex++)
	{
		Vec3 localToWorldPos = modelMat.TransformPosition3D(m_localVerts[vertIndex].m_position);
		Vertex_PCU localToWorldVert = m_localVerts[vertIndex];
		localToWorldVert.m_position = localToWorldPos;
		//localToWorldVert.m_color = m_tint;
		verts.push_back(localToWorldVert);
	}

	g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());

	if (g_debugMode)
	{
		DebugRender();
	}
}


void Prop::DebugRender() const
{
	Mat44 modelMat = GetModelMatrix();
	std::vector<Vertex_PCU> debugVerts;
//	AddVertsForArrow3D(debugVerts, m_curPosition, m_curPosition + modelMat.GetIBasis3D() * 1.5f, Rgba8::RED, 0.125f);
//	AddVertsForArrow3D(debugVerts, m_curPosition, m_curPosition + modelMat.GetJBasis3D() * 1.5f, Rgba8::GREEN, 0.125f);
// 	AddVertsForArrow3D(debugVerts, m_curPosition, m_curPosition + m_velocity, Rgba8::WHITE, 0.125f);
	g_theRenderer->SetModelMatrix(Mat44());
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) debugVerts.size(), debugVerts.data());
}


void Prop::SetVertexArray(std::vector<Vertex_PCU> const& vertices)
{
	m_localVerts = vertices;
}


void Prop::SetTint(Rgba8 const& tint)
{
	m_tint = tint;
}


void Prop::SetTexture(Texture* texture)
{
	m_texture = texture;
}


void Prop::InitLocalVerts()
{
	m_pushesEntity = false;
	m_pushedByEntity = true;
	m_bouncesOffWalls = true;
	m_angularVelocityDegrees =  0.0f;
	m_orientationDegrees = 0;
	m_velocity = Vec3(0.0f, 0.0f, 0.0f);
	m_type = ENTITY_TYPE_PUCK;
	m_physicsRadius = g_gameConfigBlackboard.GetValue("puckRadius", 0.5f);
	float puckHeight = g_gameConfigBlackboard.GetValue("puckHeight", 0.0f);
	UNUSED(puckHeight);
	m_tint = Rgba8(255, 229, 72, 255);
	//m_tint = Rgba8::WHITE;
	AddVertsForCylinderZ3DToVector(m_localVerts, Vec2::ZERO, FloatRange(0.0f, puckHeight), m_physicsRadius, 32.0f, m_tint);
	//AddVertsForDisk3D(m_localVerts, Vec3::ZERO, m_physicsRadius, 32.0f, m_tint);
	AddVertsForNonUniformDisk3D(m_localVerts, Vec3::ZERO, 0.45f, 0.65f, 16.0f, m_tint);

	m_neonTrailPositions.Resize(PROP_NUM_PTS_NEON_TRAIL);

	m_puckSlidePlaybackId = g_audioSystem->PlaySoundAt(g_gameSounds[SFX_PUCK_SLIDE_ON_TABLE], m_curPosition, Vec3::ZERO, 0.0f, true);
}
