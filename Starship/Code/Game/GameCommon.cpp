#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

void DebugDrawLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color)
{
	Vertex_PCU localVertexes[NUM_LINE_VERTS];

	float originX = 0.0f;
	float originY = 0.0f;
	float halfWidth = width / 2.0f;
	float lenght = line.GetLength();

	for (int index = 0; index < NUM_LINE_VERTS; index++)
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

	float orientationAngle = line.GetOrientationDegrees();

	TransformVertexArrayXY3D(NUM_LINE_VERTS, localVertexes, 1.0f, orientationAngle, startPos);

	g_theRenderer->DrawVertexArray(NUM_LINE_VERTS, localVertexes);
}

void DebugDrawRing(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color)
{
	const int totalVertCount = NUM_RING_QUADS * NUM_QUAD_VERTS; //total vertexes = number of quads in a ring * number of vertexes in a single quad
	Vertex_PCU* localVertexes = new Vertex_PCU[totalVertCount];

	float thetaInterval = (360.0f / static_cast<float>(NUM_RING_QUADS)); //the angle at the center of the circle for each individual quad segment of the ring
	
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

		for (int index = initVertexCount; index < initVertexCount + NUM_QUAD_VERTS; index++)
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

		initVertexCount += NUM_QUAD_VERTS;
		startThetaDegrees += thetaInterval;							 
		endThetaDegrees += thetaInterval;                             
	}

	g_theRenderer->DrawVertexArray(totalVertCount, localVertexes);

	delete[] localVertexes;
	localVertexes = nullptr;
}


void DrawDottedLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color, int numDots)
{
	Vertex_PCU localVertexes[NUM_LINE_VERTS];

// 	float originX = 0.0f;
// 	float originY = 0.0f;
// 	float halfWidth = width / 2.0f;
//	float lenght = line.GetLength();
	float dotLength = 0.8f;
	float dotSpace = 0.45f;
	Vec2 lineNormal = line.GetNormalized();
	for (int dotIndex = 0; dotIndex < numDots; dotIndex++)
	{
		Vec2 startPosition = startPos + dotIndex * (dotLength + dotSpace) * lineNormal;
		Vec2 endPosition = startPosition + dotLength * lineNormal;
		Vec2 dotLine = endPosition - startPosition;
		DebugDrawLine(startPosition, dotLine, width, color);
	}

// 	for (int index = 0; index < NUM_LINE_VERTS; index++)
// 	{
// 		localVertexes[index].m_color = color;
// 	}
// 
// 	//lower triangle
// 	localVertexes[0].m_position = Vec3(originX, originY - halfWidth);
// 	localVertexes[1].m_position = Vec3(originX + lenght, originY - halfWidth);
// 	localVertexes[2].m_position = Vec3(originX + lenght, originY + halfWidth);
// 	//upper triangle
// 	localVertexes[3].m_position = Vec3(originX + lenght, originY + halfWidth);
// 	localVertexes[4].m_position = Vec3(originX, originY + halfWidth);
// 	localVertexes[5].m_position = Vec3(originX, originY - halfWidth);
// 
// 	float orientationAngle = line.GetOrientationDegrees();
// 
// 	TransformVertexArrayXY3D(NUM_LINE_VERTS, localVertexes, 1.0f, orientationAngle, startPos);
// 
// 	g_theRenderer->DrawVertexArray(NUM_LINE_VERTS, localVertexes);
}
