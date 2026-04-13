#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>

class	Map;
struct	AABB2;
struct	TileDefinition;

struct Tile
{
public:
	IntVec2					m_gridCoords;
	TileDefinition const*   m_tileDefinition = nullptr;
	
public:
	Tile();
	~Tile();

	//getters
	AABB2	GetBoundingBox() const;
	Vec2	GetCenter() const;
};
