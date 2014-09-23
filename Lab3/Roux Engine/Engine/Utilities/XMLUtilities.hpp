#ifndef XML_UTILITIES
#define XML_UTILITIES

//-----------------------------------------------------------------------------------------------
#include <string>
#include <ctype.h>
#include <set>
#include <algorithm>


#include "Engine/Primitives/Vector2.hpp"
#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Primitives/Color.hpp"
#include "Engine/Utilities/pugixml.hpp"
#include "Engine/Utilities/CommonUtilities.hpp"
#include "Engine/Utilities/NamedProperties.hpp"


//-----------------------------------------------------------------------------------------------
struct ErrorMessagePackage
{
	ErrorMessagePackage();
	ErrorMessagePackage(ErrorType error, const std::string& incorrectString, const std::string& expectedString)
						: m_error(error), m_incorrectString(incorrectString), m_expectedValue(expectedString)
	{

	}
	ErrorType m_error;
	std::string m_incorrectString;
	std::string m_expectedValue;
};



//-----------------------------------------------------------------------------------------------
class XMLDocumentParser
{
public:

	static XMLDocumentParser& GetInstance()
	{
		static XMLDocumentParser instance; 

		return instance;
	}
	
	inline std::string		GetFileName() const;
	inline std::string		GetStringXMLAttribute( pugi::xml_node node, const std::string& attributeName, const std::string& defaultValueIfNotFound );
	inline static void		BuildFlagsFromString( const std::string& flagsAsString, NamedProperties& propertiesOut );
	inline static void		BuildFlagsFromString( const std::string& flagsAsString, std::vector<std::string>& flagNamesOut );
	inline static bool		GetTruthValueFromFlag( std::string& actualStringNameOut);


	bool					GetBoolXMLAttribute( pugi::xml_node node, const std::string& attributeName, bool defaultValueIfNotFound );
	int						GetIntXMLAttribute( pugi::xml_node node, const std::string& attributeName, int defaultValueIfNotFound );
	float					GetFloatXMLAttribute( pugi::xml_node node, const std::string& attributeName, float defaultValueIfNotFound );
	Vector2f				GetVector2XMLAttribute( pugi::xml_node node, const std::string& attributeName, const Vector2f& defaultValueIfNotFound );
	Vector2i				GetIntVector2XMLAttribute( pugi::xml_node node, const std::string& attributeName, const Vector2i& defaultValueIfNotFound );
	Vector3f				GetVector3XMLAttribute( pugi::xml_node node, const std::string& attributeName, const Vector3f& defaultValueIfNotFound );
	Vector3i				GetIntVector3XMLAttribute( pugi::xml_node node, const std::string& attributeName, const Vector3i& defaultValueIfNotFound );
	Color					GetRgbaXMLAttribute( pugi::xml_node node, const std::string& attributeName, const Color& defaultValueIfNotFound );
	FloatRange				GetFloatRangeXMLAttribute( pugi::xml_node node, const std::string& attributeName, const FloatRange& defaultValueIfNotFound );
	IntRange				GetIntRangeXMLAttribute( pugi::xml_node node, const std::string& attributeName, const IntRange& defaultValueIfNotFound );
	
	void					LoadDocument( const std::string& fileName );
	void					ValidateXMLAttributes( pugi::xml_node node, const std::string& requiredAttributes, const std::string& optionalAttributes = "" );
	void					ValidateXMLChildElements( pugi::xml_node node, const std::string& requiredChildElements, const std::string& optionalChildElements = "" );

	pugi::xml_document m_doc;

private:

	XMLDocumentParser() {}; 

	XMLDocumentParser(XMLDocumentParser const&);              
	void operator=(XMLDocumentParser const&); 

	void					OutputErrorPackagesAndDie( const std::vector<ErrorMessagePackage>& packages, const std::string& parentNodeName );
	void					OutputErrorSetsAndDie( const std::set<ErrorType>& errors, const std::string& badValueAsString, const std::string& attributeName, const std::string& parentNodeName );
	std::string				GetAppropriateErrorMessage(ErrorType error, const std::string& badValueAsString, const std::string& optionalExpectedValue, const std::string& parentNodeName );

	std::string m_filename;
};



//-----------------------------------------------------------------------------------------------
inline std::string XMLDocumentParser::GetFileName() const
{
	return m_filename;
}

//-----------------------------------------------------------------------------------------------
inline std::string XMLDocumentParser::GetStringXMLAttribute( pugi::xml_node node, const std::string& attributeName, const std::string& defaultValueIfNotFound )
{
	pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	std::string result;

	if(attribute.empty())
		return defaultValueIfNotFound;
	else
	{
		result = attribute.as_string();
		return result;
	}
}

//-----------------------------------------------------------------------------------------------
void XMLDocumentParser::BuildFlagsFromString( const std::string& flagsAsString, NamedProperties& propertiesOut )
{
	std::vector<std::string> flagNames = SplitStringBySingleCharacterDelimiters( flagsAsString, "," );

	std::string actualString;
	bool truthValue;

	for( auto iter = flagNames.begin(); iter != flagNames.end(); ++iter )
	{
		actualString = *iter;

		truthValue = GetTruthValueFromFlag( actualString );
		propertiesOut.Set( actualString, truthValue );
	}
}

//-----------------------------------------------------------------------------------------------
void XMLDocumentParser::BuildFlagsFromString( const std::string& flagsAsString, std::vector<std::string>& flagNamesOut )
{
	flagNamesOut = SplitStringBySingleCharacterDelimiters( flagsAsString, "," );
}

//-----------------------------------------------------------------------------------------------
bool XMLDocumentParser::GetTruthValueFromFlag( std::string& actualStringNameOut)
{
	bool result = true;

	if( actualStringNameOut.size() > 0 && actualStringNameOut[0] == '!' )
	{
		std::string newStringValue;

		newStringValue.append( ++actualStringNameOut.begin(), actualStringNameOut.end() );
		actualStringNameOut = newStringValue;
		result = false;
	}

	return result;
}

#endif