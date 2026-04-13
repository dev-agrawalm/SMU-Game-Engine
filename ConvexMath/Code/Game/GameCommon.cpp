#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Plane2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/ConvexPoly2.hpp"
#include "Engine/Math/RaycastUtils.hpp"

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
		localVertexes[initVertexCount].m_position = Vec2(centerPosition + innerPoint2).GetVec3();
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


void DrawArrow(Vec2 const& arrowStart, Vec2 const& arrowEnd, Rgba8 const& color, float width /*= 1.0f*/, float arrowTipLength /*= 1.0f*/)
{
	std::vector<Vertex_PCU> arrowVerts;
	AddVertsForArrow2DToVector(arrowVerts, arrowStart, arrowEnd, color, width, arrowTipLength, 0.0f);
	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) arrowVerts.size(), arrowVerts.data());
}


void AddVertsForConvexPoly2BorderToVector(std::vector<Vertex_PCU>& vertVector, ConvexPoly2 const& convexPoly2, Rgba8 const& color, float borderWidth /*= 0.5f*/)
{
	int numberPts = convexPoly2.GetNumPoints();
	for (int i = 0; i < numberPts; i++)
	{
		int index1 = i;
		int index2 = i + 1 >= numberPts ? 0 : i + 1;
		Vec2 pt1 = convexPoly2.GetPointAtIndex(index1);
		Vec2 pt2 = convexPoly2.GetPointAtIndex(index2);
		LineSegment2 borderLine = LineSegment2(pt1, pt2);
		AddVertsForLineSegment2DToVector(vertVector, borderLine, color, borderWidth, 0.0f);
	}
}


void DrawPlane2(Plane2 const& plane, float width, Rgba8 const& color, Rgba8 const& fwdNormalColor, bool drawFwdNormal/* = false*/)
{
	Vec2 planeNormal = plane.m_fwdNormal;
	float planeDistance = plane.m_distanceFromOrigin;
	Vec2 pt1 = planeNormal * planeDistance;
	Vec2 pt2;
	if (planeNormal.x == 0.0f)
	{
		pt2 = Vec2(10.0f, pt1.y);
	}
	else if (planeNormal.y == 0.0f)
	{
		pt2 = Vec2(pt1.x, 10.0f);
	}
	else
	{
		pt2 = Vec2(planeDistance / planeNormal.x, 0.0f);
	}
	LineSegment2 lineSegment = LineSegment2(pt1, pt2);
	lineSegment.StretchFromCenter(10000.0f);

	std::vector<Vertex_PCU> planeVerts;
	AddVertsForLineSegment2DToVector(planeVerts, lineSegment, color, width, 0.0f);
	if (drawFwdNormal)
	{
		AddVertsForArrow2DToVector(planeVerts, pt1, pt1 + planeNormal * ARROW_LENGTH, fwdNormalColor, width, ARROW_TIP_LENGTH, 0.0f);
	}

	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) planeVerts.size(), planeVerts.data());
}


void DrawConvexPoly2(ConvexPoly2 const& convexPoly, Rgba8 const& color, Rgba8 const& borderColor /*= Rgba8::BLACK*/, float borderWidth /*= 0.5f*/)
{
	int numberPts = convexPoly.GetNumPoints();
	GUARANTEE_OR_DIE(numberPts >= 3, "Convex poly should have atleast 3 points for drawing");
	
	std::vector<Vertex_PCU> convexPolyVerts;
	Vec2 pt0 = convexPoly.GetPointAtIndex(0);
	if (numberPts == 3)
	{
		Vec2 pt1 = convexPoly.GetPointAtIndex(1);
		Vec2 pt2 = convexPoly.GetPointAtIndex(2);
	
		Vertex_PCU vert1 = {};
		vert1.m_position = pt0.GetVec3();
		vert1.m_color = color;

		Vertex_PCU vert2 = {};
		vert2.m_position = pt1.GetVec3();
		vert2.m_color = color;

		Vertex_PCU vert3 = {};
		vert3.m_position = pt2.GetVec3();
		vert3.m_color = color;

		convexPolyVerts.push_back(vert1);
		convexPolyVerts.push_back(vert2);
		convexPolyVerts.push_back(vert3);
	}
	else
	{

		for (int i = 1; i <= convexPoly.GetNumPoints() - 2; i++)
		{
			Vec2 pt1 = convexPoly.GetPointAtIndex(i);
			Vec2 pt2 = convexPoly.GetPointAtIndex(i+1);

			Vertex_PCU vert1(pt0.GetVec3(), color);
			Vertex_PCU vert2(pt1.GetVec3(), color);
			Vertex_PCU vert3(pt2.GetVec3(), color);

			convexPolyVerts.push_back(vert1);
			convexPolyVerts.push_back(vert2);
			convexPolyVerts.push_back(vert3);
		}
	}

	//Draw borders for convex poly
	for (int i = 0; i < numberPts; i++)
	{
		int index1 = i;
		int index2 = i + 1 >= numberPts ? 0 : i + 1;
		Vec2 pt1 = convexPoly.GetPointAtIndex(index1);
		Vec2 pt2 = convexPoly.GetPointAtIndex(index2);
		LineSegment2 borderLine = LineSegment2(pt1, pt2);
		AddVertsForLineSegment2DToVector(convexPolyVerts, borderLine, borderColor, borderWidth, 0.0f);
	}

	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) convexPolyVerts.size(), convexPolyVerts.data());
}


void DrawRaycast(BaseRaycastResult2D const& raycastResult, float raycastLineWidth, Rgba8 const& preImpactColor /*= Rgba8::GREEN*/, Rgba8 const& postImpactColor /*= Rgba8::RED*/, Rgba8 const& impactNormalColor /*= Rgba8::BLUE*/)
{
	LineSegment2 preImpactLine = LineSegment2(raycastResult.m_startPos, raycastResult.m_impactPoint);
	LineSegment2 postImpactLine = LineSegment2(raycastResult.m_impactPoint, raycastResult.m_startPos + raycastResult.m_forwardNormal * raycastResult.m_maxDistance);
	std::vector<Vertex_PCU> raycastVerts;
	AddVertsForLineSegment2DToVector(raycastVerts, preImpactLine, preImpactColor, raycastLineWidth, 0.0f);
	AddVertsForLineSegment2DToVector(raycastVerts, postImpactLine, postImpactColor, raycastLineWidth, 0.0f);
	AddVertsForArrow2DToVector(raycastVerts, raycastResult.m_impactPoint, raycastResult.m_impactPoint + raycastResult.m_impactNormal * ARROW_LENGTH, impactNormalColor, raycastLineWidth, ARROW_TIP_LENGTH, 0.0f);

	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) raycastVerts.size(), raycastVerts.data());
}


BSPNode* BSPTree::GetNodeForPoint(Vec2 const& refPt)
{
	BSPNode* node = m_rootNode;
	if (node == nullptr)
		return node;

	int numIterations = 0;
	while (true)
	{
		if (node->m_frontNode == nullptr && node->m_backNode == nullptr)
		{
			return node;
		}

		Plane2 partitionPlane = node->m_partitionInfo.m_partitionPlane;
		if (partitionPlane.IsPointBehind(refPt))
		{
			node = node->m_backNode;
		}
		else if (partitionPlane.IsPointInFront(refPt))
		{
			node = node->m_frontNode;
		}

		numIterations++;
		if (numIterations > 100)
			return nullptr;
	}
}
