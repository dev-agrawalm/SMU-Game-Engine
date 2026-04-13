#include "Engine/Math/RandomNumberGenerator.hpp"
#include <stdlib.h>
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

int RandomNumberGenerator::GetRandomIntLessThan(int upperLimit_NonInclusive) const
{
	int randInt = rand();
	randInt %= upperLimit_NonInclusive;
	return randInt;
}

int RandomNumberGenerator::GetRandomIntInRange(int lowerLimit_Inclusive, int upperLimit_Inclusive) const
{
	int randInt = rand();
	int range = upperLimit_Inclusive - lowerLimit_Inclusive + 1; //we add one because lower limit and upper limit are both inclusive but subtraction does not include the lower limit

	return (lowerLimit_Inclusive) + (randInt % range);
}

float RandomNumberGenerator::GetRandomFloatZeroToOne() const
{
	float randFloat = static_cast<float>(rand());
	return randFloat * (1.0f / RAND_MAX) ;
}

float RandomNumberGenerator::GetRandomFloatInRange(float lowerLimit_Inclusive, float upperLimit_Inclusive) const
{
	float range = upperLimit_Inclusive - lowerLimit_Inclusive;
	float randFloat = GetRandomFloatZeroToOne();
	float randRange = randFloat * range;

	return lowerLimit_Inclusive + randRange;
}

bool RandomNumberGenerator::GetRandomChance(float probabilityFraction)
{
	probabilityFraction = ClampZeroToOne(probabilityFraction);
	float randomValue = GetRandomFloatZeroToOne();
	if (randomValue <= probabilityFraction)
	{
		return true;
	}

	return false;
}


Rgba8 RandomNumberGenerator::GetRandomColor()
{
	float r = GetRandomFloatZeroToOne();
	float g = GetRandomFloatZeroToOne();
	float b = GetRandomFloatZeroToOne();

	Rgba8 randColor;
	randColor.r = FloatToByte(r);
	randColor.g = FloatToByte(g);
	randColor.b = FloatToByte(b);
	randColor.a = 255;

	return randColor;
}


Vec2 RandomNumberGenerator::GetRandomPositionInAABB2(Vec2 const&aabb2Mins, Vec2 const& aabb2Maxs)
{
	float x = GetRandomFloatInRange(aabb2Mins.x, aabb2Maxs.x);
	float y = GetRandomFloatInRange(aabb2Mins.y, aabb2Maxs.y);

	return Vec2(x, y);
}


Vec2 RandomNumberGenerator::GetRandomPositionInAABB2(float minX, float minY, float maxX, float maxY)
{
	float x = GetRandomFloatInRange(minX, maxX);
	float y = GetRandomFloatInRange(minY, maxY);

	return Vec2(x, y);
}


Vec3 RandomNumberGenerator::GetRandomPositionInAABB3(Vec3 const& cubeMin, Vec3 const& cubeMax)
{
	float x = GetRandomFloatInRange(cubeMin.x, cubeMax.x);
	float y = GetRandomFloatInRange(cubeMin.y, cubeMax.y);
	float z = GetRandomFloatInRange(cubeMin.z, cubeMax.z);

	return Vec3(x, y, z);
}
