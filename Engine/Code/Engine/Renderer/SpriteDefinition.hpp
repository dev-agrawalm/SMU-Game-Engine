#pragma once
#include "Engine/Math/Vec2.hpp"

class	SpriteSheet;
class	Texture;

class SpriteDefinition
{
public:
	explicit SpriteDefinition(SpriteSheet const& spriteSheet, int spriteIndex, Vec2 const& uvAtMins, Vec2 const& uvAtMaxs);

	void				GetUVs(Vec2& out_uvAtMins, Vec2& out_uvAtMaxs) const;
	SpriteSheet const&	GetSpriteSheet() const;
	Texture const&		GetTexture() const;
	float				GetAspect() const;
	int					GetSpriteSheetIndex() const;

protected:
	int m_spriteIndex	= -1;
	Vec2 m_uvAtMins		= Vec2::ZERO;
	Vec2 m_uvAtMaxs		= Vec2::ONE;
	SpriteSheet const& m_spriteSheet;
};