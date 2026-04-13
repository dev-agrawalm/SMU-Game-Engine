#pragma once

struct Rgba8;
struct Vec2;
struct Vec3;

class RandomNumberGenerator
{
public:
	int	  GetRandomIntLessThan(int upperLimit_NonInclusive) const;
	int	  GetRandomIntInRange(int lowerLimit_Inclusive, int upperLimit_Inclusive) const;
	float GetRandomFloatZeroToOne() const;
	float GetRandomFloatInRange(float lowerLimit_Inclusive, float upperLimit_Inclusive) const;
	bool  GetRandomChance(float probabilityFraction); //probability fraction should be between 0 and 1
	Rgba8 GetRandomColor();
	Vec2 GetRandomPositionInAABB2(Vec2 const&aabb2Mins, Vec2 const& aabb2Maxs);
	Vec2 GetRandomPositionInAABB2(float minX, float minY, float maxX, float maxY);
	Vec3 GetRandomPositionInAABB3(Vec3 const& cubeMin, Vec3 const& cubeMax);
private:
	//unsigned int m_seed = 0;
	//int m_position = 0;
};