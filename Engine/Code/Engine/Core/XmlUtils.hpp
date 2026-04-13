#pragma once
#include "Engine/Core/StringUtils.hpp"
#include<string>
#include "ThirdParty/tinyxml2/tinyxml2.h"

struct IntVec2;
struct Vec2;
struct Rgba8;
struct Vec3;
struct FloatRange;

typedef tinyxml2::XMLElement	XmlElement;
typedef tinyxml2::XMLAttribute	XmlAttribute;
typedef tinyxml2::XMLDocument	XmlDocument;
typedef tinyxml2::XMLError		XmlError;

int			ParseXmlAttribute(XmlElement const& element, char const* attributeName, int					defaultValue);
uint32_t	ParseXmlAttribute(XmlElement const& element, char const* attributeName, uint32_t			defaultValue);
char		ParseXmlAttribute(XmlElement const& element, char const* attributeName, char				defaultValue);
bool		ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool				defaultValue);
float		ParseXmlAttribute(XmlElement const& element, char const* attributeName, float				defaultValue);
Vec2		ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const&			defaultValue);
Vec3		ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const&			defaultValue);
Rgba8		ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const&		defaultValue);
IntVec2		ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const&		defaultValue);
std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const&	defaultValue);
std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const*			defaultValue);
FloatRange	ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange const&	defaultValue);
Strings		ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValue, char stringDelimiter = ',');
