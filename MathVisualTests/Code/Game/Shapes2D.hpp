#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include<vector>
#include "Engine/Core/Vertex_PCU.hpp"

enum Shape2DName
{
	SHAPE_CAPSULE_2D,
	SHAPE_OBB_2D,
	SHAPE_DISK_2D,
};


struct Shape2D
{
public:
	virtual ~Shape2D() {}
	virtual void AddVertsForShapeToVector(std::vector<Vertex_PCU>& verts, Rgba8 const& color) = 0;
	virtual Vec2 GetNearestPoint(Vec2 const& referencePoint) = 0;
	virtual Vec2 GetCenter() const = 0;
	virtual bool PushDiskOutOfShape(Vec2& diskCenter, float diskRadius) = 0;

public:
	Shape2DName m_name;
};


struct Disk2D : public Shape2D
{
public:
	Disk2D() { m_name = SHAPE_DISK_2D; }
	Disk2D(float radius, Vec2 const& center) : m_radius(radius), m_center(center) { m_name = SHAPE_DISK_2D; }
	virtual void AddVertsForShapeToVector(std::vector<Vertex_PCU>& verts, Rgba8 const& color) override;
	virtual Vec2 GetNearestPoint(Vec2 const& referencePoint) override;
	virtual Vec2 GetCenter() const override;
	virtual bool PushDiskOutOfShape(Vec2& diskCenter, float diskRadius) override;

public:
	float m_radius = 0.0f;
	Vec2 m_center;
};


struct OBB2D : public Shape2D
{
public:
	OBB2D() { m_name = SHAPE_OBB_2D; }
	OBB2D(OBB2 const& obb) : m_obb(obb) { m_name = SHAPE_OBB_2D; }
	virtual void AddVertsForShapeToVector(std::vector<Vertex_PCU>& verts, Rgba8 const& color) override;
	virtual Vec2 GetNearestPoint(Vec2 const& referencePoint) override;
	virtual Vec2 GetCenter() const override;
	virtual bool PushDiskOutOfShape(Vec2& diskCenter, float diskRadius) override;
public:
	OBB2 m_obb;
};


struct Capsule2D : public Shape2D
{
public:
	Capsule2D() { m_name = SHAPE_CAPSULE_2D; }
	Capsule2D(Capsule2 const& capsule) : m_capsule(capsule) { m_name = SHAPE_CAPSULE_2D; }
	virtual void AddVertsForShapeToVector(std::vector<Vertex_PCU>& verts, Rgba8 const& color) override;
	virtual Vec2 GetNearestPoint(Vec2 const& referencePoint) override;
	virtual Vec2 GetCenter() const override;
	virtual bool PushDiskOutOfShape(Vec2& diskCenter, float diskRadius) override;
public:
	Capsule2 m_capsule;
};


