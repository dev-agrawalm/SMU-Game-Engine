#pragma once
#include <string>

struct IntRange
{
public:
	int m_min = 0;
	int m_max = 0;

public:
	static const IntRange ZERO;
	static const IntRange ONE;
	static const IntRange ZERO_TO_ONE;

public:
	IntRange();
	~IntRange();
	IntRange(IntRange const& copyFrom);
	explicit IntRange(int min, int max);

	void SetFromText(std::string const& valueString);

	bool IsOnRange(int referenceInt) const;
	bool IsOverlappingWith(IntRange const& range) const;

	void operator=(IntRange const& copyFrom);
	bool operator==(IntRange const& compare) const;
	bool operator!=(IntRange const& compare) const;
};