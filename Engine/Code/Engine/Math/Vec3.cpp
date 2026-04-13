#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Vec3 const Vec3::ZERO = Vec3(0.0f, 0.0f, 0.0f);

Vec3::~Vec3()
{
	//empty
}


Vec3::Vec3()
{
	//empty
}


//-----------------------------------------------------------------------------------------------
Vec3::Vec3(const Vec3& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
{}


//-----------------------------------------------------------------------------------------------
Vec3::Vec3(float initialX, float initialY, float initalZ)
	: x(initialX)
	, y(initialY)
	, z(initalZ)
{}


Vec3::Vec3(float initialX, float initialY)
	: x(initialX)
	, y(initialY)
	, z(0.0f)
{

}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator + (const Vec3& vecToAdd) const
{
	return Vec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-(const Vec3& vecToSubtract) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(float uniformScale) const
{
	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(const Vec3& vecToMultiply) const
{
	return Vec3(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/(float inverseScale) const
{
	float scale = 1.0f / inverseScale;
	return Vec3(x * scale, y * scale, z * scale);
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=(const Vec3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=(const Vec3& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=(const float uniformDivisor)
{
	float uniformDivisorInverse = 1.0f / uniformDivisor;
	x *= uniformDivisorInverse;
	y *= uniformDivisorInverse;
	z *= uniformDivisorInverse;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=(const Vec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


float Vec3::GetLength() const
{
	float length = SqrtFloat(x * x + y * y + z * z);
	return length;
}


float Vec3::GetLengthXY() const
{
	float length = SqrtFloat(x * x + y * y);
	return length;
}


float Vec3::GetLengthSquared() const
{
	float lengthSqrd = (x * x + y * y + z * z);
	return lengthSqrd;
}


float Vec3::GetLengthXYSquared() const
{
	float lengthSqrd = (x * x + y * y);
	return lengthSqrd;
}


float Vec3::GetAngleAboutZDegrees() const
{
	Vec2 tempVec(x, y);

	return tempVec.GetOrientationDegrees();
}


float Vec3::GetAngleAboutZRadians() const
{
	Vec2 tempVec(x, y);
	return tempVec.GetOrientationRadians();
}


Vec3 Vec3::GetRotatedAboutZDegrees(float degrees) const
{
	Vec2 tempVec(x, y);
	tempVec.RotateDegrees(degrees);

	return Vec3(tempVec.x, tempVec.y, z);
}


Vec3 Vec3::GetRotatedAboutZRadians(float radians) const
{
	Vec2 tempVec(x, y);
	tempVec.RotateRadians(radians);

	return Vec3(tempVec.x, tempVec.y, z);
}


Vec2 Vec3::GetVec2() const
{
	return Vec2(x, y);
}


void Vec3::SetFromText(std::string text)
{
	Strings floatStrings = SplitStringOnDelimiter(text, ',');
	GUARANTEE_RECOVERABLE(floatStrings.size() == 3, "Text for Vec3 has more than 2 comma which is against the acceptable format");

	x = static_cast<float>( atof(floatStrings[0].c_str()) );
	y = static_cast<float>( atof(floatStrings[1].c_str()) );
	z = static_cast<float>( atof(floatStrings[2].c_str()) );
}


//-----------------------------------------------------------------------------------------------
const Vec3 operator*(float uniformScale, const Vec3& vecToScale)
{
	return vecToScale * uniformScale;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==(const Vec3& compare) const
{
	if (x == compare.x && y == compare.y && z == compare.z)
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=(const Vec3& compare) const
{
	if (x == compare.x && y == compare.y && z == compare.z)
	{
		return false;
	}

	return true;
}


Vec3 Vec3::GetNormalized() const
{
	float length = GetLength();
	if (length == 0.0f)
		return Vec3::ZERO;
	float lengthInverse = 1.0f / length;
	return Vec3(x * lengthInverse, y * lengthInverse, z * lengthInverse);
}


Vec3 Vec3::GetXYVec3() const
{
	return Vec3(x, y, 0.0f);
}


Vec3 Vec3::GetReflected(Vec3 const& impactSurfaceNormal) const
{
	float impactSurfaceNormalComponentLength = DotProduct3D(*this, impactSurfaceNormal);
	Vec3 impactSurfaceNormalComponent = impactSurfaceNormalComponentLength * impactSurfaceNormal;
	Vec3 impactSurfaceTangetComponent = *this - impactSurfaceNormalComponent;
	Vec3 reflectedVector = impactSurfaceTangetComponent + (-impactSurfaceNormalComponent);
	return reflectedVector;
}


float Vec3::GetLengthAndNormalise()
{
	float length = GetLength();

	float lengthInverse = 1.0f / length;
	x *= lengthInverse;
	y *= lengthInverse;
	z *= lengthInverse;

	return length;
}


void Vec3::Normalize()
{
	float length = GetLength();
	float lengthInverse = 1.0f / length;

	x *= lengthInverse;
	y *= lengthInverse;
	z *= lengthInverse;
}
