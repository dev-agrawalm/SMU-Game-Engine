#include "Engine/Math/AABB3.hpp"

AABB3 const AABB3::ZERO_TO_ONE = AABB3(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));

Vec3 AABB3::GetCenter() const
{
	Vec3 halfDims = (m_maxs - m_mins) * 0.5f;
	return m_mins + halfDims;
}


Vec3 AABB3::GetDimensions() const
{
	return Vec3(m_maxs.x - m_mins.x, m_maxs.y - m_mins.y, m_maxs.z - m_mins.z);
}


Vec3 AABB3::GetHalfDimensions() const
{
	return GetDimensions() * 0.5f;
}


AABB3::AABB3()
{

}


AABB3::AABB3(Vec3 const& mins, Vec3 const& maxs) :
	m_mins(mins),
	m_maxs(maxs)
{

}


AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) :
	m_mins(minX, minY, minZ),
	m_maxs(maxX, maxY, maxZ)
{

}


AABB3::AABB3(AABB3 const& copyFrom) :
	m_mins(copyFrom.m_mins),
	m_maxs(copyFrom.m_maxs)
{

}


AABB3::AABB3(Vec3 const& center, float xDims, float yDims, float zDims)
{
	float halfDimsX = xDims * 0.5f;
	float halfDimsY = yDims * 0.5f;
	float halfDimsZ = zDims * 0.5f;

	m_mins = center - Vec3(halfDimsX, halfDimsY, halfDimsZ);
	m_maxs = center + Vec3(halfDimsX, halfDimsY, halfDimsZ);
}


AABB3::AABB3(Vec3 const& center, float sideDims)
{
	float halfDims = sideDims * 0.5f;

	m_mins = center - Vec3(halfDims, halfDims, halfDims);
	m_maxs = center + Vec3(halfDims, halfDims, halfDims);
}


AABB3::~AABB3()
{

}
