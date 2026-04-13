#include "Engine/Math/Plane3.hpp"
#include "Engine/Math/MathUtils.hpp"

float Plane3::GetShortestDistanceFromPlane(Vec3 const& refPt) const
{
	return DotProduct3D(refPt, m_fwdNormal) - m_distanceFromOrigin;
}
