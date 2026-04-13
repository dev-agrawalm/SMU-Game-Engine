#include "Game/Shapes.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RaycastUtils.hpp"

Vec3 AABB3D::GetNearestPoint(Vec3 const& referencePoint)
{
	return GetNearestPointOnAABB3(referencePoint, m_aabb3);
}


BaseRaycastResult3D AABB3D::Raycast(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance)
{
	return RaycastVsAABB3(rayStart, rayForwardNormal, rayDistance, m_aabb3);
}


void AABB3D::Translate(Vec3 const& translation)
{
	m_aabb3.m_mins += translation;
	m_aabb3.m_maxs += translation;
}


Vec3 ZCylinder3D::GetNearestPoint(Vec3 const& referencePoint)
{
	return GetNearestPointOnZCylinder3D(referencePoint, m_center, m_minMax, m_radius);
}


BaseRaycastResult3D ZCylinder3D::Raycast(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance)
{
	return RaycastVsZCylinder3D(rayStart, rayForwardNormal, rayDistance, m_center, m_minMax, m_radius);
}


void ZCylinder3D::Translate(Vec3 const& translation)
{
	Vec2 translationXY = translation.GetVec2();
	m_center += translationXY;
	m_minMax.m_min += translation.z;
	m_minMax.m_max += translation.z;
}


Vec3 ZSphere3D::GetNearestPoint(Vec3 const& referencePoint)
{
	return GetNearestPointOnZSphere3D(referencePoint, m_center, m_radius);
}


BaseRaycastResult3D ZSphere3D::Raycast(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance)
{
	return RaycastVsZSphere3D(rayStart, rayForwardNormal, rayDistance, m_center, m_radius);
}


void ZSphere3D::Translate(Vec3 const& translation)
{
	m_center += translation;
}
