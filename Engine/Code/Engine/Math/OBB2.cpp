#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"

OBB2::OBB2()
{
}


OBB2::~OBB2()
{

}


OBB2::OBB2(OBB2 const& copyBox)
	: m_center(copyBox.m_center)
	, m_iBasisNormal(copyBox.m_iBasisNormal)
	, m_halfDims(copyBox.m_halfDims)
{
}


OBB2::OBB2(Vec2 const& center, Vec2 const& iBasisNormal, Vec2 halfDims)
	: m_center(center)
	, m_iBasisNormal(iBasisNormal)
	, m_halfDims(halfDims)
{
}


OBB2::OBB2(Vec2 const& center, Vec2 const& iBasisNormal, float width, float height)
	: m_center(center)
	, m_iBasisNormal(iBasisNormal)
	, m_halfDims(Vec2(width * 0.5f, height * 0.5f))
{
}


bool OBB2::IsPointInside(Vec2 const& worldPoint) const
{
	Vec2 localPos = GetLocalPosForWorldPos(worldPoint);

	return localPos.x > -m_halfDims.x && localPos.x < m_halfDims.x
			&& localPos.y > -m_halfDims.y && localPos.y < m_halfDims.y;
}


void OBB2::GetCornersInWorldSpace(Vec2& out_bottomLeft, Vec2& out_bottomRight, Vec2& out_topLeft, Vec2& out_topRight) const
{
	out_bottomLeft	= GetWorldPosForLocalPos(Vec2(	-m_halfDims.x,	-m_halfDims.y));
	out_bottomRight = GetWorldPosForLocalPos(Vec2(	m_halfDims.x,	-m_halfDims.y));
	out_topLeft		= GetWorldPosForLocalPos(Vec2(	-m_halfDims.x,	m_halfDims.y));
	out_topRight	= GetWorldPosForLocalPos(Vec2(	m_halfDims.x,	m_halfDims.y));
}


Vec2 const OBB2::GetCenter() const
{
	return m_center;
}


Vec2 const OBB2::GetDimensions() const
{
	return m_halfDims * 2.0f;
}


Vec2 const OBB2::GetNearestPointForWorldPos(Vec2 const& worldPointOfReference) const
{
	if (IsPointInside(worldPointOfReference))
	{
		return worldPointOfReference;
	}

	Vec2 refPointInLocalSpace = GetLocalPosForWorldPos(worldPointOfReference);
	float nearestPointInLocalSpaceX = Clamp(refPointInLocalSpace.x, -m_halfDims.x, m_halfDims.x);
	float nearestPointInLocalSpaceY = Clamp(refPointInLocalSpace.y, -m_halfDims.y, m_halfDims.y);

	return GetWorldPosForLocalPos(Vec2(nearestPointInLocalSpaceX, nearestPointInLocalSpaceY));
}


Vec2 const OBB2::GetLocalPosForWorldPos(Vec2 const& worldPos) const
{
	Vec2 centerToPos		= worldPos - m_center;
	Vec2 iBasis				= m_iBasisNormal;
	Vec2 jBasis				= m_iBasisNormal.GetRotated90Degrees();
	float localSpacePosX	= DotProduct2D(centerToPos, iBasis);
	float localSpacePosY	= DotProduct2D(centerToPos, jBasis);
	return Vec2(localSpacePosX, localSpacePosY);
}


Vec2 const OBB2::GetWorldPosForLocalPos(Vec2 const& localPos) const
{
	Vec2 posIComponent = localPos.x * m_iBasisNormal;
	Vec2 posJComponent = localPos.y * m_iBasisNormal.GetRotated90Degrees();
	return posJComponent + posIComponent + m_center;
}


void OBB2::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_iBasisNormal.RotateDegrees(rotationDeltaDegrees);
}


void OBB2::operator=(OBB2 const& box)
{
	m_iBasisNormal	= box.m_iBasisNormal;
	m_halfDims		= box.m_halfDims;
	m_center		= box.m_center;
}
