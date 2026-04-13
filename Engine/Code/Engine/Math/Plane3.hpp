#pragma once
#include "Engine/Math/Vec3.hpp"


struct Plane3
{
public:
	float GetShortestDistanceFromPlane(Vec3 const& refPt) const;

public:
	Vec3 m_fwdNormal;
	float m_distanceFromOrigin = 0.0f;
};
