#include "Game/Player.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

constexpr int PLAYER_NUM_PTS_NEON_TRAIL = 10;

Player::Player()
{
	Initialise();
}


Player::Player(Vec3 const& position)
	: Entity(position)
{
	Initialise();
}


Player::~Player()
{

}


void Player::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	m_neonGlowSpline.Clear();
	//calculate velocities for each hermite curve in the spline made up of the neon trail positions
	//using the catmull rom algorithm
	Vec2 splineVelocities[PLAYER_NUM_PTS_NEON_TRAIL] = {};
	for (int trailPosIndex = 0; trailPosIndex < m_neonTrailPositions.GetSize() - 2; trailPosIndex++)
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
	for (int trailPosIndex = 0; trailPosIndex < m_neonTrailPositions.GetSize() - 1; trailPosIndex++)
	{
		CubicHermiteCurve2D hermiteCurve = {};
		hermiteCurve.m_start = m_neonTrailPositions.GetNthElement(trailPosIndex).GetVec2();
		hermiteCurve.m_end = m_neonTrailPositions.GetNthElement(trailPosIndex + 1).GetVec2();
		hermiteCurve.m_startVelocity = splineVelocities[trailPosIndex];
		hermiteCurve.m_endVelocity = splineVelocities[trailPosIndex + 1];
		m_neonGlowSpline.AddCurveAtEnd(hermiteCurve);
	}
}


void Player::Render()
{
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(0, nullptr);

	std::vector<Vertex_PCU> verts;
	verts.reserve(1000);
	if (m_neonGlowSpline.GetNumCurves() > 0)
	{
		float splineStep = 0.01f;
		float splineT = 0.0f;
		while (splineT <= 1.0f)
		{
			float radius = RangeMap(splineT, 0.0f, 1.0f, 0.01f, 0.1f);
			Vec2 splinePt = m_neonGlowSpline.GetPointAtFraction(splineT);
			Vec3 splinePt3D = splinePt.GetVec3();
			Rgba8 trailColor = m_baseColor;
			trailColor.a = 150;
			AddVertsForDisk3D(verts, splinePt3D, radius, 16.0f, trailColor);
			splineT += splineStep;
		}
	}
	
	Mat44 modelMat = Mat44::CreateTranslation3D(m_curPosition);
	for (int vertIndex = 0; vertIndex < (int) m_localVerts.size(); vertIndex++)
	{
		Vec3 localToWorldPos = modelMat.TransformPosition3D(m_localVerts[vertIndex].m_position);
		Vertex_PCU localToWorldVert = m_localVerts[vertIndex];
		localToWorldVert.m_position = localToWorldPos;
		verts.push_back(localToWorldVert);
	}

	g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
}


void Player::DebugRender() const
{
}


void Player::Initialise()
{
	static float paddleBaseHeight = g_gameConfigBlackboard.GetValue("paddleBaseHeight", 0.5f);
	static float paddleBaseRadius = g_gameConfigBlackboard.GetValue("paddleBaseRadius", 1.5f);
	static float paddleHandleHeight = g_gameConfigBlackboard.GetValue("paddleHandleHeight", 1.5f);
	static float paddleHandleRadius = g_gameConfigBlackboard.GetValue("paddleHandleRadius", 0.5f);
	
	m_pushesEntity = true;
	m_pushedByEntity = false;
	m_type = ENTITY_TYPE_PLAYER;
	m_physicsRadius = paddleBaseRadius;

	//from palette
	m_baseColor = Rgba8(255, 0, 0);
	m_handleColor = Rgba8(0, 255, 0); 
	AddVertsForCylinderZ3DToVector(m_localVerts, Vec2::ZERO, FloatRange(0.0f, paddleBaseHeight), paddleBaseRadius, 32.0f, m_baseColor);
	AddVertsForCylinderZ3DToVector(m_localVerts, Vec2::ZERO, FloatRange(paddleBaseHeight, paddleBaseHeight + paddleHandleHeight), paddleHandleRadius, 32.0f, m_handleColor);

	m_neonTrailPositions.Resize(PLAYER_NUM_PTS_NEON_TRAIL);
}