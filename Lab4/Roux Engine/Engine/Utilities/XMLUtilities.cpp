#include "XMLUtilities.hpp"

#include "Engine/Rendering/OpenGLRenderer.hpp"

//-----------------------------------------------------------------------------------------------
void XMLDocumentParser::LoadDocument( const std::string& fileName )
{
	m_filename = fileName;

	pugi::xml_parse_result result;
	m_doc.reset();
	result = m_doc.load_file(m_filename.c_str());

	if(result.status != pugi::status_ok)
	{
		std::string errorMessage = "\"" + m_filename + "\" not found.";
		OpenGLRenderer::RenderErrorWindow(errorMessage,"Errors:");
		std::exit(0);
	}
}

//-----------------------------------------------------------------------------------------------
bool XMLDocumentParser::GetBoolXMLAttribute( pugi::xml_node node, const std::string& attributeName, bool defaultValueIfNotFound )
{
	pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	bool result = defaultValueIfNotFound;

	if(attribute.empty())
		return result;
	else
	{
		std::set<ErrorType> errors;

		std::string actualValueAsString = attribute.as_string();
		std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");

		if(splitStrings.size() > 1)
		{
			errors.insert(TOO_MANY_VALUES);
			OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
		}
		else
		{
			actualValueAsString = splitStrings.back();
			errors = ValidateIsBool(actualValueAsString);

			if(errors.empty())
				result = static_cast<int>(strtol(actualValueAsString.c_str(), 0, 10)) > 0;
			else
				OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());	
		}
	}
	return result;
}

//-----------------------------------------------------------------------------------------------
 int XMLDocumentParser::GetIntXMLAttribute( pugi::xml_node node, const std::string& attributeName, int defaultValueIfNotFound )
{
	pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	int result = defaultValueIfNotFound;

	if(attribute.empty())
		return result;
	else
	{
		std::set<ErrorType> errors;

		std::string actualValueAsString = attribute.as_string();
		std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");

		if(splitStrings.size() > 1)
		{
			errors.insert(TOO_MANY_VALUES);
			OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
		}
		else
		{
			actualValueAsString = splitStrings.back();
			errors = ValidateIsInt(actualValueAsString);

			if(errors.empty())
				result = static_cast<int>(strtol(actualValueAsString.c_str(), 0, 10));
			else
				OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());	
		}
	}
	return result;
}

//-----------------------------------------------------------------------------------------------
 float XMLDocumentParser::GetFloatXMLAttribute( pugi::xml_node node, const std::string& attributeName, float defaultValueIfNotFound )
{
	pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	float result = defaultValueIfNotFound;

	if(attribute.empty())
		return result;
	else
	{
		std::set<ErrorType> errors;

		std::string actualValueAsString = attribute.as_string();
		std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");

		if(splitStrings.size() > 1)
		{
			errors.insert(TOO_MANY_VALUES);
			OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
		}
		else
		{
			actualValueAsString = splitStrings.back();
			errors = ValidateIsFloat(actualValueAsString);

			if(errors.empty())
				result = static_cast<float>(strtod(actualValueAsString.c_str(),0));
			else
				OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
		}
	}
	return result;
}

//-----------------------------------------------------------------------------------------------
 Vector2f XMLDocumentParser::GetVector2XMLAttribute( pugi::xml_node node, const std::string& attributeName, const Vector2f& defaultValueIfNotFound )
{
	pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	Vector2f result = defaultValueIfNotFound;

	if(attribute.empty())
		return result;
	else
	{
		std::string actualValueAsString = attribute.as_string();
		std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");

		std::set<ErrorType> errors;	

		if(splitStrings.size() > 5)
		{
			errors.insert(TOO_MANY_VALUES);
			OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
		}
		else
		{
			errors = ValidateIsVector2(splitStrings);
			splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t (,)");

			if(splitStrings.size() != 2)
			{
				errors.insert(TOO_MANY_VALUES);
				OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
			}
			else
			{
				std::set<ErrorType> additionalErrors;

				additionalErrors = ValidateIsFloat(splitStrings[0]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				additionalErrors = ValidateIsFloat(splitStrings[1]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				if(errors.empty())
				{
					result.x = static_cast<float>(strtod(splitStrings[0].c_str(),0));
					result.y = static_cast<float>(strtod(splitStrings[1].c_str(),0));
				}
				else
				{
					OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
 Vector2i XMLDocumentParser::GetIntVector2XMLAttribute( pugi::xml_node node, const std::string& attributeName, const Vector2i& defaultValueIfNotFound )
{
	pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	Vector2i result = defaultValueIfNotFound;

	if(attribute.empty())
		return result;
	else
	{
		std::string actualValueAsString = attribute.as_string();
		std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");

		std::set<ErrorType> errors;	

		if(splitStrings.size() > 5)
		{
			errors.insert(TOO_MANY_VALUES);
			OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
		}
		else
		{
			errors = ValidateIsVector2(splitStrings);
			splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t (,)");

			if(splitStrings.size() != 2)
			{
				errors.insert(TOO_MANY_VALUES);
				OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
			}
			else
			{
				std::set<ErrorType> additionalErrors;

				additionalErrors = ValidateIsInt(splitStrings[0]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				additionalErrors = ValidateIsInt(splitStrings[1]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				if(errors.empty())
				{
					result.x = static_cast<int>(strtol(splitStrings[0].c_str(), 0, 10));
					result.y = static_cast<int>(strtol(splitStrings[1].c_str(), 0, 10));
				}
				else
				{
					OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
				}	
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
 Vector3f XMLDocumentParser::GetVector3XMLAttribute( pugi::xml_node node, const std::string& attributeName, const Vector3f& defaultValueIfNotFound )
{
	pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	Vector3f result = Vector3f::Zero();

	if(attribute.empty())
		return defaultValueIfNotFound;
	else
	{
		std::string actualValueAsString = attribute.as_string();
		std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");

		std::set<ErrorType> errors;	

		if(splitStrings.size() > 7)
		{
			errors.insert(TOO_MANY_VALUES);
			OutputErrorSetsAndDie(errors, actualValueAsString, "", node.name());
		}
		else
		{
			errors = ValidateIsVector3(splitStrings);
			splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t (,)");

			if(splitStrings.size() != 3)
			{
				errors.insert(TOO_MANY_VALUES);
				OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
			}
			else
			{
				std::set<ErrorType> additionalErrors;

				additionalErrors = ValidateIsFloat(splitStrings[0]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				additionalErrors = ValidateIsFloat(splitStrings[1]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				additionalErrors = ValidateIsFloat(splitStrings[2]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				if(errors.empty())
				{
					result.x = static_cast<float>(strtod(splitStrings[0].c_str(),0));
					result.y = static_cast<float>(strtod(splitStrings[1].c_str(),0));
					result.z = static_cast<float>(strtod(splitStrings[2].c_str(),0));
				}
				else
				{
					OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
				}	
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
 Vector3i XMLDocumentParser::GetIntVector3XMLAttribute( pugi::xml_node node, const std::string& attributeName, const Vector3i& defaultValueIfNotFound )
{
	pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	Vector3i result = Vector3i::Zero();

	if(attribute.empty())
		return defaultValueIfNotFound;
	else
	{
		std::string actualValueAsString = attribute.as_string();
		std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");

		std::set<ErrorType> errors;	

		if(splitStrings.size() > 7)
		{
			errors.insert(TOO_MANY_VALUES);
			OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
		}
		else
		{
			errors = ValidateIsVector3(splitStrings);
			splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t (,)");

			if(splitStrings.size() != 3)
			{
				errors.insert(TOO_MANY_VALUES);
				OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
			}
			else
			{
				std::set<ErrorType> additionalErrors;

				additionalErrors = ValidateIsInt(splitStrings[0]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				additionalErrors = ValidateIsInt(splitStrings[1]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				additionalErrors = ValidateIsInt(splitStrings[2]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				if(errors.empty())
				{
					result.x = static_cast<int>(strtol(splitStrings[0].c_str(), 0, 10));
					result.y = static_cast<int>(strtol(splitStrings[1].c_str(), 0, 10));
					result.z = static_cast<int>(strtol(splitStrings[2].c_str(), 0, 10));
				}
				else
				{
					OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------
 Color XMLDocumentParser::GetRgbaXMLAttribute( pugi::xml_node node, const std::string& attributeName, const Color& defaultValueIfNotFound )
{
	pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	Color result = defaultValueIfNotFound;
	bool hasFourValues = false;

	if(attribute.empty())
		return result;
	else
	{
		std::string actualValueAsString = attribute.as_string();
		std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");

		std::set<ErrorType> errors;	
		errors = ValidateIsVector3(splitStrings);

		if(errors.find(TOO_MANY_COMMAS) != errors.end())
		{
			errors = ValidateIsVector4(splitStrings);
			hasFourValues = true;
		}

		if((!hasFourValues && splitStrings.size() > 7) || (hasFourValues && splitStrings.size() > 9))
		{
			errors.insert(TOO_MANY_VALUES);
			OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
		}
		else
		{
			splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t (,)");

			if((!hasFourValues && splitStrings.size() != 3) || (hasFourValues && splitStrings.size() != 4))
			{
				errors.insert(TOO_MANY_VALUES);
				OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
			}
			else
			{
				std::set<ErrorType> additionalErrors;	

				additionalErrors = ValidateIsUChar(splitStrings[0]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				additionalErrors = ValidateIsUChar(splitStrings[1]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				additionalErrors = ValidateIsUChar(splitStrings[2]);
				errors.insert(additionalErrors.begin(), additionalErrors.end());

				if(hasFourValues)
				{
					additionalErrors = ValidateIsUChar(splitStrings[3]);
					errors.insert(additionalErrors.begin(), additionalErrors.end());
				}

				if(errors.empty())
				{
					result.r = static_cast<unsigned char>(strtol(splitStrings[0].c_str(), 0, 10));
					result.g = static_cast<unsigned char>(strtol(splitStrings[1].c_str(), 0, 10));
					result.b = static_cast<unsigned char>(strtol(splitStrings[2].c_str(), 0, 10));

					if(hasFourValues)
						result.a = static_cast<unsigned char>(strtol(splitStrings[3].c_str(), 0, 10));
				}
				else
				{
					OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
				}
			}	
		}
	}

	return result;
}

 //-----------------------------------------------------------------------------------------------
 FloatRange XMLDocumentParser::GetFloatRangeXMLAttribute( pugi::xml_node node, const std::string& attributeName, const FloatRange& defaultValueIfNotFound )
 {
	 pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	 FloatRange result = defaultValueIfNotFound;

	 if(attribute.empty())
		 return result;
	 else
	 {
		 std::string actualValueAsString = attribute.as_string();
		 std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");
		 uint splitStringSize = splitStrings.size();

		 std::set<ErrorType> errors;

		 if( splitStringSize > 5 )
		 {
			 errors.insert(TOO_MANY_VALUES);
			 OutputErrorSetsAndDie(errors, actualValueAsString, "", node.name());
		 }
		 else
		 {
			 errors = ValidateIsRange(splitStrings);
			 splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t (~)");
			 splitStringSize = splitStrings.size();

			 if( splitStringSize > 2 )
			 {
				 errors.insert(TOO_MANY_VALUES);
				 OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
			 }
			 else
			 {
				 std::set<ErrorType> additionalErrors;

				 additionalErrors = ValidateIsFloat(splitStrings[0]);
				 errors.insert(additionalErrors.begin(), additionalErrors.end());

				 if( splitStringSize > 1 )
				 {
					 additionalErrors = ValidateIsFloat(splitStrings[1]);
					 errors.insert(additionalErrors.begin(), additionalErrors.end());
				 }

				 if(errors.empty())
				 {
					 result.SetMin( static_cast<float>(strtod(splitStrings[0].c_str(),0)) );
					 result.SetMax( result.GetMin() );

					 if( splitStringSize > 1)
						 result.SetMax( static_cast<float>(strtod(splitStrings[1].c_str(),0)) );
				 }
				 else
				 {
					 OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
				 }	
			 }
		 }
	 }

	 return result;
 }

 //-----------------------------------------------------------------------------------------------
 IntRange XMLDocumentParser::GetIntRangeXMLAttribute( pugi::xml_node node, const std::string& attributeName, const IntRange& defaultValueIfNotFound )
 {
	 pugi::xml_attribute attribute = node.attribute(attributeName.c_str());
	 IntRange result = defaultValueIfNotFound;

	 if(attribute.empty())
		 return result;
	 else
	 {
		 std::string actualValueAsString = attribute.as_string();
		 std::vector<std::string> splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t ");
		 uint splitStringSize = splitStrings.size();

		 std::set<ErrorType> errors;

		 if( splitStringSize > 5 )
		 {
			 errors.insert(TOO_MANY_VALUES);
			 OutputErrorSetsAndDie(errors, actualValueAsString, "", node.name());
		 }
		 else
		 {
			 errors = ValidateIsRange(splitStrings);
			 splitStrings = SplitStringBySingleCharacterDelimiters(actualValueAsString,"\n\r\t (~)");
			 splitStringSize = splitStrings.size();

			 if( splitStringSize > 2 )
			 {
				 errors.insert(TOO_MANY_VALUES);
				 OutputErrorSetsAndDie(errors, actualValueAsString, attributeName, node.name());
			 }
			 else
			 {
				 std::set<ErrorType> additionalErrors;

				 additionalErrors = ValidateIsInt(splitStrings[0]);
				 errors.insert(additionalErrors.begin(), additionalErrors.end());

				 if( splitStringSize > 1 )
				 {
					 additionalErrors = ValidateIsInt(splitStrings[1]);
					 errors.insert(additionalErrors.begin(), additionalErrors.end());
				 }

				 if(errors.empty())
				 {
					 result.SetMin( static_cast<int>( strtol( splitStrings[0].c_str(), 0, 10 ) ) );
					 result.SetMax( result.GetMin() );

					 if( splitStringSize > 1)
						 result.SetMax( static_cast<int>( strtol( splitStrings[1].c_str(), 0, 10 ) ) );
				 }
				 else
				 {
					 OutputErrorSetsAndDie( errors, actualValueAsString, attributeName, node.name() );
				 }	
			 }
		 }
	 }

	 return result;
 }

//-----------------------------------------------------------------------------------------------
 void XMLDocumentParser::ValidateXMLAttributes( pugi::xml_node node, const std::string& requiredAttributes, const std::string& optionalAttributes )
{
	std::vector<std::string> requiredAttributesList;
	std::vector<std::string> allAttributesList;
	std::vector<std::string> lowerCase_allAttributesList;
	std::set<std::string> encounteredAttributes;

	std::vector<ErrorMessagePackage> errorMessagePackages;  

	requiredAttributesList = SplitStringBySingleCharacterDelimiters(requiredAttributes, "\n\r\t ,");
	allAttributesList = SplitStringBySingleCharacterDelimiters(optionalAttributes, "\n\r\t ,");

	allAttributesList.insert(allAttributesList.end(), requiredAttributesList.begin(), requiredAttributesList.end());	

	lowerCase_allAttributesList = allAttributesList;

	for(auto iter = lowerCase_allAttributesList.begin(); iter != lowerCase_allAttributesList.end(); ++ iter)
	{
		ToLowerCaseString(*iter);
	}

	pugi::xml_attribute_iterator attribIterator = node.attributes_begin();

	for(; attribIterator != node.attributes_end(); ++ attribIterator)
	{
		std::string asString(attribIterator->name());

		auto foundIter = find(allAttributesList.begin(), allAttributesList.end(), asString);

		if(foundIter != allAttributesList.end())
		{

			if(encounteredAttributes.find(asString) != encounteredAttributes.end())
				errorMessagePackages.push_back(ErrorMessagePackage(DUPLICATE_ATTRIBUTE_NAME, asString, ""));
			else
				encounteredAttributes.insert(asString);

			for(int i = 0; i < (int)requiredAttributesList.size(); ++ i)
			{
				if(requiredAttributesList[i] == asString)
				{
					std::string temp = requiredAttributesList.back();

					if(i != (int)requiredAttributesList.size() - 1)
					{
						requiredAttributesList[i] = temp;
						--i;
					}

					requiredAttributesList.pop_back();
				}
			}
		}
		else
		{
			std::string asLowerCaseString = asString;
			ToLowerCaseString(asLowerCaseString);

			int foundIndex = 0;
			for(foundIter = lowerCase_allAttributesList.begin(); foundIter != lowerCase_allAttributesList.end(); ++ foundIter)
			{
				if((*foundIter) == asLowerCaseString)
				{
					break;
				}
				++ foundIndex;
			}

			if(foundIter != lowerCase_allAttributesList.end())
			{
				errorMessagePackages.push_back(ErrorMessagePackage(INCORRECT_CASE_FOR_ATTRIBUTE, asString, allAttributesList[foundIndex]));
			}
			else
			{
				errorMessagePackages.push_back(ErrorMessagePackage(ATTRIBUTE_NOT_FOUND, asString, requiredAttributes + ", " + optionalAttributes));
			}
		}
	}
	if(!requiredAttributesList.empty())
	{
		std::string remainingAttributes;
		while(!requiredAttributesList.empty())
		{
			remainingAttributes += requiredAttributesList.back();
			requiredAttributesList.pop_back();
		}

		errorMessagePackages.push_back(ErrorMessagePackage(MISSING_REQUIRED_ATTRIBUTE, "", remainingAttributes));
	}

	if(!errorMessagePackages.empty())
		OutputErrorPackagesAndDie(errorMessagePackages, node.name());
}

//-----------------------------------------------------------------------------------------------
 void XMLDocumentParser::ValidateXMLChildElements( pugi::xml_node node, const std::string& requiredChildElements, const std::string& optionalChildElements )
{
	std::vector<std::string> requiredChildElementsList;
	std::vector<std::string> allChildElementsList;
	std::vector<std::string> lowerCase_ChildElementsList;
	//std::set<std::string> encounteredChildren;

	std::vector<ErrorMessagePackage> errorMessagePackages; 

	requiredChildElementsList = SplitStringBySingleCharacterDelimiters(requiredChildElements, "\n\r\t ,");
	allChildElementsList = SplitStringBySingleCharacterDelimiters(optionalChildElements, "\n\r\t ,");

	allChildElementsList.insert(allChildElementsList.end(), requiredChildElementsList.begin(), requiredChildElementsList.end());	

	lowerCase_ChildElementsList = allChildElementsList;

	for(auto iter = lowerCase_ChildElementsList.begin(); iter != lowerCase_ChildElementsList.end(); ++ iter)
	{
		ToLowerCaseString(*iter);
	}

	pugi::xml_node_iterator childIterator = node.begin();

	for(; childIterator != node.end(); ++ childIterator)
	{
		std::string asString(childIterator->name());

		auto foundIter = find(allChildElementsList.begin(), allChildElementsList.end(), asString);

		if(foundIter != allChildElementsList.end())
		{

			//if(encounteredChildren.find(asString) != encounteredChildren.end())
			//	errorMessagePackages.push_back(ErrorMessagePackage(DUPLICATE_CHILD_NODE_NAME, asString, ""));
			//else
			//	encounteredChildren.insert(asString);

			for(int i = 0; i < (int)requiredChildElementsList.size(); ++ i)
			{
				if(requiredChildElementsList[i] == asString)
				{
					std::string temp = requiredChildElementsList.back();

					if(i != (int)requiredChildElementsList.size() - 1)
					{
						requiredChildElementsList[i] = temp;
						--i;
					}

					requiredChildElementsList.pop_back();
					break;
				}
			}
		}
		else
		{
			std::string asLowerCaseString = asString;
			ToLowerCaseString(asLowerCaseString);

			int foundIndex = 0;
			for(foundIter = lowerCase_ChildElementsList.begin(); foundIter != lowerCase_ChildElementsList.end(); ++ foundIter)
			{
				if((*foundIter) == asLowerCaseString)
				{
					break;
				}
				++ foundIndex;
			}

			if(foundIter != lowerCase_ChildElementsList.end())
			{
				errorMessagePackages.push_back(ErrorMessagePackage(INCORRECT_CASE_FOR_CHILD_NODE, asString, allChildElementsList[foundIndex]));
			}
			else
			{
				errorMessagePackages.push_back(ErrorMessagePackage(CHILD_NODE_NOT_FOUND, asString, requiredChildElements + ", " + optionalChildElements));
			}
		}
	}
	if(!requiredChildElementsList.empty())
	{
		std::string remainingChildren;
		while(!requiredChildElementsList.empty())
		{
			remainingChildren += requiredChildElementsList.back();
			requiredChildElementsList.pop_back();
		}

		errorMessagePackages.push_back(ErrorMessagePackage(MISSING_REQUIRED_CHILD_NODE, "", remainingChildren));
	}

	if(!errorMessagePackages.empty())
		OutputErrorPackagesAndDie(errorMessagePackages, node.name());
}

//-----------------------------------------------------------------------------------------------
 void XMLDocumentParser::OutputErrorPackagesAndDie( const std::vector<ErrorMessagePackage>& packages, const std::string& parentNodeName )
{
	std::string errorLog = "Errors in file " + m_filename + ":\n";

	for(auto iter = packages.begin(); iter != packages.end(); ++iter)
	{
		errorLog += "\n\n" + GetAppropriateErrorMessage(iter->m_error, iter->m_incorrectString, iter->m_expectedValue, parentNodeName);
	}

	OpenGLRenderer::RenderErrorWindow(errorLog, "Errors:");
	std::exit(0);
}

//-----------------------------------------------------------------------------------------------
 void XMLDocumentParser::OutputErrorSetsAndDie( const std::set<ErrorType>& errors, const std::string& badValueAsString, const std::string& attributeName, const std::string& parentNodeName )
{
	std::string errorLog = "Errors in file " + m_filename + ":\n";

	for(auto iter = errors.begin(); iter != errors.end(); ++iter)
	{
		errorLog += "\n\n" + GetAppropriateErrorMessage((*iter), badValueAsString, attributeName, parentNodeName);
	}

	OpenGLRenderer::RenderErrorWindow(errorLog, "Errors:");
	std::exit(0);
}

//-----------------------------------------------------------------------------------------------
 std::string XMLDocumentParser::GetAppropriateErrorMessage( ErrorType error, const std::string& badValueAsString, const std::string& optionalExpectedValue, const std::string& parentNodeName )
{
	std::string errorMessage;

	switch(error)
	{
	case TOO_MANY_VALUES:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" has too many values than expected.";
		break;

	case NOT_BOOL_VALUE:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" is not a boolean value.\nCorrect values are:\ntrue\n\nfalse";
		break;

	case HAS_NON_DIGIT_VALUES:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" has one or numbers with non-digit symbols.";
		break;

	case VALUE_OUT_OF_RANGE:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" is out of range. Acceptable values are in the range 0 - 255.";
		break;

	case ILLEGAL_DECIMAL:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" has one or numbers with an illegal decimal.";
		break;

	case TOO_MANY_PARENTHESIS:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" has too many parenthesis.";
		break;

	case MISSING_COMMA:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" is missing one or more commas.";
		break;

	case MISSING_TILDE:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" is missing one or more tildes.";
		break;

	case MISSING_CLOSING_PARENTHESIS:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" is missing a closing parenthesis.";
		break;

	case MISSING_OPENING_PARENTHESIS:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" is missing a opening parenthesis.";
		break;

	case TOO_MANY_COMMAS:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" has too many commas.";
		break;

	case TOO_MANY_TILDES:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" has too many tildes.";
		break;

	case TOO_MANY_DECIMALS:
		errorMessage = "For attribute \"" + optionalExpectedValue + "\": \"" + badValueAsString + "\" has one or more numbers with too many decimals.";
		break;

	case INCORRECT_CASE_FOR_ATTRIBUTE:
		errorMessage = "For node \"" + parentNodeName + "\": \"" + badValueAsString + "\" is an existing attribute, but does have correct capitalization format. Correct format: \"" + optionalExpectedValue + "\".";
		break;

	case ATTRIBUTE_NOT_FOUND:
		errorMessage = "For node \"" + parentNodeName + "\": \"" + badValueAsString + "\" is not a attribute of the parent node. Accepted attribute names: \"" + optionalExpectedValue + "\".";
		break;

	case DUPLICATE_ATTRIBUTE_NAME:
		errorMessage = "For node \"" + parentNodeName + "\": \"" + badValueAsString + "\" appears more than once for the parent node.";
		break;

	case MISSING_REQUIRED_ATTRIBUTE:
		errorMessage = "For node \"" + parentNodeName + "\": missing the following attributes: \"" + optionalExpectedValue + "\".";
		break;

	case INCORRECT_CASE_FOR_CHILD_NODE:
		errorMessage = "For node \"" + parentNodeName + "\": \"" + badValueAsString + "\" is an existing child element, but does have correct capitalization format. Correct format: \"" + optionalExpectedValue + "\".";
		break;
	case CHILD_NODE_NOT_FOUND:
		errorMessage = "For node \"" + parentNodeName + "\": \"" + badValueAsString + "\" is not a child element of the parent node. Accepted child element names: \"" + optionalExpectedValue + "\".";
		break;

	case DUPLICATE_CHILD_NODE_NAME:
		errorMessage = "For node \"" + parentNodeName + "\": \"" + badValueAsString + "\" appears more than once for the parent node.";
		break;

	case MISSING_REQUIRED_CHILD_NODE:
		errorMessage = "For node \"" + parentNodeName + "\": missing the following child elements: \"" + optionalExpectedValue + "\".";
		break;
	}

	return errorMessage;
}

