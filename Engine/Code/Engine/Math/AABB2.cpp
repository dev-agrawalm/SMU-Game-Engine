#include"Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

AABB2 const AABB2::ZERO_TO_ONE = AABB2(0.0f, 0.0f, 1.0f, 1.0f);

AABB2::AABB2()
{

}


AABB2::AABB2(float minX, float minY, float maxX, float maxY)
{
	m_mins = Vec2(minX, minY);
	m_maxs = Vec2(maxX, maxY);
}


AABB2::AABB2(Vec2 const& mins, Vec2 const& maxs)
{
	m_mins = mins;
	m_maxs = maxs;
}


AABB2::AABB2(AABB2 const& copyBox)
{
	m_mins = copyBox.m_mins;
	m_maxs = copyBox.m_maxs;
}


AABB2::AABB2(Vec2 const& center, float width, float height)
{
	m_mins = center - Vec2(width, height) * 0.5f;
	m_maxs = center + Vec2(width, height) * 0.5f;
}


AABB2::~AABB2()
{

}


bool AABB2::IsPointInside(Vec2 const& point) const
{
	float minX		= m_mins.x;
	float minY		= m_mins.y;
	float maxX		= m_maxs.x;
	float maxY		= m_maxs.y;
	float pointX	= point.x;
	float pointY	= point.y;

	if ( (pointX > minX && pointX < maxX) 
		&& (pointY > minY && pointY < maxY) )
	{
		return true;
	}

	return false;
}


Vec2 const AABB2::GetCenter() const
{
	Vec2 halfDims	= (m_maxs - m_mins) * 0.5f;
	Vec2 centerPos	= m_mins + halfDims;

	return centerPos;
}


Vec2 const AABB2::GetDimensions() const
{
	return (m_maxs - m_mins);
}


Vec2 const AABB2::GetNearestPoint(Vec2 const& pointOfReference) const
{
	float nearestX = Clamp(pointOfReference.x, m_mins.x, m_maxs.x);
	float nearestY = Clamp(pointOfReference.y, m_mins.y, m_maxs.y);

	return Vec2(nearestX, nearestY);
}


Vec2 const AABB2::GetPointAtUV(Vec2 const& uvCoords) const
{
	float xFraction = uvCoords.x;
	float yFraction = uvCoords.y;

	float pointX = Lerp(m_mins.x, m_maxs.x, xFraction);
	float pointY = Lerp(m_mins.y, m_maxs.y, yFraction);

	return Vec2(pointX, pointY);
}


Vec2 const AABB2::GetUVForPoint(Vec2 const& point) const
{
	float pointX = point.x;
	float pointY = point.y;

	float u = GetFraction(pointX, m_mins.x, m_maxs.x);
	float v = GetFraction(pointY, m_mins.y, m_maxs.y);

	return Vec2(u, v);
}


void AABB2::Translate(Vec2 const& byDistance)
{
	m_mins += byDistance;
	m_maxs += byDistance;
}


void AABB2::SetCenter(Vec2 const& newCenter)
{
	Vec2 halfDims = (m_maxs - m_mins) * 0.5f;

	m_mins = newCenter - halfDims;
	m_maxs = newCenter + halfDims;
}


void AABB2::SetDimensions(Vec2 const& newDimensions)
{
	Vec2 newHalfDims = newDimensions * 0.5f;
	Vec2 center = GetCenter();

	m_mins = center - newHalfDims;
	m_maxs = center + newHalfDims;
}


void AABB2::SetDimensions(float newWidth, float newHeight)
{
	SetDimensions(Vec2(newWidth, newHeight));
}


void AABB2::SetHeight(float newHeight)
{
	Vec2 dims = GetDimensions();
	SetDimensions(dims.x, newHeight);
}


void AABB2::SetWidth(float newWidth)
{
	Vec2 dims = GetDimensions();
	SetDimensions(newWidth, dims.y);
}


void AABB2::StretchToIncludePoint(Vec2 const& pointToInclude)
{
	float pointX = pointToInclude.x;
	float pointY = pointToInclude.y;
	float minX = m_mins.x;
	float maxX = m_maxs.x;
	float minY = m_mins.y;
	float maxY = m_maxs.y;

	//stretch along x dimensions
	if (pointX > maxX)
	{
		m_maxs.x = pointX;
	}
	else if (pointX < minX)
	{
		m_mins.x = pointX;
	}
	
	//stretch along y dimension
	if (pointY > maxY)
	{
		m_maxs.y = pointY;
	}
	else if (pointY < minY)
	{
		m_mins.y = pointY;
	}
}


void AABB2::UniformScaleFromCenter(float scalingFactor)
{
	Vec2 dims = GetDimensions();
	float width = dims.x;
	float newWidth = width * scalingFactor;
	float widthDiff = newWidth - width;
	float widthDiffHalf = widthDiff * 0.5f;
	m_mins.x -= widthDiffHalf;
	m_maxs.x += widthDiffHalf;

	float height = dims.y;
	float newHeight = height * scalingFactor;
	float heightDiff = newHeight - height;
	float heightDiffHalf = heightDiff * 0.5f;
	m_mins.y -= heightDiffHalf;
	m_maxs.y += heightDiffHalf;
}


void AABB2::SetFromText(std::string valueText)
{
	Strings floatStrings = SplitStringOnDelimiter(valueText, ',');
	GUARANTEE_RECOVERABLE(floatStrings.size() == 4, "Text for AABB2 does not have 3 commas which is against the acceptable format");

	m_mins.x = static_cast<float>(atof(floatStrings[0].c_str()));
	m_mins.y = static_cast<float>(atof(floatStrings[1].c_str()));
	m_maxs.x = static_cast<float>(atof(floatStrings[2].c_str()));
	m_maxs.y = static_cast<float>(atof(floatStrings[3].c_str()));
}


void AABB2::operator=(AABB2 const& box)
{
	m_mins = box.m_mins;
	m_maxs = box.m_maxs;
}
