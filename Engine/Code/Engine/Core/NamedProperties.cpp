#include "Engine/Core/NamedProperties.hpp"

BaseTypedProperty::~BaseTypedProperty()
{

}


void NamedProperties::AddProperty(std::string const& propertyName, const char* text)
{
	std::string textString(text);
	AddProperty(propertyName, textString);
}


bool NamedProperties::GetProperty(std::string const& propertyName, std::string& out_value, const char* defaultValue) const
{
	std::string defaultString(defaultValue);
	return GetProperty(propertyName, out_value, defaultString);
}


std::string NamedProperties::GetProperty(std::string const& propertyName, const char* defaultValue) const
{
	std::string defaultString(defaultValue);
	return GetProperty(propertyName, defaultString);
}

