#include "Engine/Math/ConvexPoly2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Math/MathUtils.hpp"

void ConvexPoly2::AddPoint(Vec2 const& pointToAdd)
{
	m_orderedPts.push_back(pointToAdd);
	m_numPts++;
}


int ConvexPoly2::GetNumPoints() const
{
	return (int) m_orderedPts.size();
}


Vec2 ConvexPoly2::GetPointAtIndex(int index) const
{
	if (index < 0 && index >= m_orderedPts.size())
		ERROR_AND_DIE(Stringf("Out of bounds index for convex poly. Index: %i", index).c_str());

	return m_orderedPts[index];
}


ConvexHull2 ConvexPoly2::GetAsConvexHull() const
{
	ConvexHull2 convexHull = {};
	for (int i = 0; i < m_numPts; i++)
	{
		int index1 = i;
		int index2 = i + 1 == m_numPts ? 0 : i + 1;
		Vec2 pt1 = m_orderedPts[index1];
		Vec2 pt2 = m_orderedPts[index2];
		Vec2 lineVec = pt2 - pt1;
		Vec2 perpendicularLineVec = Vec2(lineVec.y, -lineVec.x);
		Vec2 planeNormal = perpendicularLineVec.GetNormalized();
		float planeDistance = DotProduct2D(pt1, planeNormal);
		Plane2 plane = Plane2(planeNormal, planeDistance);
		convexHull.m_unorderedPlanes.push_back(plane);
	}

	return convexHull;
}


void ConvexPoly2::ClearAllPoints()
{
	m_orderedPts.clear();
	m_numPts = 0;
}

