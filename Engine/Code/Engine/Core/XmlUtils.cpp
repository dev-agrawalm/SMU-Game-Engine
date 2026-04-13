#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/FloatRange.hpp"

int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	return element.IntAttribute(attributeName, defaultValue);
}


char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	return *(element.Attribute(attributeName, &defaultValue));
}


bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	return element.BoolAttribute(attributeName, defaultValue);
}


float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	return element.FloatAttribute(attributeName, defaultValue);
}


Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	Vec2 outputVec2 = defaultValue;

	tinyxml2::XMLAttribute const* attribute = element.FindAttribute(attributeName);
	if (attribute)
		outputVec2.SetFromText(attribute->Value());

	return outputVec2;
}


Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue)
{
	Vec3 outputVec3 = defaultValue;

	tinyxml2::XMLAttribute const* attribute = element.FindAttribute(attributeName);
	if (attribute)
		outputVec3.SetFromText(attribute->Value());

	return outputVec3;
}


Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	Rgba8 outputRgba8 = defaultValue;

	tinyxml2::XMLAttribute const* attribute = element.FindAttribute(attributeName);
	if (attribute)
		outputRgba8.SetFromText(attribute->Value());

	return outputRgba8;
}


IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	IntVec2 outputIntVec2 = defaultValue;

	tinyxml2::XMLAttribute const* attribute = element.FindAttribute(attributeName);
	if (attribute)
		outputIntVec2.SetFromText(attribute->Value());

	return outputIntVec2;
}


Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValue, char stringDelimiter)
{
	Strings outputStrings = defaultValue;

	tinyxml2::XMLAttribute const* attribute = element.FindAttribute(attributeName);
	if (attribute)
		outputStrings = SplitStringOnDelimiter(attribute->Value(), stringDelimiter);

	return outputStrings;
}


std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	std::string outputString = defaultValue;

	tinyxml2::XMLAttribute const* attribute = element.FindAttribute(attributeName);
	if(attribute)
		outputString = attribute->Value();

	return outputString;
}


std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const* defaultValue)
{
	std::string outputString;
	tinyxml2::XMLAttribute const* attribute = element.FindAttribute(attributeName);
	if (attribute)
	{
		outputString = attribute->Value();
		return outputString;
	}
	else
		return defaultValue;
}


FloatRange ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange const& defaultValue)
{
	FloatRange outputFloatRange = defaultValue;

	tinyxml2::XMLAttribute const* attribute = element.FindAttribute(attributeName);
	if (attribute)
		outputFloatRange.SetFromText(attribute->Value());

	return outputFloatRange;
}


uint32_t ParseXmlAttribute(XmlElement const& element, char const* attributeName, uint32_t defaultValue)
{
	return element.IntAttribute(attributeName, defaultValue);
}

