#pragma once
#include <vector>
#include <string>
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"

enum class TextDrawMode
{
	OVERRUN,
	SHRINK_TO_FIT
};


struct GlyphStruct
{
public:
	float m_preSpace = 0.0f;
	float m_aspect = 1.0f;
	float m_postSpace = 0.0f;
	AABB2 m_uvs;
	unsigned char m_glyphChar = 0;
};


class Image;

class BitmapFont
{
	friend class Renderer; // Only the RenderContext can create new BitmapFont objects!

private:
	BitmapFont(char const* fontFilePathNameWithNoExtension, Texture const& fontTexture);

public:
	static const Vec2 ALIGNED_CENTER;
	static const Vec2 ALIGNED_CENTER_LEFT;
	static const Vec2 ALIGNED_CENTER_RIGHT;
	static const Vec2 ALIGNED_BOTTOM_CENTER;
	static const Vec2 ALIGNED_BOTTOM_LEFT;
	static const Vec2 ALIGNED_BOTTOM_RIGHT;
	static const Vec2 ALIGNED_TOP_CENTER;
	static const Vec2 ALIGNED_TOP_RIGHT;
	static const Vec2 ALIGNED_TOP_LEFT;

public:
	~BitmapFont();
	Texture const& GetTexture() const;

	void InitialiseUsingFixedAspect();
	void InitialiseUsingImage(Image const& bitmapFontImage);
	void InitialiseUsingImage(const char* bitmapFontImagePath);

	void AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins,
						   float cellHeight, std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float glyphAspectModifier = 1.f);
	
	void AddVertsForTextInAABB2(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box, float cellHeight,
								std::string const& text, Rgba8 const& tint = Rgba8::WHITE, float glyphAspectModifier = 1.0f,
								Vec2 const& alignment = ALIGNED_CENTER, TextDrawMode mode = TextDrawMode::SHRINK_TO_FIT, int maxGlyphsToDraw = 99999999);

protected:
	float GetGlyphAspect(int glyphUnicode) const; // For now this will always return 1.0f!!!
	float GetTextLength(std::string const& text, float cellHeight, float glyphAspectMultiplier);

protected:
	std::string		m_fontFilePathNameWithNoExtension	= "";
	SpriteSheet		m_fontGlyphsSpriteSheet;
	GlyphStruct m_glyphData[256] = {};
};