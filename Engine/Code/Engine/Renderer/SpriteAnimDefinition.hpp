#pragma once
#include <vector>
#include <string>

class SpriteSheet;
class SpriteDefinition;

enum class SpriteAnimPlaybackMode
{
	INVALID,
	LOOP,
	PING_PONG,
	ONCE
};


class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition(SpriteSheet const& spriteSheet, int startingSpriteIndex, int endingSpriteIndex, float duration, 
						 SpriteAnimPlaybackMode playbackMode = SpriteAnimPlaybackMode::ONCE);
	SpriteAnimDefinition(SpriteSheet const& spriteSheet, std::vector<int> spriteIndices, float duration,
						 SpriteAnimPlaybackMode playbackMode = SpriteAnimPlaybackMode::ONCE);
	~SpriteAnimDefinition();

	void SetAnimPlaybackMode(SpriteAnimPlaybackMode playbackMode);

	SpriteDefinition const& GetSpriteDefAtTime(float time) const;

private:
	SpriteSheet const& m_spriteSheet;
	float m_animDuration = 1.0f;
	SpriteAnimPlaybackMode m_animationPlaybackMode = SpriteAnimPlaybackMode::INVALID;
	std::vector<int> m_animSpriteIndices;
};
