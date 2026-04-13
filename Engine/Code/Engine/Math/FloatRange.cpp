#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const FloatRange FloatRange::ZERO			= FloatRange(0.0f, 0.0f);
const FloatRange FloatRange::ONE			= FloatRange(1.0f, 1.0f);
const FloatRange FloatRange::ZERO_TO_ONE	= FloatRange(0.0f, 1.0f);

FloatRange::FloatRange()
{
}


FloatRange::FloatRange(FloatRange const& copyFrom)
	: m_min(copyFrom.m_min)
	, m_max(copyFrom.m_max)
{
}


FloatRange::FloatRange(float a, float b)
{
	if (a < b)
	{
		m_min = a;
		m_max = b;
	}
	else
	{
		m_min = b;
		m_max = a;
	}
}


void FloatRange::SetFromText(std::string text)
{
	Strings floatValues = SplitStringOnDelimiter(text, '~');
	GUARANTEE_OR_DIE(floatValues.size() == 2, "Text for float range is in the wrong format. Correct format: minFloatValue~MaxFloatValue");

	m_min = (float) atof(floatValues[0].c_str());
	m_max = (float) atof(floatValues[1].c_str());
}


bool FloatRange::IsOnRange(float referenceFloat) const
{
	return (referenceFloat >= m_min) && (referenceFloat <= m_max);
}


bool FloatRange::IsOverlappingWith(FloatRange const& range) const
{
	return !((range.m_max >= m_max && range.m_min >= m_max) || (range.m_max <= m_min && range.m_min <= m_min));
}


bool FloatRange::operator!=(FloatRange const& compare) const
{
	return !(m_min == compare.m_min && m_max == compare.m_max);
}


bool FloatRange::operator==(FloatRange const& compare) const
{
	return (m_min == compare.m_min && m_max == compare.m_max);
}


void FloatRange::operator=(FloatRange const& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}


FloatRange::~FloatRange()
{
}
