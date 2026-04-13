#pragma once
#include "Engine/Renderer/SpriteDefinition.hpp"
#include<vector>
#include "Engine/Math/IntVec2.hpp"

struct AABB2;

class Texture;

class SpriteSheet
{
public:
	explicit SpriteSheet(Texture const& texture, IntVec2 const& simpleGridLayout);

	IntVec2					GetGridLayout() const;
	Texture const&			GetTexture() const;
	int						GetNumSprites() const;
	SpriteDefinition const& GetSpriteDefinition(int spriteIndex) const;
	SpriteDefinition const& GetSpriteDefinition(IntVec2 const& spriteCoords) const;
	void					GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_AtMaxs, int spriteIndex) const;
	//sprite coords begin from top left and end at bottom right
	void					GetSpriteUVs(Vec2& out_uvMins, Vec2& out_uvMaxs, IntVec2 const& spriteCoords) const;
	AABB2					GetSpriteUVs(IntVec2 const& spriteCoords);
	int						GetSpriteIndex(IntVec2 const& spriteCoords);

protected:
	Texture const&					m_texture;
	std::vector<SpriteDefinition>	m_spriteDefs;
	IntVec2 m_cellCount;
};
