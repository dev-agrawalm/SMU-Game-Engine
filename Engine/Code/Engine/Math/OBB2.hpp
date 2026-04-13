#pragma once
#include "Engine/Math/Vec2.hpp"

struct OBB2
{
public:
	Vec2 m_center;
	Vec2 m_iBasisNormal;
	Vec2 m_halfDims;

public: //member functions
	OBB2();
	~OBB2();
	OBB2(OBB2 const& copyBox);
	explicit OBB2(Vec2 const& center, Vec2 const& iBasisNormal, Vec2 halfDims);
	explicit OBB2(Vec2 const& center, Vec2 const& iBasisNormal, float width, float height);

	//getters
	void		GetCornersInWorldSpace(Vec2& out_bottomLeft, Vec2& out_bottomRight, Vec2& out_topLeft, Vec2& out_topRight) const;
	bool		IsPointInside(Vec2 const& worldPoint) const;
	Vec2 const	GetCenter() const;
	Vec2 const	GetDimensions() const;
	Vec2 const	GetNearestPointForWorldPos(Vec2 const& worldPointOfReference) const;
	Vec2 const	GetLocalPosForWorldPos(Vec2 const& worldPos) const;
	Vec2 const	GetWorldPosForLocalPos(Vec2 const& localPos) const;

	//mutators
	void RotateAboutCenter(float rotationDeltaDegrees);

	//operator overloads
	void operator= (OBB2 const& box);
};