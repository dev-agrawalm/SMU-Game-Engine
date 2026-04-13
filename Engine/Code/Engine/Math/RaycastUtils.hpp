#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

struct AABB2;
struct LineSegment2;
struct AABB3;
struct FloatRange;
struct Plane2;
struct Plane3;
struct ConvexHull2;

struct BaseRaycastResult2D
{
public:
	Vec2 m_startPos;
	Vec2 m_forwardNormal;
	float m_maxDistance = 0.0f;

	bool m_didHit = false;
	Vec2 m_impactPoint;
	Vec2 m_impactNormal;
	float m_impactDistance = 0.0f;
	float m_impactFraction = 0.0f;
};


struct BaseRaycastResult3D
{
public:
	Vec3 m_startPos;
	Vec3 m_forwardNormal;
	float m_maxDistance = 0.0f;

	bool m_didHit = false;
	Vec3 m_impactPoint;
	Vec3 m_impactNormal;
	float m_impactDistance = 0.0f;
	float m_impactFraction = 0.0f;
};


BaseRaycastResult2D	RaycastVsDisk2D(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, Vec2 const& diskCenter, float diskRadius);
BaseRaycastResult2D	RaycastVsAABB2(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, AABB2 const& aabb2);
BaseRaycastResult2D	RaycastVsLineSegment2(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, LineSegment2 const& line);
BaseRaycastResult2D	RaycastVsPlane2(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, Plane2 const& plane);
BaseRaycastResult2D	RaycastVsConvexHull2(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayDistance, ConvexHull2 const& convexHull);

BaseRaycastResult3D RaycastVsZSphere3D(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance, Vec3 const& sphereCenter, float sphereRadius);
BaseRaycastResult3D RaycastVsAABB3(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance, AABB3 const& aabb3);
BaseRaycastResult3D RaycastVsZCylinder3D(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance, Vec2 const& cylinderCenter, FloatRange const& minMaxZ, float cylinderRadius);
BaseRaycastResult3D	RaycastVsPlane3(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance, Plane3 const& plane);
