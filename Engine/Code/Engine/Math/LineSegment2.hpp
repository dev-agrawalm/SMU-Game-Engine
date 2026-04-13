#pragma once
#include "Engine/Math/Vec2.hpp"

struct LineSegment2
{
public:
	Vec2 m_start;
	Vec2 m_end;

public:
	LineSegment2();
	~LineSegment2();
	LineSegment2(LineSegment2 const& copyFrom);
	explicit LineSegment2(Vec2 const& start, Vec2 const& end);
	explicit LineSegment2(float startX, float startY, float endX, float endY);

	//mutators
	void StretchFromCenter(float stretchByLength);

	//getters
	Vec2		 GetLineVector()								const;
	Vec2		 GetDirection()									const;
	Vec2		 GetCenter()									const;
	float		 GetDistance()									const;
	float		 GetOrientationDegrees()						const;
	LineSegment2 GetStretchedFromCenter(float stretchByLength)	const;
	
	//operators
	void operator=(LineSegment2 const& copyFrom);
	bool operator==(LineSegment2 const& toCompare) const;
};