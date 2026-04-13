#pragma once
#include "Engine/Math/Plane2.hpp"
#include <vector>

struct ConvexHull2
{
public:
	int GetNumPlanes();
	Plane2 GetPlaneAtIndex(int index);

public:
	std::vector<Plane2> m_unorderedPlanes;
};
