#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

SpriteSheet::SpriteSheet(Texture const& texture, IntVec2 const& simpleGridLayout)
	: m_texture(texture)
	, m_cellCount(simpleGridLayout)
{
	int textureWidth = m_texture.m_dimensions.x;
	int textureHeight = m_texture.m_dimensions.y;
	int gridX = simpleGridLayout.x;
	int gridY = simpleGridLayout.y;
	int numOfSprites = simpleGridLayout.x * simpleGridLayout.y;
	m_spriteDefs.reserve( (size_t) numOfSprites);

	float texelWidthFraction = 0.01f;
	float texelHeightFraction = 0.01f;
	for (int spriteDefIndex = 0; spriteDefIndex < numOfSprites; spriteDefIndex++)
	{
		Vec2 uvAtMins;
		uvAtMins.x = ( (float) (spriteDefIndex % gridX) / gridX );
		uvAtMins.y = ( (float) (gridY - 1 - (spriteDefIndex / gridX)) / gridY );
		uvAtMins.x += texelWidthFraction / textureWidth;
		uvAtMins.y += texelHeightFraction / textureHeight;

		Vec2 uvAtMaxs;
		uvAtMaxs.x = uvAtMins.x + (1.0f / simpleGridLayout.x);
		uvAtMaxs.y = uvAtMins.y + (1.0f / simpleGridLayout.y);
		uvAtMaxs.x -= texelWidthFraction / textureWidth;
		uvAtMaxs.y -= texelHeightFraction / textureHeight;

		SpriteDefinition spriteDef(*this, spriteDefIndex, uvAtMins, uvAtMaxs);
		m_spriteDefs.push_back(spriteDef);
	}
}


IntVec2 SpriteSheet::GetGridLayout() const
{
	return m_cellCount;
}


Texture const& SpriteSheet::GetTexture() const
{
	return m_texture;
}


int SpriteSheet::GetNumSprites() const
{
	return (int) m_spriteDefs.size();
}


SpriteDefinition const& SpriteSheet::GetSpriteDefinition(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}


SpriteDefinition const& SpriteSheet::GetSpriteDefinition(IntVec2 const& spriteCoords) const
{
	int spriteIndex = spriteCoords.y * m_cellCount.x + spriteCoords.x;
	return GetSpriteDefinition(spriteIndex);
}


void SpriteSheet::GetSpriteUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex) const
{
	m_spriteDefs[spriteIndex].GetUVs(out_uvAtMins, out_uvAtMaxs);
}


void SpriteSheet::GetSpriteUVs(Vec2& out_uvMins, Vec2& out_uvMaxs, IntVec2 const& spriteCoords) const
{
	int index = spriteCoords.x + spriteCoords.y * m_cellCount.x;
	GetSpriteUVs(out_uvMins, out_uvMaxs, index);
}


AABB2 SpriteSheet::GetSpriteUVs(IntVec2 const& spriteCoords)
{
	int index = spriteCoords.x + spriteCoords.y * m_cellCount.x;
	Vec2 mins;
	Vec2 maxs;
	GetSpriteUVs(mins, maxs, index);
	return AABB2(mins, maxs);
}


int SpriteSheet::GetSpriteIndex(IntVec2 const& spriteCoords)
{
	return spriteCoords.x + spriteCoords.y * m_cellCount.x;
}
