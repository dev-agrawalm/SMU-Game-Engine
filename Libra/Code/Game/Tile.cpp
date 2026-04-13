#include "Game/Tile.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Engine/Core/EngineCommon.hpp"

Tile::Tile()
{
}


Tile::~Tile()
{
}


AABB2 Tile::GetBoundingBox() const
{
	AABB2 tileAABB2;
	float tileWidth		= g_gameConfigBlackboard.GetValue("tileWidth", 1.0f);
	tileAABB2.m_mins.x	= m_gridCoords.x * tileWidth;
	tileAABB2.m_mins.y	= m_gridCoords.y * tileWidth;
	tileAABB2.m_maxs.x	= tileAABB2.m_mins.x + tileWidth;
	tileAABB2.m_maxs.y	= tileAABB2.m_mins.y + tileWidth;
	return tileAABB2;
}


Vec2 Tile::GetCenter() const
{
	return GetBoundingBox().GetCenter();
}
