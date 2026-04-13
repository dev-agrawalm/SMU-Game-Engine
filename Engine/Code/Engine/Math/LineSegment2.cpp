#include "Engine/Math/LineSegment2.hpp"

LineSegment2::LineSegment2()
{

}


void LineSegment2::StretchFromCenter(float stretchByLength)
{
	Vec2 lineDirection = GetDirection();
	Vec2 lineCenter = GetCenter();
	float stretchingDisplacement = stretchByLength * 0.5f;

	m_start = lineCenter - lineDirection * stretchingDisplacement;
	m_end = lineCenter + lineDirection * stretchingDisplacement;
}


LineSegment2::~LineSegment2()
{

}


LineSegment2::LineSegment2(LineSegment2 const& copyFrom) :
	m_start(copyFrom.m_start),
	m_end(copyFrom.m_end)
{

}


LineSegment2::LineSegment2(Vec2 const& start, Vec2 const& end) :
	m_start(start),
	m_end(end)
{

}


LineSegment2::LineSegment2(float startX, float startY, float endX, float endY) :
	m_start(Vec2(startX, startY)),
	m_end(Vec2(endX, endY))
{

}


Vec2 LineSegment2::GetLineVector() const
{
	return (m_end - m_start);
}


float LineSegment2::GetDistance() const
{
	return (m_end - m_start).GetLength();
}


Vec2 LineSegment2::GetDirection() const
{
	return (m_end - m_start).GetNormalized();
}


Vec2 LineSegment2::GetCenter() const
{
	return m_start + (m_end - m_start) * 0.5f;
}


float LineSegment2::GetOrientationDegrees() const
{
	return (m_end - m_start).GetOrientationDegrees();
}


LineSegment2 LineSegment2::GetStretchedFromCenter(float stretchByLength) const
{
	Vec2 lineDirection			 = GetDirection();
	Vec2 lineCenter				 = GetCenter();
	float stretchingDisplacement = stretchByLength * 0.5f;
	
	Vec2 start  = lineCenter - lineDirection * stretchingDisplacement;
	Vec2 end	= lineCenter + lineDirection * stretchingDisplacement;

	return LineSegment2(start, end);
}


bool LineSegment2::operator==(LineSegment2 const& toCompare) const
{
	if (m_start == toCompare.m_start &&
		m_end == toCompare.m_end)
	{
		return true;
	}

	return false;
}


void LineSegment2::operator=(LineSegment2 const& copyFrom)
{
	m_start = copyFrom.m_start;
	m_end	= copyFrom.m_end;
}
