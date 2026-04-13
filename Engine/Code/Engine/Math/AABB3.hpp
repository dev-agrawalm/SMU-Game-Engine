#pragma once
#include "Engine/Math/Vec3.hpp"

struct AABB3
{
public:
	Vec3 m_mins;
	Vec3 m_maxs;

public:
	AABB3();
	~AABB3();
	AABB3(Vec3 const& mins, Vec3 const& maxs);
	AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	AABB3(Vec3 const& center, float xDims, float yDims, float zDims);
	AABB3(AABB3 const& copyFrom);
	AABB3(Vec3 const& center, float sideDims);

public:
	static const AABB3 ZERO_TO_ONE;

	Vec3 GetCenter() const;
	Vec3 GetDimensions() const;
	Vec3 GetHalfDimensions() const;
};