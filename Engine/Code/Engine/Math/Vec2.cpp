#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"

Vec2 const Vec2::ZERO	= Vec2(0.0f, 0.0f);
Vec2 const Vec2::ONE	= Vec2(1.0f, 1.0f);

Vec2::~Vec2()
{
}


Vec2::Vec2()
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2(  Vec2 const& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}


Vec2 const Vec2::MakeFromPolarDegrees(float degrees, float length)
{
	float x = length * CosDegrees(degrees);
	float y = length * SinDegrees(degrees);

	return Vec2(x,y);
}


Vec2 const Vec2::MakeFromPolarRadians(float radians, float length)
{
	float x = length * CosRadians(radians);
	float y = length * SinRadians(radians);

	return Vec2(x, y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + (  Vec2 const& vecToAdd ) const
{
	return Vec2(x + vecToAdd.x, y + vecToAdd.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-(  Vec2 const& vecToSubtract ) const
{
	return Vec2(x - vecToSubtract.x, y - vecToSubtract.y);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2(-x, -y);
}


float Vec2::GetLength() const
{
	float length = SqrtFloat((x * x) + (y * y));
	return length;
}


float Vec2::GetLengthSquared() const
{
	float magnitudeSqrd = x * x + y * y;
	return magnitudeSqrd;
}


float Vec2::GetOrientationDegrees() const
{
	float thetaDegrees = ATan2Degrees(y, x);
	return thetaDegrees;
}


float Vec2::GetOrientationRadians() const
{
	float thetaRadians = ATan2Radians(y, x);
	return thetaRadians;
}


Vec2 Vec2::GetRotated90Degrees() const
{
	return Vec2(-y, x);
}


Vec2 Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2(y, -x);
}


Vec2 Vec2::GetRotatedDegrees(float degrees) const
{
	float thetaDegrees = GetOrientationDegrees();
	thetaDegrees += degrees;
	float length = GetLength();

	return MakeFromPolarDegrees(thetaDegrees, length);
}


Vec2 Vec2::GetRotatedRadians(float radians) const
{
	float thetaRadians = GetOrientationRadians();
	thetaRadians += radians;
	float length = GetLength();

	return MakeFromPolarRadians(thetaRadians, length);
}


void Vec2::SetOrientationDegrees(float degrees)
{
	float length = GetLength();
	x = length * CosDegrees(degrees);
	y = length * SinDegrees(degrees);
}


void Vec2::SetOrientationRadians(float radians)
{
	float length = GetLength();
	x = length * CosRadians(radians);
	y = length * SinRadians(radians);
}


void Vec2::SetPolarDegrees(float degrees, float length)
{
	x = length * CosDegrees(degrees);
	y = length * SinDegrees(degrees);
}


void Vec2::SetPolarRadians(float radians, float length)
{
	x = length * CosRadians(radians);
	y = length * SinRadians(radians);
}


void Vec2::RotateDegrees(float degrees)
{
	Vec2 rotatedVec = GetRotatedDegrees(degrees);
	x = rotatedVec.x;
	y = rotatedVec.y;
}


void Vec2::RotateRadians(float radians)
{
	Vec2 rotatedVec = GetRotatedRadians(radians);
	x = rotatedVec.x;
	y = rotatedVec.y;
}


void Vec2::Rotate90Degrees()
{
	Vec2 rotatedVec = GetRotated90Degrees();
	x = rotatedVec.x;
	y = rotatedVec.y;
}


void Vec2::RotateMinus90Degrees()
{
	Vec2 rotatedVec = GetRotatedMinus90Degrees();
	x = rotatedVec.x;
	y = rotatedVec.y;
}


Vec2 Vec2::GetClamped(float maxLength) const
{
	float thetaDegrees = GetOrientationDegrees();
	float length = GetLength();
	
	if (length > maxLength)
	{
		length = maxLength;
	}

	return MakeFromPolarDegrees(thetaDegrees, length);
}


Vec2 Vec2::GetNormalized() const
{
	float length = GetLength();
	if (length == 0.0f)
		return Vec2::ZERO;

	float inverseLength = 1.0f / length;

	return Vec2(x * inverseLength, y * inverseLength);
}


Vec2 Vec2::GetReflected(Vec2 const& impactSurfaceNormal) const
{
	float impactSurfaceNormalComponentLength = DotProduct2D(*this, impactSurfaceNormal);
	Vec2 impactSurfaceNormalComponent = impactSurfaceNormalComponentLength * impactSurfaceNormal;
	Vec2 impactSurfaceTangetComponent = *this - impactSurfaceNormalComponent;
	Vec2 reflectedVector = impactSurfaceTangetComponent + (-impactSurfaceNormalComponent);
	return reflectedVector;
}


Vec3 Vec2::GetVec3() const
{
	return Vec3(x, y, 0.0f);
}


IntVec2 Vec2::GetIntVec2() const
{
	int intX = RoundDownToInt(x);
	int intY = RoundDownToInt(y);
	return IntVec2(intX, intY);
}


float Vec2::GetLengthAndNormalise()
{
	float length = GetLength();

	if (length == 0.0f)
	{
		x = 0.0f;
		y = 0.0f;
	}
	else
	{
		float inverseLength = 1.0f / length;
		x *= inverseLength;
		y *= inverseLength;
	}

	return length;
}


void Vec2::SetFromText(std::string text)
{
	Strings floatStrings = SplitStringOnDelimiter(text, ',');
	GUARANTEE_RECOVERABLE(floatStrings.size() == 2, "Text for Vec2 has more than 1 comma which is against the acceptable format");

// 	floatStrings[0] = RemoveWhitespacesFromString(floatStrings[0]);
// 	floatStrings[1] = RemoveWhitespacesFromString(floatStrings[1]);

	x = static_cast<float>(atof(floatStrings[0].c_str()));
	y = static_cast<float>(atof(floatStrings[1].c_str()));
}


void Vec2::SetLength(float length)
{
	float thetaDegrees = GetOrientationDegrees();

	Vec2 newVec = MakeFromPolarDegrees(thetaDegrees, length);

	x = newVec.x;
	y = newVec.y;
}


void Vec2::ClampLength(float maxLength)
{
	Vec2 newVec = GetClamped(maxLength);

	x = newVec.x;
	y = newVec.y;
}


void Vec2::Normalize()
{
	float length = GetLength();
	float inverseLength = 1.0f / length;
	x *= inverseLength;
	y *= inverseLength;
}


void Vec2::Reflect(Vec2 const& impactSurfaceNormal)
{
	float impactSurfaceNormalComponentLength = DotProduct2D(*this, impactSurfaceNormal);
	Vec2 impactSurfaceNormalComponent = impactSurfaceNormalComponentLength * impactSurfaceNormal;
	Vec2 impactSurfaceTangetComponent = *this - impactSurfaceNormalComponent;
	Vec2 reflectedVector = impactSurfaceTangetComponent + (-impactSurfaceNormalComponent);

	*this = reflectedVector;
}


float Vec2::NormalizeAndGetPreviousLength()
{
	float length = GetLength();
	float inverseLength = 1.0f / length;
	x *= inverseLength;
	y *= inverseLength;

	return length;
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2(x * uniformScale, y * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*(  Vec2 const& vecToMultiply ) const
{
	return Vec2(x * vecToMultiply.x, y * vecToMultiply.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2(x / inverseScale, y / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=(  Vec2 const& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=(  Vec2 const& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=(  Vec2 const& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale,  Vec2 const& vecToScale )
{
	return vecToScale * uniformScale;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==(  Vec2 const& compare ) const
{
	if (x == compare.x && y == compare.y)
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=(  Vec2 const& compare ) const
{
	//return !(*this == compare);

	if (x == compare.x && y == compare.y)
	{
		return false;
	}

	return true;
}
