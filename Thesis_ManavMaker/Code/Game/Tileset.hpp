#pragma once
#include "Engine/Math/AABB2.hpp"

class SpriteSheet;

enum TileSetType
{
	TILE_SET_TYPE_NONE = -1,
	TILE_SET_SINGLE_SPRITE = 0,
	TILE_SET_16_SPRITES,
	TILE_SET_48_SPRITES,
	TILE_SET_TYPE_COUNT
};


struct Tileset
{
public:
	TileSetType m_type = TILE_SET_TYPE_NONE;
	SpriteSheet* m_spriteSheet = nullptr;
	AABB2* m_spriteUVs = nullptr;
};
