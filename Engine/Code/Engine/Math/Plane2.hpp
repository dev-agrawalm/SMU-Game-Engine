#pragma once
#include "Engine/Math/Vec2.hpp"

struct Plane2
{
public:
	Plane2() {}
	Plane2(Vec2 const& fwdNormal, float distanceFromOrigin);
	Plane2(Vec2 const& fwdNormal, Vec2 const& ptOnPlane);
	Plane2(Plane2 const& copyFrom);

	bool GetPointOfIntersection(Plane2 const& refPlane, Vec2& out_pointOfIntersection) const;
	float GetShortestDistanceFromPlane(Vec2 const& refPt) const;
	bool IsPointBehind(Vec2 const& refPt) const;
	bool IsPointInFront(Vec2 const& refPt) const;
	bool IsPointOnPlane(Vec2 const& refPt) const;

	bool			operator==(Plane2 const& compare) const;
	Plane2			operator-() const;
public:
	Vec2 m_fwdNormal;
	float m_distanceFromOrigin = 0.0f;
};
