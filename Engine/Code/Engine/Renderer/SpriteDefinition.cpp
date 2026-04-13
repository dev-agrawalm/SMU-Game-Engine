#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/Vec2.hpp"

SpriteDefinition::SpriteDefinition(SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs)
	: m_spriteSheet(spriteSheet)
	, m_uvAtMins(uvAtMins)
	, m_uvAtMaxs(uvAtMaxs)
	, m_spriteIndex(spriteIndex)
{

}


void SpriteDefinition::GetUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const
{
	out_uvAtMaxs = m_uvAtMaxs;
	out_uvAtMins = m_uvAtMins;
}


SpriteSheet const& SpriteDefinition::GetSpriteSheet() const
{
	return m_spriteSheet;
}


Texture const& SpriteDefinition::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}


float SpriteDefinition::GetAspect() const
{
	float width = m_uvAtMaxs.x - m_uvAtMins.x;
	float height = m_uvAtMaxs.y - m_uvAtMins.y;
	return (width / height);
}


int SpriteDefinition::GetSpriteSheetIndex() const
{
	return m_spriteIndex;
}
