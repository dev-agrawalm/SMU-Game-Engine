#include"Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include<cmath>


const IntVec2 IntVec2::ZERO = IntVec2(0, 0);


IntVec2::IntVec2()
{

}


IntVec2::~IntVec2()
{

}


IntVec2::IntVec2(IntVec2 const& copyVec)
{
	x = copyVec.x;
	y = copyVec.y;
}


IntVec2::IntVec2(int fromX, int fromY)
{
	x = fromX;
	y = fromY;
}


IntVec2::IntVec2(Vec2 const& convertVec)
{
	x = RoundToNearestInt(convertVec.x);
	y = RoundToNearestInt(convertVec.y);
}


float IntVec2::GetLength() const
{
	float fX = static_cast<float>(x);
	float fY = static_cast<float>(y);
	float length = SqrtFloat((fX * fX) + (fY * fY));
	return length;
}


int IntVec2::GetLengthSquared() const
{
	return ((x * x) + (y * y));
}


int IntVec2::GetTaxiCabLength() const
{
	int absoluteX = abs(x);
	int absoluteY = abs(y);

	int taxicabLenth = absoluteX + absoluteY;
	return taxicabLenth;
}


float IntVec2::GetOrientationDegrees() const
{
	float fX = static_cast<float>(x);
	float fY = static_cast<float>(y);

	Vec2 tempVec = Vec2(fX, fY);
	return tempVec.GetOrientationDegrees();
}


float IntVec2::GetOrientationRadians() const
{
	float fX = static_cast<float>(x);
	float fY = static_cast<float>(y);

	Vec2 tempVec = Vec2(fX, fY);
	return tempVec.GetOrientationRadians();
}


IntVec2 IntVec2::GetRotated90Degrees() const
{
	float fX = static_cast<float>(x);
	float fY = static_cast<float>(y);

	Vec2 tempVec = Vec2(fX, fY);
	tempVec.Rotate90Degrees();
	return IntVec2(tempVec);
}


IntVec2 IntVec2::GetRotatedMinus90Degrees() const
{
	float fX = static_cast<float>(x);
	float fY = static_cast<float>(y);

	Vec2 tempVec = Vec2(fX, fY);
	tempVec.RotateMinus90Degrees();
	return IntVec2(tempVec);
}


Vec2 IntVec2::GetVec2() const
{
	float vec2X = (float) x;
	float vec2Y = (float) y;
	return Vec2(vec2X, vec2Y);
}


void IntVec2::SetFromText(std::string text)
{
	Strings intStrings = SplitStringOnDelimiter(text, ',');
	GUARANTEE_RECOVERABLE(intStrings.size() == 2, "Text for Vec2 has more than 1 comma which is against the acceptable format");

// 	floatStrings[0] = RemoveWhitespacesFromString(floatStrings[0]);
// 	floatStrings[1] = RemoveWhitespacesFromString(floatStrings[1]);

	x = atoi(intStrings[0].c_str());
	y = atoi(intStrings[1].c_str());
}


void IntVec2::Rotate90Degrees()
{
	float fX = static_cast<float>(x);
	float fY = static_cast<float>(y);

	Vec2 tempVec = Vec2(fX, fY);
	tempVec.Rotate90Degrees();

	x = RoundToNearestInt(tempVec.x);
	y = RoundToNearestInt(tempVec.y);
}


void IntVec2::RotateMinus90Degrees()
{
	float fX = static_cast<float>(x);
	float fY = static_cast<float>(y);

	Vec2 tempVec = Vec2(fX, fY);
	tempVec.RotateMinus90Degrees();

	x = RoundToNearestInt(tempVec.x);
	y = RoundToNearestInt(tempVec.y);
}


void IntVec2::operator=(IntVec2 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}


void IntVec2::operator-=(IntVec2 const& vecToSub)
{
	x -= vecToSub.x;
	y -= vecToSub.y;
}


void IntVec2::operator+=(IntVec2 const& vecToAdd)
{
	x = x + vecToAdd.x;
	y = y + vecToAdd.y;
}


IntVec2 IntVec2::operator+(IntVec2 const& vecToAdd)
{
	return IntVec2(x + vecToAdd.x, y + vecToAdd.y);
}


IntVec2 IntVec2::operator-(IntVec2 const& vecToSub)
{
	return IntVec2(x - vecToSub.x, y - vecToSub.y);
}


bool IntVec2::operator!=(IntVec2 const& vecToCompare)
{
	return !(*this == vecToCompare);
}


bool IntVec2::operator==(IntVec2 const& vecToCompare)
{
	return (x == vecToCompare.x) && (y == vecToCompare.y);
}


