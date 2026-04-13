#pragma once
#include <string>

struct Vec4
{
public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;

public:
	Vec4();
	~Vec4();
	Vec4(Vec4 const& copyFrom);
	explicit Vec4(float initalX, float initalY, float initalZ, float initalW);

	void SetFromText(std::string const& valueText);

	void operator*=(float scale);
	Vec4 operator-(Vec4 const& subtract);
};
