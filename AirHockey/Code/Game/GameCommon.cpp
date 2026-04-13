#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/Plane3.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/FloatRange.hpp"

bool g_debugMode = true;

constexpr float ARROW_HEAD_HEIGHT = 0.15f;
constexpr float ARROW_HEAD_BASE_RADIUS_MULTIPLIER = 1.5f;
constexpr float ARROW_HEAD_TOP_RADIUS = 0.01f;

void DrawLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color)
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

	Vec2 iBasis = line.GetNormalized();
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	TransformVertexArrayXY3D(NUM_VERTS_LINE, localVertexes, iBasis, jBasis, startPos);

	g_theRenderer->SetDiffuseTexture(nullptr);
	g_theRenderer->DrawVertexArray(NUM_VERTS_LINE, localVertexes);
}


void DrawRing(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color)
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


void AddVertsForLine3D(std::vector<Vertex_PCU>& verts, Vec3 const& lineStart, Vec3 const& lineEnd, Rgba8 const& color, float thickness)
{
	Vec3 lineUpVector = (lineEnd - lineStart).GetNormalized();
	Vec3 lineCenter = (lineEnd + lineStart) * 0.5f;
	float lineLength = (lineEnd - lineStart).GetLength();
	float halfLineLength = lineLength * 0.5f;
	float lineRadius = thickness * 0.5f;

	Mat44 lineModelMatrix = GetModelMatrixFromUpVector(lineUpVector, lineCenter);
	int lineVertStart = (int) verts.size();
	AddVertsForCylinderZ3DToVector(verts, Vec2(0.0f, 0.0f), FloatRange(-halfLineLength, halfLineLength), lineRadius, 4.0f, color);
	int numLineVerts = (int) verts.size() - lineVertStart;
	TransformVertexArray3D(numLineVerts, &verts[lineVertStart], lineModelMatrix);
}


void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, Vec3 const& arrowStart, Vec3 const& arrowEnd, Rgba8 const& color, float thickness)
{
	Vec3 lineUpVector = (arrowEnd - arrowStart).GetNormalized();
	Vec3 lineCenter = (arrowEnd + arrowStart) * 0.5f;
	float lineLength = (arrowEnd - arrowStart).GetLength();
	float halfLineLength = lineLength * 0.5f;
	float lineRadius = thickness * 0.5f;

	int lineVertStart = (int) verts.size();
	Mat44 lineModelMatrix = GetModelMatrixFromUpVector(lineUpVector, lineCenter);
	AddVertsForCylinderZ3DToVector(verts, Vec2(0.0f, 0.0f), FloatRange(-halfLineLength, halfLineLength - ARROW_HEAD_HEIGHT), lineRadius, 4.0f, color);
	AddVertsForCylinderZ3DToVector(verts, Vec2(0.0f, 0.0f), FloatRange(halfLineLength - ARROW_HEAD_HEIGHT, halfLineLength + ARROW_HEAD_HEIGHT),
		lineRadius * ARROW_HEAD_BASE_RADIUS_MULTIPLIER, ARROW_HEAD_TOP_RADIUS, 4.0f, color);
	int numLineVerts = (int) verts.size() - lineVertStart;
	TransformVertexArray3D(numLineVerts, &verts[lineVertStart], lineModelMatrix);
}


void AddVertsForDisk3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numSegments, Rgba8 const& color)
{
	float	thetaInterval = (DEGREES_360 / numSegments);
	float	startThetaDegrees = 0.0f;

	while (startThetaDegrees < DEGREES_360)
	{
		float endThetaDegrees = startThetaDegrees + thetaInterval;
		float cosStartTheta = CosDegrees(startThetaDegrees);
		float sinStartTheta = SinDegrees(startThetaDegrees);
		float cosEndTheta = CosDegrees(endThetaDegrees);
		float sinEndTheta = SinDegrees(endThetaDegrees);

		Vec3 topSegmentStartPoint = center + Vec3(radius * cosStartTheta, radius * sinStartTheta);
		Vec3 topSegmentEndPoint = center + Vec3(radius * cosEndTheta, radius * sinEndTheta);

		verts.push_back(Vertex_PCU(center, color, Vec2::ZERO));
		verts.push_back(Vertex_PCU(topSegmentStartPoint, color, Vec2::ZERO));
		verts.push_back(Vertex_PCU(topSegmentEndPoint, color, Vec2::ZERO));

		startThetaDegrees += thetaInterval;
	}
}


void AddVertsForNonUniformDisk3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius_xNorth, float radius_yWest, float numSegments, Rgba8 const& color)
{
	float	thetaInterval = (DEGREES_360 / numSegments);
	float	startThetaDegrees = 0.0f;

	while (startThetaDegrees < DEGREES_360)
	{
		float endThetaDegrees = startThetaDegrees + thetaInterval;
		float cosStartTheta = CosDegrees(startThetaDegrees);
		float sinStartTheta = SinDegrees(startThetaDegrees);
		float cosEndTheta = CosDegrees(endThetaDegrees);
		float sinEndTheta = SinDegrees(endThetaDegrees);

		Vec3 topSegmentStartPoint = center + Vec3(radius_xNorth * cosStartTheta, radius_yWest * sinStartTheta);
		Vec3 topSegmentEndPoint = center + Vec3(radius_xNorth * cosEndTheta, radius_yWest * sinEndTheta);

		verts.push_back(Vertex_PCU(center, color, Vec2::ZERO));
		verts.push_back(Vertex_PCU(topSegmentStartPoint, color, Vec2::ZERO));
		verts.push_back(Vertex_PCU(topSegmentEndPoint, color, Vec2::ZERO));

		startThetaDegrees += thetaInterval;
	}
}


Vec3 GetCurrentMousePos_ScreenToWorld(Mat44 const& cameraModelMatrix)
{
	float fovHalfAngleDegrees = FOV_ANGLE_DEGREES * 0.5f;
	float nearPlaneHalfHeight = Z_NEAR * TanDegrees(fovHalfAngleDegrees) /** 2.0f*/;
	float nearPlaneHalfWidth = nearPlaneHalfHeight * g_window->GetWindowConfig().m_aspectRatio;

	Vec3 cameraPos = cameraModelMatrix.GetTranslation3D();
	Vec3 cameraUp = cameraModelMatrix.GetKBasis3D();
	Vec3 cameraFwd = cameraModelMatrix.GetIBasis3D();
	Vec3 cameraLeft = cameraModelMatrix.GetJBasis3D();
	
	Vec2 normalizedScreenPos = g_window->GetCurrentNormalizedMouseScreenPosition();
	normalizedScreenPos = normalizedScreenPos * 2.0f - Vec2(1.0f, 1.0f);

	Vec3 ptOnNearPlane = cameraPos + cameraFwd * Z_NEAR - cameraLeft * normalizedScreenPos.x * nearPlaneHalfWidth + cameraUp * normalizedScreenPos.y * nearPlaneHalfHeight;
	Vec3 rayStart = ptOnNearPlane;
	Vec3 rayFwdNormal = (ptOnNearPlane - cameraPos).GetNormalized();
	float rayMaxDistance = 1000.0f;
	Plane3 gamePlane = {};
	gamePlane.m_distanceFromOrigin = 0.0f;
	gamePlane.m_fwdNormal = Vec3(0.0f, 0.0f, 1.0f);
	BaseRaycastResult3D hitResult = RaycastVsPlane3(rayStart, rayFwdNormal, rayMaxDistance, gamePlane);
	return hitResult.m_impactPoint;
}

Vec3 GetMousePos_ScreenToWorld(Mat44 const& cameraModelMatrix, IntVec2 const& refMousePos)
{
	float fovHalfAngleDegrees = FOV_ANGLE_DEGREES * 0.5f;
	float nearPlaneHeight = Z_NEAR * TanDegrees(fovHalfAngleDegrees) /** 2.0f*/;
	float nearPlaneWidth = nearPlaneHeight * g_window->GetWindowConfig().m_aspectRatio;

	Vec3 cameraPos = cameraModelMatrix.GetTranslation3D();
	Vec3 cameraUp = cameraModelMatrix.GetKBasis3D();
	Vec3 cameraFwd = cameraModelMatrix.GetIBasis3D();
	Vec3 cameraLeft = cameraModelMatrix.GetJBasis3D();

	Vec2 normalizedScreenPos = g_window->GetNormalizedMouseScreenPosition(refMousePos);
	normalizedScreenPos = normalizedScreenPos * 2.0f - Vec2(1.0f, 1.0f);

	Vec3 ptOnNearPlane = cameraPos + cameraFwd * Z_NEAR - cameraLeft * normalizedScreenPos.x * nearPlaneWidth + cameraUp * normalizedScreenPos.y * nearPlaneHeight;
	Vec3 rayStart = ptOnNearPlane;
	Vec3 rayFwdNormal = (ptOnNearPlane - cameraPos).GetNormalized();
	float rayMaxDistance = 1000.0f;
	Plane3 gamePlane = {};
	gamePlane.m_distanceFromOrigin = 0.0f;
	gamePlane.m_fwdNormal = Vec3(0.0f, 0.0f, 1.0f);
	BaseRaycastResult3D hitResult = RaycastVsPlane3(rayStart, rayFwdNormal, rayMaxDistance, gamePlane);
	return hitResult.m_impactPoint;
}
