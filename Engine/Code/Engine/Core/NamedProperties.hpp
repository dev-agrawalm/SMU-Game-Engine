#pragma once
#include <map>
#include <string>

class BaseTypedProperty
{
public:
	virtual ~BaseTypedProperty();
	virtual BaseTypedProperty* Clone() const = 0;
};


template <typename T>
class TypedProperty : public BaseTypedProperty
{
public:
	virtual BaseTypedProperty* Clone() const override;

public:
	T m_value;
};


template <typename T>
BaseTypedProperty* TypedProperty<T>::Clone() const
{
	TypedProperty<T>* clonedProperty = new TypedProperty<T>();
	clonedProperty->m_value = m_value;
	return clonedProperty;
}


class NamedProperties
{
public:
	template<typename T>
	void AddProperty(std::string const& propertyName, T const& value);
	void AddProperty(std::string const& propertyName, const char* text);
	
	template<typename T>
	bool GetProperty(std::string const& propertyName, T& out_value, T const& defaultValue) const;
	bool GetProperty(std::string const& propertyName, std::string& out_value, const char* defaultValue) const;
	template<typename T>
	T GetProperty(std::string const& propertyName, T const& defaultValue) const;
	std::string GetProperty(std::string const& propertyName, const char* defaultValue) const;

private:
	std::map<std::string, BaseTypedProperty*> m_properties;
};


template<typename T>
void NamedProperties::AddProperty(std::string const& propertyName, T const& value)
{
	TypedProperty<T>* typedProperty = new TypedProperty<T>();
	typedProperty->m_value = value;
	m_properties[propertyName] = typedProperty;
}


template<typename T>
bool NamedProperties::GetProperty(std::string const& propertyName, T& out_value, T const& defaultValue) const
{
	auto mapIter = m_properties.begin();
	while (mapIter != m_properties.end())
	{
		if (_stricmp(mapIter->first.c_str(), propertyName.c_str()) == 0)
		{
			BaseTypedProperty* baseTypedProperty = mapIter->second;
			TypedProperty<T>* typedProp = dynamic_cast<TypedProperty<T>*>(baseTypedProperty);
			if (typedProp == nullptr) 
			{
				out_value = defaultValue;
				return false;
			}
			
			out_value = typedProp->m_value;
			return true;
		}
		mapIter++;
	}

	out_value = defaultValue;
	return false;
}


template<typename T>
T NamedProperties::GetProperty(std::string const& propertyName, T const& defaultValue) const
{
	auto mapIter = m_properties.begin();
	while (mapIter != m_properties.end())
	{
		if (_stricmp(mapIter->first.c_str(), propertyName.c_str()) == 0)
		{
			BaseTypedProperty* baseTypedProperty = mapIter->second;
			TypedProperty<T>* typedProp = dynamic_cast<TypedProperty<T>*>(baseTypedProperty);
			if (typedProp == nullptr)
			{
				return defaultValue;
			}

			return typedProp->m_value;
		}
		mapIter++;
	}

	return defaultValue;
}

