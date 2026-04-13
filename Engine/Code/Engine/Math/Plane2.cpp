#include "Engine/Math/Plane2.hpp"
#include "Engine/Math/MathUtils.hpp"

Plane2::Plane2(Vec2 const& fwdNormal, float distanceFromOrigin)
	: m_fwdNormal(fwdNormal)
	, m_distanceFromOrigin(distanceFromOrigin)
{

}


Plane2::Plane2(Plane2 const& copyFrom)
	: m_distanceFromOrigin(copyFrom.m_distanceFromOrigin)
	, m_fwdNormal(copyFrom.m_fwdNormal)
{

}


Plane2::Plane2(Vec2 const& fwdNormal, Vec2 const& ptOnPlane)
	: m_fwdNormal(fwdNormal)
{
	m_distanceFromOrigin = DotProduct2D(ptOnPlane, fwdNormal);
}


bool Plane2::GetPointOfIntersection(Plane2 const& refPlane, Vec2& out_pointOfIntersection) const
{
	if (m_fwdNormal == refPlane.m_fwdNormal || m_fwdNormal == -refPlane.m_fwdNormal)
		return false;

	float x1 = m_fwdNormal.x;
	float y1 = m_fwdNormal.y;
	float y1Inverse = 1.0f / y1;
	float d1 = m_distanceFromOrigin;

	float x2 = refPlane.m_fwdNormal.x;
	float y2 = refPlane.m_fwdNormal.y;
	float y2Inverse = 1.0f / y2;
	float d2 = refPlane.m_distanceFromOrigin;
	if (y1 == 0.0f)
	{
		float x = d1 / x1;
		float y = (d2 - x * x2) * y2Inverse;
		out_pointOfIntersection = Vec2(x, y);
		return true;
	}

	if (y2 == 0.0f)
	{
		float x = d2 / x2;
		float y = (d1 - x * x1) * y1Inverse;
		out_pointOfIntersection = Vec2(x, y);
		return true;
	}

	float m1 = -x1 * y1Inverse;
	float c1 = d1 * y1Inverse;
	float m2 = -x2 * y2Inverse;
	float c2 = d2 * y2Inverse;
	float x = (c2 - c1) / (m1 - m2);
	float y = m1 * x + c1;
	out_pointOfIntersection.x = x;
	out_pointOfIntersection.y = y;
	return true;
}


float Plane2::GetShortestDistanceFromPlane(Vec2 const& refPt) const
{
	return DotProduct2D(refPt, m_fwdNormal) - m_distanceFromOrigin;
}


bool Plane2::IsPointBehind(Vec2 const& refPt) const
{
	return DotProduct2D(refPt, m_fwdNormal) < m_distanceFromOrigin;
}


bool Plane2::IsPointInFront(Vec2 const& refPt) const
{
	return DotProduct2D(refPt, m_fwdNormal) > m_distanceFromOrigin;
}


bool Plane2::IsPointOnPlane(Vec2 const& refPt) const
{
	return DotProduct2D(refPt, m_fwdNormal) == m_distanceFromOrigin;
}


Plane2 Plane2::operator-() const
{
	return Plane2(-m_fwdNormal, -m_distanceFromOrigin);
}


bool Plane2::operator==(Plane2 const& compare) const
{
	return (m_fwdNormal == compare.m_fwdNormal) && (m_distanceFromOrigin == compare.m_distanceFromOrigin);
}
