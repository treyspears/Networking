#ifndef NAMED_PROPERTIES
#define NAMED_PROPERTIES

#include <map>
#include <string>

//-----------------------------------------------------------------------------------------------
class NamedPropertiesBase
{

public:
	virtual ~NamedPropertiesBase() {}

};

//-----------------------------------------------------------------------------------------------
template< typename T_PropertyType >
class TypedNamedPropteries : public NamedPropertiesBase
{

public:
	TypedNamedPropteries( const T_PropertyType& data )
		: m_data(data) {}

	T_PropertyType m_data;
};

//-----------------------------------------------------------------------------------------------
class NamedProperties
{

public:

	enum GetPropertyResult
	{
		GET_PROPERTY_SUCCESS,
		GET_PROPERTY_NOT_FOUND,
		GET_PROPERTY_FOUND_BUT_WRONG_TYPE,
		GET_NO_PROPERTIES
	};

	template< typename T_PropertyType >
	void Set(const std::string& keyName, const T_PropertyType& typedValue);
	
	void Set(const std::string& keyName, const char* valueString);

	template< typename T_PropertyType >
	GetPropertyResult Get(const std::string& keyName, T_PropertyType& outValue) const;

	static NamedProperties EmptyProperties();

	std::map<std::string, NamedPropertiesBase*> m_properties;
};

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
template< typename T_PropertyType >
inline void NamedProperties::Set(const std::string& keyName, const T_PropertyType& typedValue)
{
	TypedNamedPropteries< T_PropertyType >* namedPropertyWithType = new TypedNamedPropteries< T_PropertyType >(typedValue);

	m_properties[keyName] = namedPropertyWithType;
}

//-----------------------------------------------------------------------------------------------
inline void NamedProperties::Set(const std::string& keyName, const char* valueString)
{
	std::string asString(valueString);
	Set(keyName, asString);
}

//-----------------------------------------------------------------------------------------------
template< typename T_PropertyType >
inline NamedProperties::GetPropertyResult NamedProperties::Get(const std::string& keyName, T_PropertyType& outValue) const
{
	GetPropertyResult result;

	if(m_properties.size() == 0)
	{
		result = GET_NO_PROPERTIES;
		return result;
	}

	auto foundIter = m_properties.find(keyName);

	if(foundIter == m_properties.end())
	{
		result = GET_PROPERTY_NOT_FOUND;
	}
	else
	{
		NamedPropertiesBase* valueAsNamedPropertyBase = foundIter->second;
		TypedNamedPropteries<T_PropertyType>* valueAsTypedProperty = dynamic_cast< TypedNamedPropteries<T_PropertyType>* >(valueAsNamedPropertyBase);
		
		if(valueAsTypedProperty == nullptr)
		{
			result = GET_PROPERTY_FOUND_BUT_WRONG_TYPE;	
		}
		else
		{
			result = GET_PROPERTY_SUCCESS;
			outValue = valueAsTypedProperty->m_data;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
inline NamedProperties NamedProperties::EmptyProperties()
{
	NamedProperties emptyProperties;

	return emptyProperties;
}

#endif