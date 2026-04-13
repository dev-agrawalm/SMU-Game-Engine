#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const IntRange IntRange::ZERO			= IntRange(0, 0);
const IntRange IntRange::ONE			= IntRange(1, 1);
const IntRange IntRange::ZERO_TO_ONE	= IntRange(0, 1);

IntRange::IntRange()
{
}


IntRange::IntRange(IntRange const& copyFrom)
	: m_min(copyFrom.m_min)
	, m_max(copyFrom.m_max)
{
}


IntRange::IntRange(int min, int max)
	: m_max(max)
	, m_min(min)
{
}


void IntRange::SetFromText(std::string const& valueString)
{
	Strings intValues = SplitStringOnDelimiter(valueString, '~');
	GUARANTEE_OR_DIE(intValues.size() == 2, "Text for int range is in the wrong format. Correct format: minIntValue~MaxIntValue");

	m_min = atoi(intValues[0].c_str());
	m_max = atoi(intValues[1].c_str());
}


bool IntRange::IsOnRange(int referenceInt) const
{
	return referenceInt >= m_min && referenceInt <= m_max;
}


bool IntRange::IsOverlappingWith(IntRange const& range) const
{
	return (range.m_max >= m_max && range.m_min >= m_max) || (range.m_max <= m_min && range.m_min <= m_min);
}


bool IntRange::operator!=(IntRange const& compare) const
{
	return !(m_min == compare.m_min && m_max == compare.m_max);
}


bool IntRange::operator==(IntRange const& compare) const
{
	return (m_min == compare.m_min && m_max == compare.m_max);
}


void IntRange::operator=(IntRange const& copyFrom)
{
	m_max = copyFrom.m_max;
	m_min = copyFrom.m_min;
}


IntRange::~IntRange()
{

}

