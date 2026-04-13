#pragma once
#include "Engine/Math/Vec2.hpp"
#include <vector>

struct ConvexHull2;

struct ConvexPoly2
{
public:
	void AddPoint(Vec2 const& pointToAdd);
	int GetNumPoints() const;
	Vec2 GetPointAtIndex(int index) const;
	
	ConvexHull2 GetAsConvexHull() const;
	void ClearAllPoints();
private:
	std::vector<Vec2> m_orderedPts;
	int m_numPts = 0;
};
