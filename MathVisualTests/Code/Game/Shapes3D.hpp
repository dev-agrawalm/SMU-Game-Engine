#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Rgba8.hpp"

struct BaseRaycastResult3D;

enum Shape3DName
{
	SHAPE_AABB3,
	SHAPE_ZCYLINDER,
	SHAPE_ZSPHERE
};


struct Shape3D
{
public:
	virtual Vec3 GetNearestPoint(Vec3 const& referencePoint) = 0;
	virtual BaseRaycastResult3D Raycast(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance) = 0;
	virtual void Translate(Vec3 const& translation) = 0;
	virtual ~Shape3D() {}

public:
	int m_vertCount = 0;
	int m_vertsStartIndex = 0;
	Shape3DName m_shape;
	Rgba8 m_color;
	bool m_isWire = false;
	bool m_isOverlapping = false;
};


struct AABB3D : public Shape3D
{
public:
	AABB3D(AABB3 const& aabb3) : m_aabb3(aabb3) {}
	
	virtual Vec3 GetNearestPoint(Vec3 const& referencePoint) override;
	virtual BaseRaycastResult3D Raycast(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance) override;
	virtual void Translate(Vec3 const& translation) override;

public:
	AABB3 m_aabb3;
};


struct ZCylinder3D : public Shape3D
{
public:
	ZCylinder3D(float radius, Vec2 const& center, FloatRange const& minMax) : m_center(center), m_radius(radius), m_minMax(minMax) {}
	
	virtual Vec3 GetNearestPoint(Vec3 const& referencePoint) override;
	virtual BaseRaycastResult3D Raycast(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance) override;
	virtual void Translate(Vec3 const& translation) override;

public:
	float m_radius = 0.0f;
	Vec2 m_center;
	FloatRange m_minMax;
};


struct ZSphere3D : public Shape3D
{
public:
	ZSphere3D(float radius, Vec3 const& center) : m_center(center), m_radius(radius) {}

	virtual Vec3 GetNearestPoint(Vec3 const& referencePoint) override;
	virtual BaseRaycastResult3D Raycast(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayDistance) override;
	virtual void Translate(Vec3 const& translation) override;

public:
	float m_radius = 0.0f;
	Vec3 m_center;
};
