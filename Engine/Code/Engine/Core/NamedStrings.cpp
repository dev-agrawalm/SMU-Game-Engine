#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/AABB2.hpp"

NamedStrings::NamedStrings()
{

}


NamedStrings::~NamedStrings()
{

}


void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	XmlAttribute const* attribute = element.FirstAttribute();
	while (attribute)
	{
		const char* attributeName = attribute->Name();
		std::string attributeValue = ParseXmlAttribute(element, attributeName, "");
		SetValue(attributeName, attributeValue);
		attribute = attribute->Next();
	}
}


void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}


std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	std::string value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value = iterator->second;
	}

	return value;
}


bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	bool value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		tinyxml2::XMLUtil::ToBool(iterator->second.c_str(), &value);
	}

	return value;
}


int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	int value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value = atoi(iterator->second.c_str());
	}

	return value;
}


float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	float value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value = static_cast<float>(atof(iterator->second.c_str()));
	}

	return value;
}


std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	std::string value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value = iterator->second;
	}

	return value;
}


Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	Rgba8 value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value.SetFromText(iterator->second);
	}

	return value;
}


Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	Vec2 value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value.SetFromText(iterator->second);
	}

	return value;
}


IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	IntVec2 value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value.SetFromText(iterator->second);
	}

	return value;
}


unsigned char NamedStrings::GetValue(std::string const& keyName, unsigned char defaultValue) const
{
	unsigned char value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value = iterator->second[0];
	}

	return value;
}


FloatRange NamedStrings::GetValue(std::string const& keyName, FloatRange const& defaultValue) const
{
	FloatRange value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value.SetFromText(iterator->second);
	}
	return value;
}


IntRange NamedStrings::GetValue(std::string const& keyName, IntRange const& defaultValue) const
{
	IntRange value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value.SetFromText(iterator->second);
	}
	return value;
}


Vec3 NamedStrings::GetValue(std::string const& keyName, Vec3 const& defaultValue) const
{
	Vec3 value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value.SetFromText(iterator->second);
	}

	return value;
}


Vec4 NamedStrings::GetValue(std::string const& keyName, Vec4 const& defaultValue) const
{
	Vec4 value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value.SetFromText(iterator->second);
	}

	return value;
}


AABB2 NamedStrings::GetValue(std::string const& keyName, AABB2 const& defaultValue) const
{
	AABB2 value = defaultValue;
	auto iterator = m_keyValuePairs.find(keyName);
	if (iterator != m_keyValuePairs.end())
	{
		value.SetFromText(iterator->second);
	}

	return value;
}


