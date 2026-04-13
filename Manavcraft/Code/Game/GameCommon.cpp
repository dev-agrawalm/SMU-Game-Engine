#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/World.hpp"
#include "Engine/Core/DebugRender.hpp"

constexpr int NUM_VERTS_LINE = 6;
constexpr int NUM_QUADS_RING = 32;
constexpr int NUM_VERTS_QUAD = 6;

std::thread* g_threads[NUM_THREADS] = {};

bool operator<(IntVec2 const& a, IntVec2 const& b)
{
	if (a.y < b.y)
		return true;

	if (a.y == b.y)
	{
		if (a.x < b.x)
			return true;
	}

	return false;
}


float GetMin(float a, float b, float c)
{
	float minAB = GetMin(a, b);
	float minABC = GetMin(minAB, c);
	return minABC;
}


WorldRaycastResult GetClosestImpactRaycast(WorldRaycastResult* raycasts, int numRaycasts)
{
	WorldRaycastResult closestImpactRaycast = {};
	float closesImpactDistance = 99999.0f;
	for (int raycastIndex = 0; raycastIndex < numRaycasts; raycastIndex++)
	{
		float impactDistance = raycasts[raycastIndex].m_impactDistance;
		bool didHit = raycasts[raycastIndex].m_didHit;
		if (didHit && impactDistance < closesImpactDistance)
		{
			closestImpactRaycast = raycasts[raycastIndex];
			closesImpactDistance = impactDistance;
		}
	}
	return closestImpactRaycast;
}


void VisualiseRaycast(WorldRaycastResult const& raycastToVisualise, float duration)
{
	Vec3 raycastStart = raycastToVisualise.m_startPos;
	Vec3 raycastForward = raycastToVisualise.m_forwardNormal;
	float raycastDistance = raycastToVisualise.m_maxDistance;

	bool didHit = raycastToVisualise.m_didHit;
	if (didHit)
	{
		Vec3 impactPos = raycastToVisualise.m_impactPoint;
		DebugAddWorldLine(raycastStart, impactPos, Rgba8::RED, 0.125f, duration, DebugRenderMode::XRAY);
		DebugAddWorldLine(impactPos, raycastStart + raycastForward * raycastDistance, Rgba8::GREEN, 0.125f, duration, DebugRenderMode::XRAY);
		DebugAddWorldWireSphere(impactPos, 0.05f, duration, Rgba8::CYAN, DebugRenderMode::XRAY);
	}
	else
	{
		DebugAddWorldLine(raycastStart, raycastStart + raycastForward * raycastDistance, Rgba8::RED, 0.125f, duration, DebugRenderMode::XRAY);
	}
}


void DrawLine2D(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color)
{
	Vertex_PCU localVertexes[NUM_VERTS_LINE];

	float originX = 0.0f;
	float originY = 0.0f;
	float halfWidth = width / 2.0f;
	float lenght = line.GetLength();

	for (int index = 0; index < NUM_VERTS_LINE; index++)
	{
		localVertexes[index].m_color = color;
	}

	//lower triangle
	localVertexes[0].m_position = Vec3(originX, originY - halfWidth);
	localVertexes[1].m_position = Vec3(originX + lenght, originY - halfWidth);
	localVertexes[2].m_position = Vec3(originX + lenght, originY + halfWidth);
	//upper triangle
	localVertexes[3].m_position = Vec3(originX + lenght, originY + halfWidth);
	localVertexes[4].m_position = Vec3(originX, originY + halfWidth);
	localVertexes[5].m_position = Vec3(originX, originY - halfWidth);

	Vec2 iBasis = line;
	Vec2 jBasis = line.GetRotated90Degrees();
	TransformVertexArrayXY3D(NUM_VERTS_LINE, localVertexes, iBasis, jBasis, startPos);

	g_theRenderer->SetDiffuseTexture(nullptr);
	g_theRenderer->DrawVertexArray(NUM_VERTS_LINE, localVertexes);
}


void DrawRing2D(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color)
{
	const int totalVertCount = NUM_QUADS_RING * NUM_VERTS_QUAD; //total vertexes = number of quads in a ring * number of vertexes in a single quad
	Vertex_PCU* localVertexes = new Vertex_PCU[totalVertCount];

	float thetaInterval = (360.0f / static_cast<float>(NUM_QUADS_RING)); //the angle at the center of the circle for each individual quad segment of the ring
	
	float startThetaDegrees = 0.0f; //angle of the starting line of our quad segment
	float endThetaDegrees = thetaInterval; //angle of the ending line of our quad segment
	float outerCircleRadius = radius + width;
	float innerCircleRadius = radius;
	int initVertexCount = 0; //number of vertexes in the vertex array that have been initialized

	//Outer Point 2 ....................... Outer Point 1      //Points on the outer circle of the ring
	//              .                     .
	//              .                     .                                //Diagram for a single quad in our ring (an approximation for a segment of the ring)
	//              .                     .
	//Inner Point 2 ....................... Inner Point 1      //Points on the inner circle of the ring
	
	while (initVertexCount < totalVertCount)                                 
	{						
		float cosStartTheta = CosDegrees(startThetaDegrees);
		float sinStartTheta = SinDegrees(startThetaDegrees);
		float cosEndTheta = CosDegrees(endThetaDegrees);
		float sinEndTheta = SinDegrees(endThetaDegrees);

		Vec2 outerPoint2 = Vec2(outerCircleRadius * cosEndTheta, outerCircleRadius * sinEndTheta);
		Vec2 outerPoint1 = Vec2(outerCircleRadius * cosStartTheta, outerCircleRadius * sinStartTheta);
		Vec2 innerPoint2 = Vec2(innerCircleRadius * cosEndTheta, innerCircleRadius * sinEndTheta);
		Vec2 innerPoint1 = Vec2(innerCircleRadius * cosStartTheta, innerCircleRadius * sinStartTheta);

		for (int index = initVertexCount; index < initVertexCount + NUM_VERTS_QUAD; index++)
		{
			localVertexes[index].m_color = color;
		}

		//lower triangle
		localVertexes[initVertexCount].m_position	  = Vec2(centerPosition + innerPoint2).GetVec3();
		localVertexes[initVertexCount + 1].m_position = Vec2(centerPosition + innerPoint1).GetVec3();
		localVertexes[initVertexCount + 2].m_position = Vec2(centerPosition + outerPoint1).GetVec3();
		//upper triangle
		localVertexes[initVertexCount + 3].m_position = Vec2(centerPosition + outerPoint1).GetVec3();
		localVertexes[initVertexCount + 4].m_position = Vec2(centerPosition + outerPoint2).GetVec3();
		localVertexes[initVertexCount + 5].m_position = Vec2(centerPosition + innerPoint2).GetVec3();

		initVertexCount += NUM_VERTS_QUAD;
		startThetaDegrees += thetaInterval;							 
		endThetaDegrees += thetaInterval;                             
	}

	g_theRenderer->SetDiffuseTexture(nullptr);
	g_theRenderer->DrawVertexArray(totalVertCount, localVertexes);

	delete[] localVertexes;
	localVertexes = nullptr;
}
