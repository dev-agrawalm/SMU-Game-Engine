#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Plane2.hpp"
#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Math/Plane3.hpp"

BaseRaycastResult2D RaycastVsDisk2D(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, Vec2 const& diskCenter, float diskRadius)
{
	BaseRaycastResult2D result = {};

	Vec2 s = rayStart;
	Vec2 c = diskCenter;

	Vec2 rayIBasis = rayForwardNormal;
	Vec2 rayJBasis = Vec2(-rayIBasis.y, rayIBasis.x);
	Vec2 sc = c - s;
	float scLengthAlongJ = DotProduct2D(sc, rayJBasis);
	if (scLengthAlongJ > diskRadius || scLengthAlongJ < -diskRadius)
	{
		return result;
	}

	float scLengthAlongI = DotProduct2D(sc, rayIBasis);
	if (scLengthAlongI > rayDistance + diskRadius || scLengthAlongI < -diskRadius)
	{
		return result;
	}

	float scLengthaSqrd = sc.GetLengthSquared();
	if (scLengthaSqrd < Square(diskRadius))
	{
		result.m_didHit = true;
		result.m_impactPoint = s;
		result.m_impactNormal = -rayForwardNormal;
		result.m_impactDistance = (result.m_impactPoint - s).GetLength();
		return result;
	}

	Vec2 impactPoint; //reference for what i is
	Vec2& i = impactPoint;
	UNUSED(i);
	float icLength = diskRadius;

	Vec2 pointOnRayNearestToDisk; //reference for what p is
	Vec2& p = pointOnRayNearestToDisk;
	UNUSED(p);

	float spLength = scLengthAlongI;
	float cpLength = scLengthAlongJ;
	float ipLength = (float) sqrt(icLength * icLength - cpLength * cpLength);
	float& m = ipLength;
	float siLength = spLength - m;
	if (siLength > rayDistance || siLength < 0.0f)
	{
		return result;
	}

	result.m_didHit = true;
	result.m_impactPoint = s + rayForwardNormal * siLength;
	result.m_impactDistance = (result.m_impactPoint - s).GetLength();
	result.m_impactNormal = (result.m_impactPoint - c).GetNormalized();
	return result;
}


BaseRaycastResult2D RaycastVsAABB2(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, AABB2 const& aabb2)
{
	BaseRaycastResult2D result = {};

	float boxMinX = aabb2.m_mins.x;
	float boxMaxX = aabb2.m_maxs.x;

	float boxMinY = aabb2.m_mins.y;
	float boxMaxY = aabb2.m_maxs.y;

	Vec2 start = rayStart;
	Vec2 end = rayStart + rayForwardNormal * rayDistance;
	FloatRange rayXRange = FloatRange(rayStart.x, end.x);
	FloatRange rayYRange = FloatRange(rayStart.y, end.y);

	float rayTXBoxMin = GetFraction(boxMinX, rayXRange.m_min, rayXRange.m_max);
	float rayTXBoxMax = GetFraction(boxMaxX, rayXRange.m_min, rayXRange.m_max);
	FloatRange tRangeBoxX;
	if(rayTXBoxMin <= rayTXBoxMax)
		tRangeBoxX = FloatRange(rayTXBoxMin, rayTXBoxMax);
	else 
		tRangeBoxX = FloatRange(rayTXBoxMax, rayTXBoxMin);

	float rayTYBoxMin = GetFraction(boxMinY, rayYRange.m_min, rayYRange.m_max);
	float rayTYBoxMax = GetFraction(boxMaxY, rayYRange.m_min, rayYRange.m_max);
	FloatRange tRangeBoxY;
	if (rayTYBoxMin <= rayTYBoxMax)
		tRangeBoxY = FloatRange(rayTYBoxMin, rayTYBoxMax);
	else if(rayTYBoxMin >= rayTYBoxMax)
		tRangeBoxY = FloatRange(rayTYBoxMax, rayTYBoxMin);

	if (rayYRange.m_min == rayYRange.m_max && rayXRange.m_min != rayXRange.m_max)
	{
		if (tRangeBoxX.m_min > 0.0f && tRangeBoxX.m_min < 1.0f && FloatRange(boxMinY, boxMaxY).IsOnRange(rayYRange.m_min))
		{
			float impactT = tRangeBoxX.m_min;
			result.m_didHit = true;
			result.m_impactPoint = rayStart + rayForwardNormal * rayDistance * impactT;
			result.m_impactDistance = rayDistance * impactT;

			if (rayStart.x <= boxMinX)
			{
				result.m_impactNormal = (aabb2.m_mins - Vec2(boxMaxX, boxMinY)).GetNormalized();
			}
			else if (rayStart.x >= boxMinX)
			{
				result.m_impactNormal = (Vec2(boxMaxX, boxMinY) - aabb2.m_mins).GetNormalized();
			}
			return result;
		}
	}
	else if (rayYRange.m_min != rayYRange.m_max && rayXRange.m_min == rayXRange.m_max && FloatRange(boxMinX, boxMaxX).IsOnRange(rayXRange.m_min))
	{
		if (tRangeBoxY.m_min > 0.0f && tRangeBoxY.m_min < 1.0f)
		{
			float impactT = tRangeBoxY.m_min;
			result.m_didHit = true;
			result.m_impactPoint = rayStart + rayForwardNormal * rayDistance * impactT;
			result.m_impactDistance = rayDistance * impactT;

			if (rayStart.y <= boxMinY)
			{
				result.m_impactNormal = (aabb2.m_mins - Vec2(boxMinX, boxMaxY)).GetNormalized();
			}
			else if (rayStart.y >= boxMinY)
			{
				result.m_impactNormal = (Vec2(boxMinX, boxMaxY) - aabb2.m_mins).GetNormalized();
			}
			return result;
		}
	}

	bool doTRangesOverlap = tRangeBoxX.IsOverlappingWith(tRangeBoxY);
	if (!doTRangesOverlap)
	{
		return result;
	}

	if (IsPointInsideAABB2D(rayStart, aabb2))
	{
		result.m_didHit = true;
		result.m_impactPoint = rayStart;
		result.m_impactNormal = -rayForwardNormal;
		result.m_impactDistance = 0.0f;
		return result;
	}
	
	float impactT = tRangeBoxX.m_min > tRangeBoxY.m_min ? tRangeBoxX.m_min : tRangeBoxY.m_min;
	if (impactT < 0.0f || impactT > 1.0f)
	{
		return result;
	}

	result.m_didHit = true;
	result.m_impactPoint = rayStart + rayForwardNormal * rayDistance * impactT;
	result.m_impactDistance = rayDistance * impactT;
	if (impactT == tRangeBoxX.m_min) //impact was on the vertical lines of aabb2
	{
		if (rayStart.x <= boxMinX)
		{
			result.m_impactNormal = (aabb2.m_mins - Vec2(boxMaxX, boxMinY)).GetNormalized();
		}
		else if (rayStart.x >= boxMinX)
		{
			result.m_impactNormal = (Vec2(boxMaxX, boxMinY) - aabb2.m_mins).GetNormalized();
		}
	}
	else if (impactT == tRangeBoxY.m_min) //impact was on the horizontal lines of aabb2
	{
		if (rayStart.y <= boxMinY)
		{
			result.m_impactNormal = (aabb2.m_mins - Vec2(boxMinX, boxMaxY)).GetNormalized();
		}
		else if (rayStart.y >= boxMinY)
		{
			result.m_impactNormal = (Vec2(boxMinX, boxMaxY) - aabb2.m_mins).GetNormalized();
		}
	}

	return result;
}


BaseRaycastResult2D RaycastVsLineSegment2(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, LineSegment2 const& line)
{
	BaseRaycastResult2D result = {};

	Vec2 s = rayStart;
	Vec2 rayIBasis = rayForwardNormal;
	Vec2 rayJBasis = rayIBasis.GetRotated90Degrees();

	Vec2 a = line.m_start;
	Vec2 b = line.m_end;

	Vec2 sa = a - s;
	Vec2 sb = b - s;

	float saJComponent = DotProduct2D(sa, rayJBasis);
	float sbJComponent = DotProduct2D(sb, rayJBasis);
	if (sbJComponent * saJComponent > 0.0f)
		return result;

	float lineSegmentTForPoI = saJComponent / (saJComponent - sbJComponent); //PoI = point of intersection
	if (lineSegmentTForPoI <= 0.0f || lineSegmentTForPoI >= 1.0f)
		return result;

	float saIComponent = DotProduct2D(sa, rayIBasis);
	float sbIComponent = DotProduct2D(sb, rayIBasis);
	float lineSegmentDistanceAlongIBasis = sbIComponent - saIComponent;
	float poiIComponent = saIComponent + lineSegmentTForPoI * lineSegmentDistanceAlongIBasis;

	if (poiIComponent <= 0.0f || poiIComponent >= rayDistance)
		return result;
	
	Vec2 poi = rayStart + poiIComponent * rayIBasis;
	result.m_didHit = true;
	result.m_impactPoint = poi;
	result.m_impactDistance = poiIComponent;

	Vec2 lineDirection = line.GetDirection();
	Vec2 normalToLine = lineDirection.GetRotated90Degrees();
	if (DotProduct2D(normalToLine, rayIBasis) > 0.0f)
	{
		normalToLine *= -1.0f;
	}
	result.m_impactNormal = normalToLine;

	return result;
}


BaseRaycastResult2D RaycastVsPlane2(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, Plane2 const& plane)
{
	BaseRaycastResult2D result = {};
	result.m_forwardNormal = rayForwardNormal;
	result.m_startPos = rayStart;
	result.m_maxDistance = rayDistance;

	Vec2 rayEnd = rayStart + rayForwardNormal * rayDistance;
	float rayStartAltitude = plane.GetShortestDistanceFromPlane(rayStart);
	float rayEndAltitude = plane.GetShortestDistanceFromPlane(rayEnd);

	if (rayEndAltitude * rayStartAltitude >= 0.0f)
		return result;

	result.m_didHit = true;
	if (rayStartAltitude > 0.0f)
	{
		result.m_impactNormal = plane.m_fwdNormal;
	}
	else
	{
		result.m_impactNormal = -plane.m_fwdNormal;
	}

	result.m_impactDistance = (rayStartAltitude * rayDistance) / DotProduct2D((rayForwardNormal * rayDistance), -plane.m_fwdNormal);
	result.m_impactPoint = rayStart + rayForwardNormal * result.m_impactDistance;
	return result;
}


BaseRaycastResult2D RaycastVsConvexHull2(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, ConvexHull2 const& convexHull)
{
	BaseRaycastResult2D result = {};
	result.m_startPos = rayStart;
	result.m_forwardNormal = rayForwardNormal;
	result.m_maxDistance = rayDistance;

	float furthestEntryDistance = -9999999.0f;
	Vec2 furthestEntryPt = Vec2::ZERO;
	Plane2 furthestEntryPlane;
	float nearestExitDistance = 999999.0f;
	Vec2 nearestExitPt = Vec2::ZERO;

	bool didEnterHull = false;
	bool didExitHull = false;
	std::vector<Plane2> const& planes = convexHull.m_unorderedPlanes;
	for (int i = 0; i < (int) planes.size(); i++)
	{
		Plane2 const& plane = planes[i];
		BaseRaycastResult2D planeRaycast = RaycastVsPlane2(rayStart, rayForwardNormal, rayDistance, plane);
		if (planeRaycast.m_didHit)
		{
			Vec2 planeNormal = plane.m_fwdNormal;
			Vec2 impactNormal = planeRaycast.m_impactNormal;
			float impactDistance = planeRaycast.m_impactDistance;
			Vec2 impactPoint = planeRaycast.m_impactPoint;
			bool isEntry = planeNormal == impactNormal;
			if (isEntry)
			{
				if (impactDistance > furthestEntryDistance)
				{
					furthestEntryPt = planeRaycast.m_impactPoint;
					furthestEntryDistance = impactDistance;
					furthestEntryPlane = plane;
				}
				didEnterHull = true;
			}
			else
			{
				if (impactDistance < nearestExitDistance)
				{
					nearestExitPt = planeRaycast.m_impactPoint;
					nearestExitDistance = impactDistance;
				}
				didExitHull = true;
			}
		}
	}

	if (!didEnterHull)
	{
		if (IsPointInsideConvexHull2(rayStart, convexHull))
		{
			result.m_didHit = true;
			result.m_impactDistance = 0.0f;
			result.m_impactNormal = -rayForwardNormal;
			result.m_impactPoint = rayStart;
			return result;
		}

		return result;
	}

	if (didExitHull)
	{
		if (furthestEntryDistance < nearestExitDistance)
		{
			if (IsPointInsideConvexHull2(furthestEntryPt, convexHull, furthestEntryPlane))
			{
				result.m_didHit = true;
				result.m_impactPoint = furthestEntryPt;
				result.m_impactNormal = furthestEntryPlane.m_fwdNormal;
				result.m_impactDistance = furthestEntryDistance;
				return result;
			}
		}
		
		return result;
	}
	else
	{
		if (IsPointInsideConvexHull2(furthestEntryPt, convexHull, furthestEntryPlane))
		{
			result.m_didHit = true;
			result.m_impactPoint = furthestEntryPt;
			result.m_impactNormal = furthestEntryPlane.m_fwdNormal;
			result.m_impactDistance = furthestEntryDistance;
			return result;
		}

		return result;
	}
}


BaseRaycastResult3D RaycastVsZSphere3D(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance, Vec3 const& sphereCenter, float sphereRadius)
{
	BaseRaycastResult3D result = {};
	result.m_startPos = rayStart;
	result.m_maxDistance = rayDistance;
	result.m_forwardNormal = rayForwardNormal;

	Vec3 s = rayStart;
	Vec3 c = sphereCenter;
	Vec3 sc = c - s;

	Vec3 rayIBasis = rayForwardNormal;
	Vec3 scIComponent = GetProjectedOnto3D(sc, rayIBasis);
	Vec3 rayJKBasis = (sc - scIComponent).GetNormalized();
	float scLengthAlongJK = DotProduct3D(sc, rayJKBasis);
	if (scLengthAlongJK > sphereRadius || scLengthAlongJK < -sphereRadius)
	{
		return result;
	}

	float scLengthAlongI = DotProduct3D(sc, rayIBasis);
	if (scLengthAlongI > rayDistance + sphereRadius || scLengthAlongI < -sphereRadius)
	{
		return result;
	}

	float scLengthaSqrd = sc.GetLengthSquared();
	if (scLengthaSqrd < Square(sphereRadius))
	{
		result.m_didHit = true;
		result.m_impactPoint = s;
		result.m_impactNormal = -rayForwardNormal;
		result.m_impactDistance = (result.m_impactPoint - s).GetLength();
		result.m_impactFraction = result.m_impactDistance / rayDistance;
		return result;
	}

	Vec2 impactPoint; //reference for what i is
	Vec2& i = impactPoint;
	UNUSED(i);
	float icLength = sphereRadius;

	Vec2 pointOnRayNearestToDisk; //reference for what p is
	Vec2& p = pointOnRayNearestToDisk;
	UNUSED(p);

	float spLength = scLengthAlongI;
	float cpLength = scLengthAlongJK;
	float ipLength = (float) sqrt(icLength * icLength - cpLength * cpLength);
	float& m = ipLength;
	float siLength = spLength - m;
	if (siLength > rayDistance || siLength < 0.0f)
	{
		return result;
	}

	result.m_didHit = true;
	result.m_impactPoint = s + rayForwardNormal * siLength;
	result.m_impactDistance = (result.m_impactPoint - s).GetLength();
	result.m_impactNormal = (result.m_impactPoint - c).GetNormalized();
	result.m_impactFraction = result.m_impactDistance / rayDistance;
	return result;
}


BaseRaycastResult3D RaycastVsAABB3(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance, AABB3 const& aabb3)
{
	//todo handle parallel lines
	BaseRaycastResult3D result = {};
	result.m_startPos = rayStart;
	result.m_maxDistance = rayDistance;
	result.m_forwardNormal = rayForwardNormal;

	Vec3 rayEnd = rayStart + rayForwardNormal * rayDistance;

	float cubeMinX = aabb3.m_mins.x;
	float cubeMinY = aabb3.m_mins.y;
	float cubeMinZ = aabb3.m_mins.z;

	float cubeMaxX = aabb3.m_maxs.x;
	float cubeMaxY = aabb3.m_maxs.y;
	float cubeMaxZ = aabb3.m_maxs.z;

	float cubeMinXRayT = GetFraction(cubeMinX, rayStart.x, rayEnd.x);
	float cubeMaxXRayT = GetFraction(cubeMaxX, rayStart.x, rayEnd.x);
	FloatRange cubeXTRange = FloatRange(cubeMinXRayT, cubeMaxXRayT);
	if (cubeXTRange.m_max < 0.0f || cubeXTRange.m_min > 1.0f)
		return result;

	float cubeMinYRayT = GetFraction(cubeMinY, rayStart.y, rayEnd.y);
	float cubeMaxYRayT = GetFraction(cubeMaxY, rayStart.y, rayEnd.y);
	FloatRange cubeYTRange = FloatRange(cubeMinYRayT, cubeMaxYRayT);
	if (cubeYTRange.m_max < 0.0f || cubeYTRange.m_min > 1.0f)
		return result;

	float cubeMinZRayT = GetFraction(cubeMinZ, rayStart.z, rayEnd.z);
	float cubeMaxZRayT = GetFraction(cubeMaxZ, rayStart.z, rayEnd.z);
	FloatRange cubeZTRange = FloatRange(cubeMinZRayT, cubeMaxZRayT);
	if (cubeZTRange.m_max < 0.0f || cubeZTRange.m_min > 1.0f)
		return result;

	bool areTRangesOverlapping = true;
	areTRangesOverlapping &= cubeXTRange.IsOverlappingWith(cubeYTRange);
	areTRangesOverlapping &= cubeYTRange.IsOverlappingWith(cubeZTRange);
	areTRangesOverlapping &= cubeZTRange.IsOverlappingWith(cubeXTRange);
	if (!areTRangesOverlapping)
		return result;

	if (IsPointInsideAABB3(rayStart, aabb3))
	{
		result.m_didHit = true;
		result.m_impactPoint = rayStart;
		result.m_impactNormal = -rayForwardNormal;
		result.m_impactDistance = 0.0f;
		result.m_impactFraction = 0.0f;
		return result;
	}

	float impactTX = cubeXTRange.m_min;
	float impactTY = cubeYTRange.m_min;
	float impactTZ = cubeZTRange.m_min;
	
	float impactT = impactTX;
	if (impactTY > impactT)
		impactT = impactTY;
	if (impactTZ > impactT)
		impactT = impactTZ;

	
	if (impactT == impactTX)
	{
		if (rayStart.x >= cubeMaxX)
		{
			result.m_impactNormal = Vec3(1.0f, 0.0f, 0.0f);
		}
		else if (rayStart.x <= cubeMinX)
		{
			result.m_impactNormal = Vec3(-1.0f, 0.0f, 0.0f);
		}
	}
	else if (impactT == impactTY)
	{
		if (rayStart.y >= cubeMaxY)
		{
			result.m_impactNormal = Vec3(0.0f, 1.0f, 0.0f);
		}
		else if (rayStart.y <= cubeMinY)
		{
			result.m_impactNormal = Vec3(0.0f, -1.0f, 0.0f);
		}
	}
	else
	{
		if (rayStart.z >= cubeMaxZ)
		{
			result.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
		}
		else if (rayStart.z <= cubeMinZ)
		{
			result.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
		}
	}

	result.m_impactPoint = rayStart + rayForwardNormal * rayDistance * impactT;
	result.m_didHit = true;
	result.m_impactDistance = rayDistance * impactT;
	result.m_impactFraction = impactT;
	return result;
}


BaseRaycastResult3D RaycastVsZCylinder3D(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance, Vec2 const& cylinderCenter, FloatRange const& minMaxZ, float cylinderRadius)
{
	BaseRaycastResult3D result = {};
	result.m_startPos = rayStart;
	result.m_maxDistance = rayDistance;
	result.m_forwardNormal = rayForwardNormal;

	Vec2 rayStartXY = rayStart.GetVec2();
	Vec2 rayFwdNormalXY = rayForwardNormal.GetVec2().GetNormalized();
	float rayDistanceXY = DotProduct3D(rayForwardNormal * rayDistance, rayFwdNormalXY.GetVec3());
	BaseRaycastResult2D xyRaycastResult = RaycastVsDisk2D(rayStartXY, rayFwdNormalXY, rayDistanceXY, cylinderCenter, cylinderRadius);
	if (!xyRaycastResult.m_didHit)
	{
		return result;
	}

	float xyImpactDistance = xyRaycastResult.m_impactDistance;
	float xyImpactT = xyImpactDistance / rayDistanceXY;
	float xyExitT = 0.0f;
	if (xyImpactDistance > 0.0f)
	{
		Vec2 impactXYToCenter = cylinderCenter - xyRaycastResult.m_impactPoint;
		Vec2& icXY = impactXYToCenter;
		float icXYProjectedOnRayFwdXYLength = DotProduct2D(icXY, rayFwdNormalXY);
		xyExitT = (xyImpactDistance + 2.0f * icXYProjectedOnRayFwdXYLength) / rayDistanceXY;
	}
	else
	{
		Vec2 impactXYToCenter = cylinderCenter - xyRaycastResult.m_impactPoint;
		Vec2& icXY = impactXYToCenter;
		float icXYProjectedOnRayFwdXYLength = DotProduct2D(icXY, rayFwdNormalXY);
		float pSqrd = impactXYToCenter.GetLengthSquared() - Square(icXYProjectedOnRayFwdXYLength);
		float qSqrd = Square(cylinderRadius) - pSqrd;
		float q = SqrtFloat(qSqrd);
		xyExitT = (icXYProjectedOnRayFwdXYLength + q) / rayDistanceXY;
	}
	FloatRange xyTRange = FloatRange(xyImpactT, xyExitT);

	Vec3 rayEnd = rayStart + rayForwardNormal * rayDistance;
	FloatRange zTRange = FloatRange::ZERO_TO_ONE;
	if (rayStart.z != rayEnd.z)
	{
		float rayTForCylinderMinZ = GetFraction(minMaxZ.m_min, rayStart.z, rayEnd.z);
		float rayTForCylinderMaxZ = GetFraction(minMaxZ.m_max, rayStart.z, rayEnd.z);
		zTRange = FloatRange(rayTForCylinderMinZ, rayTForCylinderMaxZ);
	}
	else if (rayStart.z > minMaxZ.m_max || rayStart.z < minMaxZ.m_min)
	{
		return result;
	}

	if (zTRange.m_max < 0.0f || zTRange.m_min > 1.0f)
		return result;

	bool areTRangesOverlapping = xyTRange.IsOverlappingWith(zTRange);
	if (!areTRangesOverlapping)
		return result;

	if (IsPointInsideZCylinder3D(rayStart, cylinderCenter, minMaxZ, cylinderRadius))
	{
		result.m_didHit = true;
		result.m_impactPoint = rayStart;
		result.m_impactDistance = 0.0f;
		result.m_impactNormal = -rayForwardNormal;
		result.m_impactFraction = 0.0f;
		return result;
	}

	float impactT = xyTRange.m_min;
	if (zTRange.m_min > xyTRange.m_min)
		impactT = zTRange.m_min;

	result.m_didHit = true;
	result.m_impactPoint = rayStart + rayForwardNormal * rayDistance * impactT;
	result.m_impactDistance = rayDistance * impactT;
	if (impactT == zTRange.m_min)
	{
		if (rayStart.z <= minMaxZ.m_min)
		{
			result.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
		}
		else
		{
			result.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
		}
	}
	else
	{
		Vec2 impactNormalXY = (xyRaycastResult.m_impactPoint - cylinderCenter).GetNormalized();
		result.m_impactNormal = impactNormalXY.GetVec3();
	}
	result.m_impactFraction = impactT;
	return result;
}


BaseRaycastResult3D RaycastVsPlane3(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance, Plane3 const& plane)
{
	BaseRaycastResult3D result = {};
	result.m_forwardNormal = rayForwardNormal;
	result.m_startPos = rayStart;
	result.m_maxDistance = rayDistance;

	Vec3 rayEnd = rayStart + rayForwardNormal * rayDistance;
	float rayStartAltitude = plane.GetShortestDistanceFromPlane(rayStart);
	float rayEndAltitude = plane.GetShortestDistanceFromPlane(rayEnd);

	if (rayEndAltitude * rayStartAltitude >= 0.0f)
		return result;

	result.m_didHit = true;
	if (rayStartAltitude > 0.0f)
	{
		result.m_impactNormal = plane.m_fwdNormal;
	}
	else
	{
		result.m_impactNormal = -plane.m_fwdNormal;
	}

	result.m_impactDistance = (rayStartAltitude * rayDistance) / DotProduct3D((rayForwardNormal * rayDistance), -plane.m_fwdNormal);
	result.m_impactPoint = rayStart + rayForwardNormal * result.m_impactDistance;
	return result;
}

