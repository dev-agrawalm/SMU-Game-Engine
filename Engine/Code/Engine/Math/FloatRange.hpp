#pragma once
#include <string>

struct FloatRange
{
public:
	float m_min = 0.0f;
	float m_max = 0.0f;

public:
	static const FloatRange ZERO;
	static const FloatRange ONE;
	static const FloatRange ZERO_TO_ONE;

public:
	FloatRange();
	~FloatRange();
	FloatRange(FloatRange const& copyFrom);
	explicit FloatRange(float a, float b);

	void SetFromText(std::string text);

	bool IsOnRange(float referenceFloat) const;
	bool IsOverlappingWith(FloatRange const& range) const;

	void operator=(FloatRange const& copyFrom);
	bool operator==(FloatRange const& compare) const;
	bool operator!=(FloatRange const& compare) const;
};