#include "Engine/Math/Capsule2.hpp"

Capsule2::Capsule2()
{

}


Vec2 Capsule2::GetCenter() const
{
	return m_bone.GetCenter();
}


Capsule2::~Capsule2()
{

}


Capsule2::Capsule2(Capsule2 const& copyFrom) :
	m_bone(copyFrom.m_bone),
	m_radius(copyFrom.m_radius)
{

}


Capsule2::Capsule2(LineSegment2 const& bone, float radius) :
	m_bone(bone),
	m_radius(radius)
{

}


Capsule2::Capsule2(Vec2 const& boneStart, Vec2 const& boneEnd, float radius):
	m_bone(LineSegment2(boneStart, boneEnd)),
	m_radius(radius)
{

}
