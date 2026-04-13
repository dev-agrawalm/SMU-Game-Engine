#pragma once
#include <stdint.h>

struct Vec2;
struct Vec3;
struct Vec4;
struct IntVec2;
struct AABB2;
struct AABB3;
struct LineSegment2;
struct Capsule2;
struct OBB2;
struct Rgba8;
struct FloatRange;
struct Plane2;
struct ConvexHull2;
struct Mat44;

//MATH CONSTANTS
constexpr float PI							= 3.141592653589793238f;
constexpr float DEGREES_0					= 0.0f;
constexpr float DEGREES_90					= 90.0f;
constexpr float DEGREES_180					= 180.0f;
constexpr float DEGREES_270					= 270.0f;
constexpr float DEGREES_360					= 360.0f;
constexpr float DEGREES_TO_RADIANS_COEFF	= PI / 180.0f;
constexpr float RADIANS_TO_DEGREES_COEFF	= 180.0f / PI;

//MACROS
#define Square(x) x * x

//General Math functions
int		Clamp(int value, int min, int max);
Vec2	Clamp(Vec2 const& value, Vec2 const& min, Vec2 const& max);
float	Clamp(float value, float min, float max);
float	ClampZeroToOne(float value);

float	SqrtFloat(float a);

int		RoundDownToInt(float value);
int		RoundUpToInt(float value);
int		RoundToNearestInt(float value);

int			GetMax(int a, int b);
float		GetMax(float a, float b);
uint32_t	GetMax(uint32_t a, uint32_t b);

int			GetMin(int a, int b);
float		GetMin(float a, float b);
uint32_t	GetMin(uint32_t a, uint32_t b);
float		GetAbsoluteMin(float a, float b);

//Transformation Utils
void	TransformPos2D(Vec2& pos, float uniformScale, float rotationDegrees, Vec2 const& translation);
void	TransformPos2D(Vec2& pos, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);
void	TransformPosXY3D(Vec3& pos, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY);
void	TransformPosXY3D(Vec3& pos, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY);

//Angle Utils
float	CosDegrees(float degrees);
float	CosRadians(float radians);
float	SinDegrees(float degrees);
float	SinRadians(float radians);
float	TanDegrees(float degrees);
float	ATan2Degrees(float y, float x);
float	ATan2Radians(float y, float x);
float	DegreesToRadian(float degrees); //converts degrees to radian
float	RadianToDegrees(float radians); //converts radians to degrees
float	GetShortestAngularDisplacementDegrees(float startDegrees, float endDegrees);
float	GetTurnedTowardDegrees(float currentDegrees, float targetDegrees, float maxAllowedTurningDegrees);

//Vector Utils
float	GetDistance2D(Vec2 const& a, Vec2 const& b);
float	GetDistanceSquared2D(Vec2 const& a, Vec2 const& b);
float	GetDistance3D(Vec3 const& a, Vec3 const& b);
float	GetDistanceXY3D(Vec3 const& a, Vec3 const& b);
float	GetDistanceSquared3D(Vec3 const& a, Vec3 const& b);
float	GetDistanceXYSquared3D(Vec3 const& a, Vec3 const& b);
float	DotProduct2D(Vec2 const& a, Vec2 const& b);
float	DotProduct3D(Vec3 const& a, Vec3 const& b);
float	DotProduct4D(Vec4 const& a, Vec4 const& b);
float	GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);
float	GetProjectedLength2D(Vec2 const& vecToProject, Vec2 const& vecToProjectOn);
Vec2	GetProjectedOnto2D(Vec2 const& vecToProject, Vec2 const& vecToProjectOn);
Vec3	GetProjectedOnto3D(Vec3 const& vecToProject, Vec3 const& vecToProjectOn);
int		GetTaxiCabLength2D(IntVec2 const& from, IntVec2 const& to);
float	CrossProduct2D(Vec2 const& a, Vec2 const& b);
Vec3	CrossProduct3D(Vec3 const& a, Vec3 const& b);

//2D Geometric Queries
Vec2	GetNearestPointOnInfiniteLine2D(Vec2 const& referencePoint, LineSegment2 const& infiniteLine);
Vec2	GetNearentPointOnLineSegment2D(Vec2 const& referencePoint, LineSegment2 const& lineSegment);
Vec2	GetNearestPointOnDisk2D(Vec2 const& referencePos, Vec2 const& diskCenter, float diskRadius);
Vec2	GetNearestPointOnAABB2D(Vec2 const& referencePoint, AABB2 const& aabb2);
Vec2	GetNearestPointOnCapsule2D(Vec2 const& referencePoint, Capsule2 const& capsule);
Vec2	GetNearestPointOnOBB2D(Vec2 const& referencePoint, OBB2 const& obb2);
Vec2	GetNearestPointOnPlane2(Vec2 const& refPt, Plane2 const& plane);

bool	IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool	IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool	IsPointInsideDisk2D(Vec2 const& point, Vec2 const& diskCenter, float diskRadius);
bool	IsPointInsideAABB2D(Vec2 const& point, AABB2 const& aabb2);
bool	IsPointInsideCapsule2D(Vec2 const& point, Capsule2 const& capsule);
bool	IsPointInsideOBB2D(Vec2 const& point, OBB2 const& obb2);
bool	IsPointInsideConvexHull2(Vec2 const& refPt, ConvexHull2 const& convexHull);
bool	IsPointInsideConvexHull2(Vec2 const& refPt, ConvexHull2 const& convexHull, Plane2 const& ignoreThisPlane);

bool	DoDiscsOverlap2D(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool	DoesDiskOverlapWithAABB2(Vec2 const& diskCenter, float diskRadius, AABB2 const& aabb2);
bool	DoAABB2sOverlap(AABB2 const& a, AABB2 const& b);

bool	PushDiskOutOfPoint2D(Vec2& diskCenter, float diskRadius, Vec2 const& staticPoint);
bool	PushDiskOutOfDisk2D(Vec2& mobileDiskCenter, float mobileDiskRadius, Vec2 const& staticDiskCenter, float staticDiskRadius);
bool	PushDiskOutOfDiskXY3D(Vec3& mobileDiskCenter, float mobileDiskRadius, Vec3 const& staticDiskCenter, float staticDiskRadius);
bool	PushDisksOutOfEachOther2D(Vec2& diskCenterA, float diskRadiusA, Vec2& diskCenterB, float diskRadiusB);
bool	PushDiskOutOfAABB2D(Vec2& mobileDiskCenter, float mobileDiskRadius, AABB2 const& staticBox);

//3D Geometric Queries
Vec3	GetNearestPointOnAABB3(Vec3 const& referencePoint, AABB3 const& aabb3);
Vec3	GetNearestPointOnZSphere3D(Vec3 const& referencePoint, Vec3 const& center, float radius);
Vec3	GetNearestPointOnZCylinder3D(Vec3 const& referencePoint, Vec2 const& cylinderCenterXY, FloatRange const& minMaxZ, float radius);

bool	IsPointInsideAABB3(Vec3 const& refPoint, AABB3 const& aabb3);
bool	IsPointInsideZCylinder3D(Vec3 const& referencePoint, Vec2 const& cylinderCenterXY, FloatRange const& minMaxZ, float radius);

bool	DoSpheresOverlap3D(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool	DoAABB3sOverlap3D(AABB3 const& a, AABB3 const& b);
bool	DoZCylindersOverlap3D(Vec2 const& a_centerXY, FloatRange const& a_minMaxZ, float a_radius, Vec2 const& b_centerXY, FloatRange const& b_minMaxZ, float b_radius);
bool	DoesSphereOverlapWithAABB3D(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& aabb3);
bool	DoesSphereOverlapWithZCylinder3D(Vec3 const& sphereCenter, float sphereRadius, Vec2 const& cylinderCenterXY, FloatRange const& cylinderMinMaxZ, float cylinderRadius);
bool	DoesAABB3OverlapWithZCylinder3D(AABB3 const& aabb3, Vec2 const& cylinderCenterXY, FloatRange const& cylinderMinMaxZ, float cylinderRadius);

//Interpolation Queries
float	Lerp(float start, float end, float t);
Rgba8	Lerp(Rgba8 const& start, Rgba8 const& end, float t);
Vec2	Lerp(Vec2 const& start, Vec2 const& end, float t);
float	GetFraction(float value, float start, float end);
float	RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd);
Vec2	RangeMap(Vec2 const& inValue, Vec2 const& inStart, Vec2 const& inEnd, Vec2 const& outStart, Vec2 const& outEnd);
float	RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd);
Vec2	RangeMapClamped(Vec2 const& inValue, Vec2 const& inStart, Vec2 const& inEnd, Vec2 const& outStart, Vec2 const& outEnd);

float	ByteToFloatNormalised(unsigned char byteValue);
unsigned char	FloatToByte(float floatValue);

//easing functions
float RangeMap_SmoothStart2(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMap_SmoothStop2(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMap_SmoothStop3(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMap_SmoothStop4(float inValue, float inStart, float inEnd, float outStart, float outEnd);

float RangeMapClamped_SmoothStop4(float inValue, float inStart, float inEnd, float outStart, float outEnd);

float RangeMapClamped_SmoothStart2(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped_SmoothStart3(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped_SmoothStart4(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float SmoothStart2(float t);
float SmoothStart3(float t);
float SmoothStart4(float t);
float SmoothStop2(float t);
float SmoothStop3(float t);
float SmoothStop4(float t);
float SmoothStep3(float t);

//noise functions
//wrappers for squirrel's noise functions
//return from -1 to 1 if renormalized
float Get1dPerlinNoise(float position, float scale = 1.f, unsigned int numOctaves = 1, float octavePersistence = 0.5f, float octaveScale = 2.f, bool renormalize = true, unsigned int seed = 0);
float Get2dPerlinNoise(float posX, float posY, float scale = 1.f, unsigned int numOctaves = 1, float octavePersistence = 0.5f, float octaveScale = 2.f, bool renormalize = true, unsigned int seed = 0);

//matrix utils
Mat44 GetModelMatrixFromUpVector(Vec3 const& upVector, Vec3 const& translation);