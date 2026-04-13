#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Vec4::Vec4()
{

}


Vec4::Vec4(Vec4 const& copyFrom)
	: x(copyFrom.x), y(copyFrom.y)
	, z(copyFrom.z), w(copyFrom.w)
{

}


Vec4::Vec4(float initalX, float initalY, float initalZ, float initalW)
	: x(initalX), y(initalY)
	, z(initalZ), w(initalW)
{

}


void Vec4::SetFromText(std::string const& valueText)
{
	Strings floatStrings = SplitStringOnDelimiter(valueText, ',');
	GUARANTEE_RECOVERABLE(floatStrings.size() == 4, "Text for Vec4 has more than 3 comma which is against the acceptable format");

	x = static_cast<float>(atof(floatStrings[0].c_str()));
	y = static_cast<float>(atof(floatStrings[1].c_str()));
	z = static_cast<float>(atof(floatStrings[2].c_str()));
	w = static_cast<float>(atof(floatStrings[3].c_str()));
}


void Vec4::operator*=(float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
}


Vec4 Vec4::operator-(Vec4 const& subtract)
{
	return Vec4(x - subtract.x, y - subtract.y, z - subtract.z, w - subtract.w);
}


Vec4::~Vec4()
{

}

