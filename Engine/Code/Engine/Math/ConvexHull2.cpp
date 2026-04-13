#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

int ConvexHull2::GetNumPlanes()
{
	return (uint32_t) m_unorderedPlanes.size();
}


Plane2 ConvexHull2::GetPlaneAtIndex(int index)
{
	if (index < 0 && index >= m_unorderedPlanes.size())
		ERROR_AND_DIE(Stringf("Out of bounds index for convex hull. Index: %i", index).c_str());

	return m_unorderedPlanes[index];
}


