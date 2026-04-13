#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/OBB2.hpp"
#include <cmath>
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/AABB3.hpp"
#include "ThirdParty/SquirrelNoise/SmoothNoise.hpp"
#include "Engine/Math/ConvexHull2.hpp"
#include "Engine/Math/Plane2.hpp"

int GetMin(int a, int b)
{
	if (a < b)
		return a;
	return b;
}


float GetMin(float a, float b)
{
	if (a < b)
		return a;
	return b;
}


uint32_t GetMin(uint32_t a, uint32_t b)
{
	if (a < b)
		return a;
	return b;
}


float GetAbsoluteMin(float a, float b)
{
	float absA = abs(a);
	float absB = abs(b);
	if (absA < absB)
		return a;
	return b;
}


void TransformPos2D(Vec2& pos, float uniformScale, float rotationDegrees, Vec2 const& translation)
{
	Mat44 tranformationMatrix = Mat44::CreateTranslation2D(translation);
	tranformationMatrix.AppendScaleUniform2D(uniformScale);
	tranformationMatrix.AppendZRotationDegrees(rotationDegrees);
	pos = tranformationMatrix.TransformPosition2D(pos);
}


void TransformPos2D(Vec2& pos, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	Vec2 iDisplacement = pos.x * iBasis;
	Vec2 jDisplacement = pos.y * jBasis;
	pos = iDisplacement + jDisplacement + translation;
}


void TransformPosXY3D(Vec3& pos, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	Vec2 posXY = pos.GetVec2();
	TransformPos2D(posXY, uniformScaleXY, rotationDegreesAboutZ, translationXY);

	pos.x = posXY.x;
	pos.y = posXY.y;
}


void TransformPosXY3D(Vec3& pos, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY)
{
	Vec2 posXY = pos.GetVec2();
	TransformPos2D(posXY, iBasis, jBasis, translationXY);

	pos.x = posXY.x;
	pos.y = posXY.y;
}


float SqrtFloat(float a)
{
	return sqrtf(a);
}


float Clamp(float value, float min, float max)
{
	if (value > max)
	{
		value = max;
	}

	if (value < min)
	{
		value = min;
	}

	return value;
}


int Clamp(int value, int min, int max)
{
	if (value > max)
	{
		value = max;
	}

	if (value < min)
	{
		value = min;
	}

	return value;
}


Vec2 Clamp(Vec2 const& value, Vec2 const& min, Vec2 const& max)
{
	float x = Clamp(value.x, min.x, max.x);
	float y = Clamp(value.y, min.y, max.y);
	return Vec2(x, y);
}


float ClampZeroToOne(float value)
{
	if (value > 1.0f)
	{
		value = 1.0f;
	}
	
	if (value < 0.0f)
	{
		value = 0.0f;
	}
	return value;
}


float CosDegrees(float degrees)
{
	float radians = DegreesToRadian(degrees);
	return cosf(radians);
}


float CosRadians(float radians)
{
	return cosf(radians);
}


float SinDegrees(float degrees)
{
	float radians = DegreesToRadian(degrees);
	return sinf(radians);
}


float SinRadians(float radians)
{
	return sinf(radians);
}


float TanDegrees(float degrees)
{
	float radians = DegreesToRadian(degrees);
	return tanf(radians);
}


float ATan2Degrees(float y, float x)
{
	float radians = atan2f(y, x);
	return RadianToDegrees(radians);
}


float ATan2Radians(float y, float x)
{
	return atan2f(y, x);
}


float DegreesToRadian(float degrees)
{
	return degrees * DEGREES_TO_RADIANS_COEFF;
}


float RadianToDegrees(float radians)
{
	return radians * RADIANS_TO_DEGREES_COEFF;
}


float GetDistance2D(Vec2 const& a, Vec2 const& b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;

	float distance = sqrtf(dx * dx + dy * dy);
	return distance;
}


float GetDistanceSquared2D(Vec2 const& a, Vec2 const& b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;

	float distSqrd = (dx * dx + dy * dy);
	return distSqrd;
}


float GetDistance3D(Vec3 const& a, Vec3 const& b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;

	float dist = sqrtf(dx * dx + dy * dy + dz * dz);
	return dist;
}


float GetDistanceXY3D(Vec3 const& a, Vec3 const& b)
{
	return GetDistance2D(Vec2(a.x, a.y), Vec2(b.x, b.y));
}


float GetDistanceSquared3D(Vec3 const& a, Vec3 const& b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;

	float distSqrd = (dx * dx + dy * dy + dz * dz);
	return distSqrd;
}


float GetDistanceXYSquared3D(Vec3 const& a, Vec3 const& b)
{
	return GetDistanceSquared2D(Vec2(a.x, a.y), Vec2(b.x, b.y));
}


bool DoDiscsOverlap2D(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	float sqrdDistBetweenCenters = GetDistanceSquared2D(centerA, centerB);
	float sumOfRadii = radiusA + radiusB;

	return sqrdDistBetweenCenters < Square(sumOfRadii);
}


bool DoesDiskOverlapWithAABB2(Vec2 const& diskCenter, float diskRadius, AABB2 const& aabb2)
{
	Vec2 nearestPointOnAABB2FromDisk = GetNearestPointOnAABB2D(diskCenter, aabb2);
	float distFromNearestPtSqrd = GetDistanceSquared2D(diskCenter, nearestPointOnAABB2FromDisk);
	return distFromNearestPtSqrd < (diskRadius * diskRadius);
}


bool DoAABB2sOverlap(AABB2 const& a, AABB2 const& b)
{
	FloatRange aX(a.m_mins.x, a.m_maxs.x);
	FloatRange aY(a.m_mins.y, a.m_maxs.y);
	
	FloatRange bX(b.m_mins.x, b.m_maxs.x);
	FloatRange bY(b.m_mins.y, b.m_maxs.y);

	return aX.IsOverlappingWith(bX) && aY.IsOverlappingWith(bY);
}


bool DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	float distBetweenSqheresSqrd = GetDistanceSquared3D(centerA, centerB);
	float sumOfRadiiSqrd = (radiusA + radiusB) * (radiusA + radiusB);

	return distBetweenSqheresSqrd < sumOfRadiiSqrd;
}


bool DoAABB3sOverlap3D(AABB3 const& a, AABB3 const& b)
{
	if (b.m_maxs.x <= a.m_mins.x)
		return false;
	if (b.m_mins.x >= a.m_maxs.x)
		return false;
	if (b.m_maxs.y <= a.m_mins.y)
		return false;
	if (b.m_mins.y >= a.m_maxs.y)
		return false;
	if (b.m_maxs.z <= a.m_mins.z)
		return false;
	if (b.m_mins.z >= a.m_maxs.z)
		return false;

	return true;
}


bool DoZCylindersOverlap3D(Vec2 const& a_centerXY, FloatRange const& a_minMaxZ, float a_radius, Vec2 const& b_centerXY, FloatRange const& b_minMaxZ, float b_radius)
{
	bool doCylinderDisksOverlapping = DoDiscsOverlap2D(a_centerXY, a_radius, b_centerXY, b_radius);
	if (!doCylinderDisksOverlapping)
		return false;

	return a_minMaxZ.IsOverlappingWith(b_minMaxZ);
}


bool DoesSphereOverlapWithAABB3D(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& aabb3)
{
	Vec3 nearestPointFromSphereOnAABB3 = GetNearestPointOnAABB3(sphereCenter, aabb3);
	float distanceToNearestPtSqrd = GetDistanceSquared3D(sphereCenter, nearestPointFromSphereOnAABB3);
	return distanceToNearestPtSqrd < (sphereRadius * sphereRadius);
}


bool DoesSphereOverlapWithZCylinder3D(Vec3 const& sphereCenter, float sphereRadius, Vec2 const& cylinderCenterXY, FloatRange const& cylinderMinMaxZ, float cylinderRadius)
{
	Vec3 nearestPointFromSphereOnCylinder = GetNearestPointOnZCylinder3D(sphereCenter, cylinderCenterXY, cylinderMinMaxZ, cylinderRadius);
	float distanceToNearestPtSqrd = GetDistanceSquared3D(sphereCenter, nearestPointFromSphereOnCylinder);
	return distanceToNearestPtSqrd < (sphereRadius* sphereRadius);
}


bool DoesAABB3OverlapWithZCylinder3D(AABB3 const& aabb3, Vec2 const& cylinderCenterXY, FloatRange const& cylinderMinMaxZ, float cylinderRadius)
{
	AABB2 aabb3XY = AABB2(aabb3.m_mins.GetVec2(), Vec2(aabb3.m_maxs.x, aabb3.m_maxs.y));
	bool doesAABB3XYOverlapWithCylinderXY = DoesDiskOverlapWithAABB2(cylinderCenterXY, cylinderRadius, aabb3XY);
	if (!doesAABB3XYOverlapWithCylinderXY)
		return false;

	FloatRange aabb3ZRange = FloatRange(aabb3.m_mins.z, aabb3.m_maxs.z);
	return cylinderMinMaxZ.IsOverlappingWith(aabb3ZRange);
}


bool PushDiskOutOfPoint2D(Vec2& diskCenter, float diskRadius, Vec2 const& staticPoint)
{
	if (!IsPointInsideDisk2D(staticPoint, diskCenter, diskRadius))
	{
		return false;
	}

	Vec2 diskCenterToPoint		= staticPoint - diskCenter;
	float lengthBwCenterAndPoint = diskCenterToPoint.GetLengthAndNormalise();
	if (lengthBwCenterAndPoint == 0.0f)
	{
		return false;
	}
	float excessDistance		= diskRadius - lengthBwCenterAndPoint;
	Vec2 displacement			= diskCenterToPoint * excessDistance;
	diskCenter					-= displacement;
	return true;
}


bool PushDiskOutOfDisk2D(Vec2& mobileDiskCenter, float mobileDiskRadius, Vec2 const& staticDiskCenter, float staticDiskRadius)
{
	if (!DoDiscsOverlap2D(mobileDiskCenter, mobileDiskRadius, staticDiskCenter, staticDiskRadius))
	{
		return false;
	}

	Vec2 lineBetweenCenters		= staticDiskCenter - mobileDiskCenter;
	float lengthBwCenters = lineBetweenCenters.GetLengthAndNormalise();
	if (lengthBwCenters == 0.0f)
	{
		return false;
	}
	float excessDistance		= (mobileDiskRadius + staticDiskRadius) - lengthBwCenters;
	Vec2 displacement			= lineBetweenCenters * excessDistance;
	mobileDiskCenter			-= displacement;
	return true;
}


bool PushDiskOutOfDiskXY3D(Vec3& mobileDiskCenter, float mobileDiskRadius, Vec3 const& staticDiskCenter, float staticDiskRadius)
{
	if (!DoDiscsOverlap2D(mobileDiskCenter.GetVec2(), mobileDiskRadius, staticDiskCenter.GetVec2(), staticDiskRadius))
	{
		return false;
	}

	Vec2 lineBetweenCenters = staticDiskCenter.GetVec2() - mobileDiskCenter.GetVec2();
	float lengthBwCenters = lineBetweenCenters.GetLengthAndNormalise();
	if (lengthBwCenters == 0.0f)
	{
		return false;
	}
	float excessDistance = (mobileDiskRadius + staticDiskRadius) - lengthBwCenters;
	Vec2 displacement = lineBetweenCenters * excessDistance;
	mobileDiskCenter -= displacement.GetVec3();
	return true;
}


bool PushDisksOutOfEachOther2D(Vec2& diskCenterA, float diskRadiusA, Vec2& diskCenterB, float diskRadiusB)
{
	if (!DoDiscsOverlap2D(diskCenterA, diskRadiusA, diskCenterB, diskRadiusB))
	{
		return false;
	}

	Vec2 lineFromAToB			= diskCenterB - diskCenterA;
	float aToBLength = lineFromAToB.GetLengthAndNormalise();
	if (aToBLength == 0.0f)
	{
		return false;
	}
	float excessDistance		= (diskRadiusA + diskRadiusB) - aToBLength;
	float excessDistanceHalf	= excessDistance * 0.5f;
	Vec2 displacement			= lineFromAToB * excessDistanceHalf;
	diskCenterA					-= displacement;
	diskCenterB					+= displacement;
	return true;
}


bool PushDiskOutOfAABB2D(Vec2& mobileDiskCenter, float mobileDiskRadius, AABB2 const& staticBox)
{
	Vec2 nearestPointFromCenter = staticBox.GetNearestPoint(mobileDiskCenter);
	
	if (!IsPointInsideDisk2D(nearestPointFromCenter, mobileDiskCenter, mobileDiskRadius))
	{
		return false;
	}

	PushDiskOutOfPoint2D(mobileDiskCenter, mobileDiskRadius, nearestPointFromCenter);
	return true;
}


float Lerp(float start, float end, float t)
{
	return start + t * (end - start);
}


Rgba8 Lerp(Rgba8 const& start, Rgba8 const& end, float t)
{
	Rgba8 lerpedColor;
	float startColor[4];
	float endColor[4];
	start.GetColorAsFloats(startColor);
	end.GetColorAsFloats(endColor);

	float lerpedR = Lerp(startColor[0], endColor[0], t);
	float lerpedG = Lerp(startColor[1], endColor[1], t);
	float lerpedB = Lerp(startColor[2], endColor[2], t);
	float lerpedA = Lerp(startColor[3], endColor[3], t);

	lerpedColor.r = FloatToByte(lerpedR);
	lerpedColor.g = FloatToByte(lerpedG);
	lerpedColor.b = FloatToByte(lerpedB);
	lerpedColor.a = FloatToByte(lerpedA);

	return lerpedColor;
}


Vec2 Lerp(Vec2 const& start, Vec2 const& end, float t)
{
	float lerpedX = Lerp(start.x, end.x, t);
	float lerpedY = Lerp(start.y, end.y, t);
	return Vec2(lerpedX, lerpedY);
}


float GetFraction(float value, float start, float end)
{
	if (start == end)
	{
		return -99999.0f;
	}

	return (value - start) / (end - start);
}


float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float fraction = GetFraction(inValue, inStart, inEnd);

	return Lerp(outStart, outEnd, fraction);
}


Vec2 RangeMap(Vec2 const& inValue, Vec2 const& inStart, Vec2 const& inEnd, Vec2 const& outStart, Vec2 const& outEnd)
{
	float xFraction = GetFraction(inValue.x, inStart.x, inEnd.x);
	float yFraction = GetFraction(inValue.y, inStart.y, inEnd.y);
	float outX = Lerp(outStart.x, outEnd.x, xFraction);
	float outY = Lerp(outStart.y, outEnd.y, yFraction);
	return Vec2(outX, outY);
}


float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float fraction = GetFraction(inValue, inStart, inEnd);
	fraction = ClampZeroToOne(fraction);
	return Lerp(outStart, outEnd, fraction);
}


Vec2 RangeMapClamped(Vec2 const& inValue, Vec2 const& inStart, Vec2 const& inEnd, Vec2 const& outStart, Vec2 const& outEnd)
{
	float xFraction = GetFraction(inValue.x, inStart.x, inEnd.x);
	float yFraction = GetFraction(inValue.y, inStart.y, inEnd.y);
	xFraction = ClampZeroToOne(xFraction);
	yFraction = ClampZeroToOne(yFraction);
	float outX = Lerp(outStart.x, outEnd.x, xFraction);
	float outY = Lerp(outStart.y, outEnd.y, yFraction);
	return Vec2(outX, outY);
}


float ByteToFloatNormalised(unsigned char byteValue)
{
	float floatValue = ClampZeroToOne( (float) byteValue * (1.0f / MAX_UNSIGNED_CHAR_VALUE) );
	return floatValue;
}


unsigned char FloatToByte(float floatValue)
{
	if (floatValue >= 1.0f)
		return 255;

	if (floatValue <= 0.0f)
		return 0;
	
	int intValue = RoundDownToInt(floatValue * 256.0f);
	return (unsigned char) intValue;
}


float RangeMapClamped_SmoothStart2(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float smoothStartT = GetFraction(inValue, inStart, inEnd);
	smoothStartT = SmoothStart2(smoothStartT);
	smoothStartT = ClampZeroToOne(smoothStartT);
	return Lerp(outStart, outEnd, smoothStartT);
}


float RangeMap_SmoothStart2(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float smoothStartT = GetFraction(inValue, inStart, inEnd);
	smoothStartT = SmoothStart2(smoothStartT);
	return Lerp(outStart, outEnd, smoothStartT);
}


float RangeMap_SmoothStop2(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float smoothStartT = GetFraction(inValue, inStart, inEnd);
	smoothStartT = SmoothStop2(smoothStartT);
	return Lerp(outStart, outEnd, smoothStartT);
}


float RangeMap_SmoothStop3(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float smoothStartT = GetFraction(inValue, inStart, inEnd);
	smoothStartT = SmoothStop3(smoothStartT);
	return Lerp(outStart, outEnd, smoothStartT);
}


float RangeMap_SmoothStop4(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float smoothStartT = GetFraction(inValue, inStart, inEnd);
	smoothStartT = SmoothStop4(smoothStartT);
	return Lerp(outStart, outEnd, smoothStartT);
}


float RangeMapClamped_SmoothStop4(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float smoothStartT = GetFraction(inValue, inStart, inEnd);
	smoothStartT = SmoothStop4(smoothStartT);
	smoothStartT = ClampZeroToOne(smoothStartT);
	return Lerp(outStart, outEnd, smoothStartT);
}


float SmoothStop3(float t)
{
	float oneMinusT = 1 - t;
	return 1 - oneMinusT * oneMinusT * oneMinusT;
}


float RangeMapClamped_SmoothStart3(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float smoothStartT = GetFraction(inValue, inStart, inEnd);
	smoothStartT = SmoothStart3(smoothStartT);
	smoothStartT = ClampZeroToOne(smoothStartT);
	return Lerp(outStart, outEnd, smoothStartT);
}


float RangeMapClamped_SmoothStart4(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float smoothStartT = GetFraction(inValue, inStart, inEnd);
	smoothStartT = SmoothStart4(smoothStartT);
	smoothStartT = ClampZeroToOne(smoothStartT);
	return Lerp(outStart, outEnd, smoothStartT);
}


float SmoothStart2(float t)
{
	return t * t;
}


float SmoothStart3(float t)
{
	return t * t * t;
}


float SmoothStart4(float t)
{
	return t * t * t * t;
}


float SmoothStop2(float t)
{
	float oneMinusT = 1 - t;
	return 1 - oneMinusT * oneMinusT;
}


float SmoothStop4(float t)
{
	float oneMinusT = 1 - t;
	return 1 - oneMinusT * oneMinusT * oneMinusT * oneMinusT;
}


float SmoothStep3(float t)
{
	return (3.0f * t * t) - (2.0f * t * t * t);
}


float Get1dPerlinNoise(float position, float scale /*= 1.f*/, unsigned int numOctaves /*= 1*/, float octavePersistence /*= 0.5f*/, float octaveScale /*= 2.f*/, bool renormalize /*= true*/, unsigned int seed /*= 0*/)
{
	return Compute1dPerlinNoise(position, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
}


float Get2dPerlinNoise(float posX, float posY, float scale /*= 1.f*/, unsigned int numOctaves /*= 1*/, float octavePersistence /*= 0.5f*/, float octaveScale /*= 2.f*/, bool renormalize /*= true*/, unsigned int seed /*= 0*/)
{
	return Compute2dPerlinNoise(posX, posY, scale, numOctaves, octavePersistence, octaveScale, renormalize, seed);
}


int RoundDownToInt(float value)
{
	float integer = floor(value);
	return static_cast<int>(integer);
}


int RoundUpToInt(float value)
{
	float integer = ceil(value);
	return static_cast<int>(integer);
}


int RoundToNearestInt(float value)
{
	float lowerInt = floor(value);
	float upperInt = ceil(value);

	if ( upperInt - value >= value - lowerInt)
	{
		return static_cast<int>(lowerInt);
	}
	else
	{
		return static_cast<int>(upperInt);
	}
}


int GetMax(int a, int b)
{
	if (a > b)
		return a;
	return b;
}


float GetMax(float a, float b)
{
	if (a > b)
		return a;
	return b;
}


uint32_t GetMax(uint32_t a, uint32_t b)
{
	if (a > b)
		return a;
	return b;
}


float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return (a.x * b.x) + (a.y * b.y);
}


float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}


float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}


float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	Vec2 normalisedA = a.GetNormalized();
	Vec2 normalisedB = b.GetNormalized();
	float dotProduct = DotProduct2D(normalisedA, normalisedB);
	
	// a.b = |a| * |b| * cos(theta) = a.x*b.x + a.y*b.y

	float cosTheta = dotProduct;
	float thetaRadians = acosf(cosTheta);
	float thetaDegrees = RadianToDegrees(thetaRadians);
	return thetaDegrees;
}


float GetProjectedLength2D(Vec2 const& vecToProject, Vec2 const& vecToProjectOn)
{
	Vec2 vecToProjectOnNormalised = vecToProjectOn.GetNormalized();
	return DotProduct2D(vecToProject, vecToProjectOnNormalised);
}


Vec2 GetProjectedOnto2D(Vec2 const& vecToProject, Vec2 const& vecToProjectOn)
{
	Vec2 vecToProjectOnNormalised	= vecToProjectOn.GetNormalized();
	float projectedLength			= DotProduct2D(vecToProject, vecToProjectOnNormalised);

	return vecToProjectOnNormalised * projectedLength;
}


Vec3 GetProjectedOnto3D(Vec3 const& vecToProject, Vec3 const& vecToProjectOn)
{
	float projectedLength = DotProduct3D(vecToProject, vecToProjectOn);
	Vec3 vecToProjectOnDir = vecToProjectOn.GetNormalized();
	return vecToProjectOnDir * projectedLength;
}


int GetTaxiCabLength2D(IntVec2 const& from, IntVec2 const& to)
{
	int distanceX = to.x - from.x;
	int distanceY = to.y - from.y;

	return abs(distanceX) + abs(distanceY);
}


float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	return a.x * b.y - a.y * b.x;
}


Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	float x = a.y * b.z - a.z * b.y;
	float y = a.z * b.x - a.x * b.z;
	float z = a.x * b.y - a.y * b.x;
	return Vec3(x, y, z);
}


Vec2 GetNearestPointOnDisk2D(Vec2 const& referencePos, Vec2 const& diskCenter, float diskRadius)
{
	if (IsPointInsideDisk2D(referencePos, diskCenter, diskRadius))
	{
		return referencePos;
	}

	Vec2 centerToRefPoint = referencePos - diskCenter;
	Vec2 directionTowardsRefPoint = centerToRefPoint.GetNormalized();

	return diskCenter + (directionTowardsRefPoint * diskRadius);
}


Vec2 GetNearestPointOnInfiniteLine2D(Vec2 const& referencePoint, LineSegment2 const& infiniteLine)
{
	Vec2 infiniteLineStart	= infiniteLine.m_start;
	Vec2 infiniteLineVec	= infiniteLine.GetLineVector();
	Vec2 startToPointVec	= referencePoint - infiniteLineStart;

	return (infiniteLineStart + GetProjectedOnto2D(startToPointVec, infiniteLineVec));
}


Vec2 GetNearentPointOnLineSegment2D(Vec2 const& referencePoint, LineSegment2 const& lineSegment)
{
	Vec2 lineSegmentEnd		= lineSegment.m_end;
	Vec2 lineSegmentStart	= lineSegment.m_start;
	Vec2 lineSegmentVec		= lineSegment.GetLineVector();

	Vec2 endToPointVec = referencePoint - lineSegmentEnd;
	if (DotProduct2D(lineSegmentVec, endToPointVec) >= 0.0f)
	{
		return lineSegmentEnd;
	}

	Vec2 startToPointVec = referencePoint - lineSegmentStart;
	if (DotProduct2D(lineSegmentVec, startToPointVec) <= 0.0f)
	{
		return lineSegmentStart;
	}

	return (lineSegmentStart + GetProjectedOnto2D(startToPointVec, lineSegmentVec));
}


Vec2 GetNearestPointOnCapsule2D(Vec2 const& referencePoint, Capsule2 const& capsule)
{
	Vec2 nearestPointOnCapsuleBone	= GetNearentPointOnLineSegment2D(referencePoint, capsule.m_bone);
	Vec2 boneToReferencePointVec = referencePoint - nearestPointOnCapsuleBone;
	boneToReferencePointVec.ClampLength(capsule.m_radius);
	return nearestPointOnCapsuleBone + boneToReferencePointVec;
}


Vec2 GetNearestPointOnOBB2D(Vec2 const& referencePoint, OBB2 const& obb2)
{
	return obb2.GetNearestPointForWorldPos(referencePoint);
}


Vec2 GetNearestPointOnPlane2(Vec2 const& refPt, Plane2 const& plane)
{
	float refPtAltitude = plane.GetShortestDistanceFromPlane(refPt);
	return refPt + -plane.m_fwdNormal * refPtAltitude;
}


Vec3 GetNearestPointOnAABB3(Vec3 const& referencePoint, AABB3 const& aabb3)
{
	float nearestX = Clamp(referencePoint.x, aabb3.m_mins.x, aabb3.m_maxs.x);
	float nearestY = Clamp(referencePoint.y, aabb3.m_mins.y, aabb3.m_maxs.y);
	float nearestZ = Clamp(referencePoint.z, aabb3.m_mins.z, aabb3.m_maxs.z);
	return Vec3(nearestX, nearestY, nearestZ);
}


Vec3 GetNearestPointOnZSphere3D(Vec3 const& referencePoint, Vec3 const& center, float radius)
{
	Vec3 centerToPoint = referencePoint - center;
	float distSquare = centerToPoint.GetLengthSquared();
	Vec3 centerToPointDir = centerToPoint.GetNormalized();
	if (distSquare > radius * radius)
	{
		return center + centerToPointDir * radius;
	}
	else
	{
		return referencePoint;
	}
}


Vec3 GetNearestPointOnZCylinder3D(Vec3 const& referencePoint, Vec2 const& cylinderCenterXY, FloatRange const& minMaxZ, float radius)
{
	float nearestZ = Clamp(referencePoint.z, minMaxZ.m_min, minMaxZ.m_max);

Vec2 refPoint2D = referencePoint.GetVec2();
Vec2 nearestPointXY = GetNearestPointOnDisk2D(refPoint2D, cylinderCenterXY, radius);

return Vec3(nearestPointXY.x, nearestPointXY.y, nearestZ);
}


bool IsPointInsideAABB3(Vec3 const& refPoint, AABB3 const& aabb3)
{
	return	refPoint.x > aabb3.m_mins.x &&
		refPoint.x < aabb3.m_maxs.x&&
		refPoint.y > aabb3.m_mins.y &&
		refPoint.y < aabb3.m_maxs.y&&
		refPoint.z > aabb3.m_mins.z &&
		refPoint.z < aabb3.m_maxs.z;
}


bool IsPointInsideZCylinder3D(Vec3 const& referencePoint, Vec2 const& cylinderCenterXY, FloatRange const& minMaxZ, float radius)
{
	Vec2 referencePtXY = referencePoint.GetVec2();
	float xyDistSqrd = (cylinderCenterXY - referencePtXY).GetLengthSquared();

	return	xyDistSqrd < Square(radius) &&
		referencePoint.z > minMaxZ.m_min &&
		referencePoint.z < minMaxZ.m_max;
}


Vec2 GetNearestPointOnAABB2D(Vec2 const& referencePoint, AABB2 const& aabb2)
{
	return aabb2.GetNearestPoint(referencePoint);
}


bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (!IsPointInsideDisk2D(point, sectorTip, sectorRadius))
	{
		return false;
	}

	Vec2 lineFromTipToPoint = point - sectorTip;
	float sectorApertureDegreesHalf = sectorApertureDegrees * 0.5f;
	float angularDisplacement = GetShortestAngularDisplacementDegrees(sectorForwardDegrees, lineFromTipToPoint.GetOrientationDegrees());
	if (angularDisplacement < -sectorApertureDegreesHalf || angularDisplacement > sectorApertureDegreesHalf)
	{
		return false;
	}

	return true;
}


bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	float sectorForwardOrientationDegrees = sectorForwardNormal.GetOrientationDegrees();
	return IsPointInsideOrientedSector2D(point, sectorTip, sectorForwardOrientationDegrees, sectorApertureDegrees, sectorRadius);
}


bool IsPointInsideDisk2D(Vec2 const& point, Vec2 const& diskCenter, float diskRadius)
{
	float centerToPointDistSquared = (point - diskCenter).GetLengthSquared();
	float diskRadiusSquared = diskRadius * diskRadius;
	if (centerToPointDistSquared >= diskRadiusSquared)
	{
		return false;
	}

	return true;
}


bool IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule)
{
	Vec2 nearestPointOnCapsuleBone = GetNearentPointOnLineSegment2D(point, capsule.m_bone);
	Vec2 nearestToReferencePointVec = point - nearestPointOnCapsuleBone;
	if (nearestToReferencePointVec.GetLength() >= capsule.m_radius)
	{
		return false;
	}

	return true;
}


bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& obb2)
{
	return obb2.IsPointInside(point);
}


bool IsPointInsideConvexHull2(Vec2 const& refPt, ConvexHull2 const& convexHull)
{
	std::vector<Plane2> const& planes = convexHull.m_unorderedPlanes;
	for (int i = 0; i < (int) planes.size(); i++)
	{
		Plane2 const& plane = planes[i];
		if (!plane.IsPointBehind(refPt))
			return false;
	}

	return true;
}


bool IsPointInsideConvexHull2(Vec2 const& refPt, ConvexHull2 const& convexHull, Plane2 const& ignoreThisPlane)
{
	std::vector<Plane2> const& planes = convexHull.m_unorderedPlanes;
	for (int i = 0; i < (int) planes.size(); i++)
	{
		Plane2 const& plane = planes[i];
		if(plane == ignoreThisPlane)
			continue;

		if (!plane.IsPointBehind(refPt))
			return false;
	}

	return true;
}


bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& aabb2)
{
	return aabb2.IsPointInside(point);
}


float GetShortestAngularDisplacementDegrees(float startDegrees, float endDegrees)
{
	float angularDisplacement = endDegrees - startDegrees;
	
	while (angularDisplacement > 180.0f)
	{
		angularDisplacement -= 360.0f;
	}
	while (angularDisplacement < -180.0f)
	{
		angularDisplacement += 360.0f;
	}

	return angularDisplacement;
}


float GetTurnedTowardDegrees(float currentDegrees, float targetDegrees, float maxAllowedTurningDegrees)
{
	maxAllowedTurningDegrees = abs(maxAllowedTurningDegrees);
	float totalAngularDisplacement = GetShortestAngularDisplacementDegrees(currentDegrees, targetDegrees);
	
	float orientationAfterTurning = currentDegrees + maxAllowedTurningDegrees;
	if (totalAngularDisplacement < 0.0f)
	{
		orientationAfterTurning = currentDegrees - maxAllowedTurningDegrees;
	}
	if (maxAllowedTurningDegrees > abs(totalAngularDisplacement))
	{
		orientationAfterTurning = targetDegrees;
	}

	return orientationAfterTurning;
}


Mat44 GetModelMatrixFromUpVector(Vec3 const& upVector, Vec3 const& translation)
{
	Vec3 initialCrossVec = Vec3(1.0f, 0.0f, 0.0f);
	float dotWithInitialVec = DotProduct3D(initialCrossVec, upVector);
	if (dotWithInitialVec == 1.0f || dotWithInitialVec == -1.0f)
	{
		initialCrossVec = Vec3(0.0f, 1.0f, 0.0f);
	}

	Vec3 leftVec = CrossProduct3D(upVector, initialCrossVec);
	leftVec.Normalize();

	Vec3 forwardVec = CrossProduct3D(leftVec, upVector);
	forwardVec.Normalize();

	Mat44 modelMatrix = Mat44(forwardVec, leftVec, upVector, translation);
	return modelMatrix;
}
