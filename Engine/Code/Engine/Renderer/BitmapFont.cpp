#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/DevConsole.hpp"

const Vec2 BitmapFont::ALIGNED_CENTER			= Vec2(0.5f, 0.5f);
const Vec2 BitmapFont::ALIGNED_CENTER_LEFT		= Vec2(0.0f, 0.5f);
const Vec2 BitmapFont::ALIGNED_CENTER_RIGHT		= Vec2(1.0f, 0.5f);
const Vec2 BitmapFont::ALIGNED_BOTTOM_CENTER	= Vec2(0.5f, 0.0f);
const Vec2 BitmapFont::ALIGNED_BOTTOM_LEFT		= Vec2(0.0f, 0.0f);
const Vec2 BitmapFont::ALIGNED_BOTTOM_RIGHT		= Vec2(1.0f, 0.0f);
const Vec2 BitmapFont::ALIGNED_TOP_CENTER		= Vec2(0.5f, 1.0f);
const Vec2 BitmapFont::ALIGNED_TOP_RIGHT		= Vec2(1.0f, 1.0f);
const Vec2 BitmapFont::ALIGNED_TOP_LEFT			= Vec2(0.0f, 1.0f);

BitmapFont::BitmapFont(char const* fontFilePathNameWithNoExtension, Texture const& fontTexture) 
	: m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension)
	, m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16, 16))
{
	InitialiseUsingFixedAspect();
}


BitmapFont::~BitmapFont()
{
}


Texture const& BitmapFont::GetTexture() const
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}


void BitmapFont::InitialiseUsingFixedAspect()
{
	for (int i = 0; i < 256; i++)
	{
		GlyphStruct& glyph = m_glyphData[i];
		m_fontGlyphsSpriteSheet.GetSpriteUVs(glyph.m_uvs.m_mins, glyph.m_uvs.m_maxs, i);
		glyph.m_aspect = 1.0f;
		glyph.m_glyphChar = (unsigned char) i;
	}
}


void BitmapFont::InitialiseUsingImage(Image const& bitmapFontImage)
{
	int numGlyphsInRow = 16;
	int numGlyphsInCol = 16;
	IntVec2 imageDims = bitmapFontImage.GetDimensions();
	int pixelsPerGlyphQuadX = imageDims.x / numGlyphsInRow;
	int pixelsPerGlyphQuadY = imageDims.y / numGlyphsInCol;

	for (int glyphY = 0; glyphY < numGlyphsInCol; glyphY++)
	{
		for (int glyphX = 0; glyphX < numGlyphsInRow; glyphX++)
		{

			int glyphQuadLeft = glyphX * pixelsPerGlyphQuadX;
			int glyphQuadRight = glyphQuadLeft + pixelsPerGlyphQuadX - 1;
			int glyphQuadTop = imageDims.y - 1 - glyphY * pixelsPerGlyphQuadY;
			int glyphQuadBottom = glyphQuadTop - pixelsPerGlyphQuadY + 1;

			int glyphTexelLeft = -1;
			int glyphTexelRight = -1;
			int glyphTexelTop = -1;
			int glyphTexelBottom = -1;
			for (int texelX = glyphQuadLeft; texelX <= glyphQuadRight; texelX++)
			{
				for (int texelY = glyphQuadTop; texelY >= glyphQuadBottom; texelY--)
				{
					Rgba8 texelRgba = bitmapFontImage.GetTexelRgba8(texelX, texelY);
					if (texelRgba == Rgba8::WHITE)
					{
						if (glyphTexelLeft == -1)
						{
							glyphTexelLeft = texelX;
						}

						glyphTexelRight = texelX;
						if (texelY > glyphTexelTop)
						{
							glyphTexelTop = texelY;
						}
						break;
					}
				}

				for (int texelY = glyphQuadBottom; texelY <= glyphQuadTop; texelY++)
				{
					Rgba8 texelRgba = bitmapFontImage.GetTexelRgba8(texelX, texelY);
					if (texelRgba == Rgba8::WHITE)
					{
						if (texelY < glyphTexelBottom || glyphTexelBottom < 0)
						{
							glyphTexelBottom = texelY;
						}
						break;
					}
				}
			}

			int glyphIndex = glyphY * numGlyphsInRow + glyphX;
			GlyphStruct& glyph = m_glyphData[glyphIndex];
			glyph.m_glyphChar = (unsigned char) glyphIndex;
			
			int glyphTexelWidth = glyphTexelRight - glyphTexelLeft + 1;
			if (glyphTexelWidth > 0)
			{
				glyph.m_preSpace = 1.0f / 32.0f;
				glyph.m_aspect = (float) glyphTexelWidth / (float) pixelsPerGlyphQuadX;
				glyph.m_postSpace = 1.0f / 32.0f;

				float normalizedGlyphLeft		= (float) glyphTexelLeft		/ (float) imageDims.x;
				float normalizedGlyphRight		= (float) (glyphTexelRight + 1)	/ (float) imageDims.x;
				float normalizedGlyphTop		= (float) (glyphQuadTop + 1)	/ (float) imageDims.y;
				float normalizedGlyphBottom		= (float) (glyphQuadBottom + 1)	/ (float) imageDims.y;
				glyph.m_uvs.m_mins = Vec2(normalizedGlyphLeft, normalizedGlyphBottom);
				glyph.m_uvs.m_maxs = Vec2(normalizedGlyphRight, normalizedGlyphTop);

				float correctionFactor = 0.0001f;
				glyph.m_uvs.m_mins += Vec2(correctionFactor, correctionFactor);
				glyph.m_uvs.m_maxs -= Vec2(correctionFactor, correctionFactor);
			}
		}
	}
}


void BitmapFont::InitialiseUsingImage(const char* bitmapFontImagePathWithExtension)
{
	Image fontImage = Image(bitmapFontImagePathWithExtension);
	InitialiseUsingImage(fontImage);
}


void BitmapFont::AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, Vec2 const& textMins, 
								   float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float glyphAspectModifier /*= 1.f*/)
{
	Vec2 charBoundingBoxMins = textMins;
	for (int charIndex = 0; charIndex < (int) text.size(); charIndex++)
	{
		char charAtIndex = text[charIndex];
		GlyphStruct const& glyph = m_glyphData[charAtIndex];
		int charIntValue = (int) charAtIndex;
		float glyphAspect = GetGlyphAspect(charIntValue);
		float preSpace = glyph.m_preSpace * cellHeight;
		float cellWidth = glyphAspect * cellHeight * glyphAspectModifier;
		float postSpace = glyph.m_postSpace * cellHeight;
		charBoundingBoxMins.x += preSpace;
		Vec2 charBoundingBoxMaxs = charBoundingBoxMins + Vec2(cellWidth, cellHeight);
		AABB2 charBoundingBox = AABB2(charBoundingBoxMins, charBoundingBoxMaxs);
		AABB2 glyphUVs = m_glyphData[charIntValue].m_uvs;
		if (charIntValue != 32) //don't add any verts for spacebar because it should be a blank space
		{
			AddVertsForAABB2ToVector(vertexArray, charBoundingBox, tint, glyphUVs.m_mins, glyphUVs.m_maxs);
		}
		charBoundingBoxMins.x += cellWidth + postSpace;
	}
}


void BitmapFont::AddVertsForTextInAABB2(std::vector<Vertex_PCU>& vertexArray, AABB2 const& box,
										float cellHeight, std::string const& text, Rgba8 const& tint,
										float glyphAspectModifier, Vec2 const& alignment, TextDrawMode mode, int maxGlyphsToDraw)
{
	Strings subStrings = SplitStringOnDelimiter(text, '\n');
	float widthOfLongestText = 0.0f;
	for (int stringIndex = 0; stringIndex < (int) subStrings.size(); stringIndex++)
	{
		std::string& subString = subStrings[stringIndex];
		float textLength = GetTextLength(subString, cellHeight, glyphAspectModifier);
		if (textLength > widthOfLongestText)
		{
			widthOfLongestText = textLength;
		}
	}

	float totalHeightOfText = cellHeight * (int) subStrings.size();
	float aabb2Width = box.GetDimensions().x;
	float aabb2Height = box.GetDimensions().y;

	if (mode == TextDrawMode::SHRINK_TO_FIT)
	{
		float horizontalScale = widthOfLongestText > aabb2Width ? (aabb2Width / widthOfLongestText) : 1.0f;
		float verticalScale = totalHeightOfText > aabb2Height ? (aabb2Height / totalHeightOfText) : 1.0f;

		if (verticalScale < horizontalScale)
		{
			widthOfLongestText *= verticalScale;
			totalHeightOfText *= verticalScale;
		}
		else if (horizontalScale < verticalScale)
		{
			widthOfLongestText *= horizontalScale;
			totalHeightOfText *= horizontalScale;
		}
		cellHeight = totalHeightOfText / (float) subStrings.size();
	}

	float excessWidthInAABB = box.GetDimensions().x - widthOfLongestText;
	float excessHeightInAABB = box.GetDimensions().y - totalHeightOfText;
	Vec2 textBoxMins = box.m_mins + Vec2(alignment.x * excessWidthInAABB, alignment.y * excessHeightInAABB);

	int glyphsCount = 0;
	for (int stringIndex = 0; stringIndex < (int) subStrings.size(); stringIndex++)
	{
		std::string& subString = subStrings[stringIndex];
		float widthOfText = GetTextLength(subString, cellHeight, glyphAspectModifier);
		float excessWidthInTextBox = widthOfLongestText - widthOfText;
		float heightOfText = cellHeight * ((int) subStrings.size() - 1 - stringIndex);
		float xAlignment = ClampZeroToOne(alignment.x);
		Vec2 subStringBoxMins = textBoxMins + Vec2(xAlignment * excessWidthInTextBox, heightOfText);
		Vec2 subStringBoxMaxs = subStringBoxMins + Vec2(widthOfText, cellHeight);
		int glphsRemaining = maxGlyphsToDraw - glyphsCount;
		if (glphsRemaining > 0)
		{
			std::string stringToPrint = subString.substr(0, glphsRemaining);
			glyphsCount += (int) stringToPrint.length();
			AddVertsForText2D(vertexArray, subStringBoxMins, cellHeight, stringToPrint, tint, glyphAspectModifier);
		}
	}
}


float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	if (glyphUnicode < 0 || glyphUnicode >= 255)
	{
		return 0.0f;
	}

	return m_glyphData[glyphUnicode].m_aspect;
}


float BitmapFont::GetTextLength(std::string const& text, float cellHeight, float glyphAspectMultiplier)
{
	float length = 0.0f;
	for(int charIndex = 0; charIndex < (int) text.size(); charIndex++)
	{
		char glyph = text[charIndex];
		GlyphStruct glyphData = m_glyphData[glyph];
		length += glyphData.m_preSpace * cellHeight;
		length += glyphData.m_aspect * cellHeight * glyphAspectMultiplier;
		length += glyphData.m_postSpace * cellHeight;
	}
	return length;
}

