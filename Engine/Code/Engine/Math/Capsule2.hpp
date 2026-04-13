#pragma once
#include "Engine/Math/LineSegment2.hpp"

struct Capsule2
{
public:
	float m_radius = 0.0f;
	LineSegment2 m_bone;
public:
	Capsule2();
	~Capsule2();
	Capsule2(Capsule2 const& copyFrom);
	explicit Capsule2(LineSegment2 const& bone, float radius);
	explicit Capsule2(Vec2 const& boneStart, Vec2 const& boneEnd, float radius);

	Vec2 GetCenter() const;
};