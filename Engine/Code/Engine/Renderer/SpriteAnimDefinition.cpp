#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& spriteSheet, int startingSpriteIndex,
										   int endingSpriteIndex, float duration, SpriteAnimPlaybackMode playbackMode /*= SpriteAnimPlaybackMode::ONCE*/)
	: m_spriteSheet(spriteSheet)
	, m_animDuration(duration)
	, m_animationPlaybackMode(playbackMode)
{
	for (int spriteIndex = startingSpriteIndex; spriteIndex <= endingSpriteIndex; spriteIndex++)
	{
		m_animSpriteIndices.push_back(spriteIndex);
	}
}


SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& spriteSheet, std::vector<int> spriteIndices, float duration, SpriteAnimPlaybackMode playbackMode /*= SpriteAnimPlaybackMode::ONCE*/)
	:m_spriteSheet(spriteSheet)
	, m_animSpriteIndices(spriteIndices)
	, m_animDuration(duration)
	, m_animationPlaybackMode(playbackMode)
{

}


SpriteAnimDefinition::~SpriteAnimDefinition()
{

}


void SpriteAnimDefinition::SetAnimPlaybackMode(SpriteAnimPlaybackMode playbackMode)
{
	m_animationPlaybackMode = playbackMode;
}


SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime(float time) const
{
	int numFrames = (int) m_animSpriteIndices.size(); // m_endingSpriteIndex - m_startingSpriteIndex + 1;
	float timePerFrame = m_animDuration / numFrames;

	switch (m_animationPlaybackMode)
	{
		case SpriteAnimPlaybackMode::LOOP:
		{
			while (time >= m_animDuration)
				time -= m_animDuration;

			int frameCount = 0;
			while (time > timePerFrame)
			{
				time -= timePerFrame;
				frameCount++;
			}

			int spriteIndex = frameCount;
			spriteIndex %= numFrames;
			return m_spriteSheet.GetSpriteDefinition(m_animSpriteIndices[spriteIndex]);
		}
		case SpriteAnimPlaybackMode::PING_PONG:
		{
			float fakeDuration = m_animDuration - timePerFrame;
			float timeCopy = time;
			int multipleOfDuration = 0;
			while (timeCopy > 0)
			{
				timeCopy -= fakeDuration;
				multipleOfDuration++;
			}

			while (time >= fakeDuration)
				time -= fakeDuration;

			int frameCount = 0;
			while (time > timePerFrame)
			{
				time -= timePerFrame;
				frameCount++;
			}

			int spriteIndex;
			if (multipleOfDuration % 2 == 0)
			{
				spriteIndex = ((int) m_animSpriteIndices.size() - 1) - frameCount;
			}
			else
			{
				spriteIndex = frameCount;
			}
			return m_spriteSheet.GetSpriteDefinition(m_animSpriteIndices[spriteIndex]);
		}
		case SpriteAnimPlaybackMode::ONCE:
		{
			if (time >= m_animDuration)
				return m_spriteSheet.GetSpriteDefinition(m_animSpriteIndices.back());

			int frameCount = 0;
			while (time > timePerFrame)
			{
				time -= timePerFrame;
				frameCount++;
			}
			
			int spriteIndex = frameCount;
			return m_spriteSheet.GetSpriteDefinition(m_animSpriteIndices[spriteIndex]);
		}
		case SpriteAnimPlaybackMode::INVALID: //fallthrough
		default:
			ERROR_RECOVERABLE("Invalid animation playback mode provided. Returning the first frame of the animation");
			return m_spriteSheet.GetSpriteDefinition(m_animSpriteIndices[0]);
	}
}
