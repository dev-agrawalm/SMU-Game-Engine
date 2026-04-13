#include "Game/Shapes2D.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"


void Disk2D::AddVertsForShapeToVector(std::vector<Vertex_PCU>& verts, Rgba8 const& color)
{
	AddVertsForDisk2DToVector(verts, m_center, m_radius, color, 30);
}


Vec2 Disk2D::GetNearestPoint(Vec2 const& referencePoint)
{
	return GetNearestPointOnDisk2D(referencePoint, m_center, m_radius);
}


Vec2 Disk2D::GetCenter() const
{
	return m_center;
}


bool  Disk2D::PushDiskOutOfShape(Vec2& diskCenter, float diskRadius)
{
	return PushDiskOutOfDisk2D(diskCenter, diskRadius, m_center, m_radius);
}


void OBB2D::AddVertsForShapeToVector(std::vector<Vertex_PCU>& verts, Rgba8 const& color)
{
	AddVertsForOBB2DToVector(verts, m_obb, color);
}


Vec2 OBB2D::GetNearestPoint(Vec2 const& referencePoint)
{
	return GetNearestPointOnOBB2D(referencePoint, m_obb);
}


Vec2 OBB2D::GetCenter() const
{
	return m_obb.m_center;
}


bool OBB2D::PushDiskOutOfShape(Vec2& diskCenter, float diskRadius)
{
	Vec2 nearestPointOnObb = GetNearestPointOnOBB2D(diskCenter, m_obb);
	return PushDiskOutOfDisk2D(diskCenter, diskRadius, nearestPointOnObb, 0.0f);
}


void Capsule2D::AddVertsForShapeToVector(std::vector<Vertex_PCU>& verts, Rgba8 const& color)
{
	AddVertsForCapsule2DToVector(verts, m_capsule, color);
}


Vec2 Capsule2D::GetNearestPoint(Vec2 const& referencePoint)
{
	return GetNearestPointOnCapsule2D(referencePoint, m_capsule);
}


Vec2 Capsule2D::GetCenter() const
{
	return m_capsule.GetCenter();
}


bool Capsule2D::PushDiskOutOfShape(Vec2& diskCenter, float diskRadius)
{
	LineSegment2 bone = m_capsule.m_bone;
	Vec2 nearestPointOnBone = GetNearentPointOnLineSegment2D(diskCenter, bone);
	return PushDiskOutOfDisk2D(diskCenter, diskRadius + m_capsule.m_radius, nearestPointOnBone, 0.0f);
}

