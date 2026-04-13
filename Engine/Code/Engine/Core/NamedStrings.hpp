#pragma once
#include <map>
#include <string>
#include "Engine/Core/XmlUtils.hpp"

struct IntVec2;
struct Rgba8;
struct Vec2;
struct FloatRange;
struct IntRange;
struct Vec3;
struct Vec4;
struct AABB2;

class NamedStrings
{
public:
	NamedStrings();
	~NamedStrings();
	void		PopulateFromXmlElementAttributes(XmlElement const& element);
	void		SetValue(std::string const& keyName, std::string const& newValue);
	std::string	GetValue(std::string const& keyName, std::string const& defaultValue) const;
	bool		GetValue(std::string const& keyName, bool defaultValue) const;
	int			GetValue(std::string const& keyName, int defaultValue) const;
	float		GetValue(std::string const& keyName, float defaultValue) const;
	std::string	GetValue(std::string const& keyName, char const* defaultValue) const;
	Rgba8		GetValue(std::string const& keyName, Rgba8 const& defaultValue) const;
	Vec2		GetValue(std::string const& keyName, Vec2 const& defaultValue) const;
	Vec3		GetValue(std::string const& keyName, Vec3 const& defaultValue) const;
	Vec4		GetValue(std::string const& keyName, Vec4 const& defaultValue) const;
	IntVec2		GetValue(std::string const& keyName, IntVec2 const& defaultValue) const;
	unsigned char	GetValue(std::string const& keyName, unsigned char defaultValue) const;
	FloatRange	GetValue(std::string const& keyName, FloatRange const& defaultValue) const;
	IntRange	GetValue(std::string const& keyName, IntRange const& defaultValue) const;
	AABB2		GetValue(std::string const& keyName, AABB2 const& defaultValue) const;

private:
	std::map<std::string, std::string> m_keyValuePairs;
};