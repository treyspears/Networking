#include "CommonUtilities.hpp"

//-----------------------------------------------------------------------------------------------
WeightedChoice::WeightedChoice( const std::string& asString )
	: m_sumWeightings( 0 )
{
	std::set<ErrorType> errors = ValidateIsWeightedChoice( asString );

	if( errors.size() == 0 )
	{
		std::set<ErrorType> additionalErrors = BuildWeightedChoiceFromString( asString );
		errors.insert( additionalErrors.begin(), additionalErrors.end() );
	}

	assert( errors.size() == 0 );
}

//-----------------------------------------------------------------------------------------------
std::set<ErrorType> WeightedChoice::ValidateIsWeightedChoice( const std::string& asString )
{
	std::set<ErrorType> errors;

	std::vector<std::string> asVectorOfStrings = SplitStringBySingleCharacterDelimiters( asString, "\t\r\n ");

	int openParenthesisCount = 0;
	int closingParenthesisCount = 0;
	char currentChar = '\n';
	char lastChar = currentChar;
	char nextChar = lastChar;

	std::string currentString;
	std::string previousString;
	std::string nextString;

	for( int vectorMemberIndex = 0; vectorMemberIndex < static_cast<int>( asVectorOfStrings.size() ); ++ vectorMemberIndex )
	{
		currentString = asVectorOfStrings[ vectorMemberIndex ];

		if( vectorMemberIndex !=static_cast<int>( asVectorOfStrings.size() ) - 1 )
		{
			nextString = asVectorOfStrings[ vectorMemberIndex + 1 ];
		}
		else
		{
			nextString.clear();
		}

		for( int charInStringIndex = 0; charInStringIndex < static_cast<int>( currentString.size() ); ++ charInStringIndex )
		{
			currentChar = currentString[ charInStringIndex ];

			if( charInStringIndex != static_cast<int>( currentString.size() ) - 1 )
			{
				nextChar = currentString[ charInStringIndex + 1 ];
			}
			else if( nextString.size() > 0 )
			{
				nextChar = nextString[0];
			}
			else
				nextChar = '\n';

			//-----------------------------------------------------------------------------------------------

			if( currentChar == '(' )
			{
				++ openParenthesisCount;
			}
			else if( currentChar == ')' )
			{
				if( openParenthesisCount != closingParenthesisCount )
					++ closingParenthesisCount;
				else
					errors.insert( MISSING_OPENING_PARENTHESIS );
			}
			else if( currentChar == ',' )
			{
				bool badNextChar = nextChar == ',' || nextChar == '\n';
				bool badPrevChar = lastChar == ',' || lastChar == '\n';

				if( badNextChar || badPrevChar )
				{
					errors.insert( TOO_MANY_COMMAS );
				}

				//if( !( lastChar == ')' && nextChar == '(') && openParenthesisCount <= closingParenthesisCount )
				//{
				//	errors.insert( MISSING_PARENTHESIS );
				//}
			}

			//-----------------------------------------------------------------------------------------------

			lastChar = currentChar;
		}

		previousString = currentString;
	}

	if( openParenthesisCount < closingParenthesisCount )
	{
		errors.insert( MISSING_OPENING_PARENTHESIS );
	}
	else if( closingParenthesisCount < openParenthesisCount )
	{
		errors.insert( MISSING_CLOSING_PARENTHESIS );
	}

	return errors;
}

//-----------------------------------------------------------------------------------------------
std::set<ErrorType> WeightedChoice::BuildWeightedChoiceFromString( const std::string& asString )
{
	std::set<ErrorType> errors, additionalErrors;

	std::vector<std::string> vectorOfStrings = SplitStringBySingleCharacterDelimiters( asString, "\t\n\r");
	std::string compactedString;

	for( auto iter = vectorOfStrings.begin(); iter != vectorOfStrings.end(); ++ iter )
	{
		compactedString += (*iter);
	}

	std::string furtherCompactedString;
	char currentChar = '\n';
	char previousChar = '\n';
	bool isCurrentEntry = false;

	for( int i = 0; i < static_cast<int>( compactedString.size() ); ++ i )
	{
		currentChar = compactedString[ i ];

		if( currentChar == '(' )
			isCurrentEntry = true;

		if( !isCurrentEntry && currentChar == ' ' )
		{
			continue;
		}

		if( currentChar == ')' )
			isCurrentEntry = false;

		if( currentChar == ',' && previousChar == ')' )
		{
			continue;
		}
		else
		{
			furtherCompactedString += currentChar;
		}

		previousChar = currentChar;
	}

	currentChar = '\n';
	char nextChar = '\n';
	isCurrentEntry = false;

	std::string finalStringToParse;
	std::string weightAsString;

	ChoiceAndWeight currentChoiceWithWeight;

	for( int i = 0; i < static_cast<int>( furtherCompactedString.size() ); ++ i )
	{
		currentChar = furtherCompactedString[ i ];

		if( i < static_cast<int>( furtherCompactedString.size() ) - 1 )
		{
			nextChar = furtherCompactedString[ i + 1 ];
		}
		else
		{
			nextChar = '\n';
		}

		if( currentChar == '(' )
			isCurrentEntry = true;

		if( isCurrentEntry )
		{
			finalStringToParse += currentChar;
		}

		if( !isCurrentEntry )
			weightAsString += currentChar;

		if( currentChar == ')' )
			isCurrentEntry = false;
		
		if( i == 0 && currentChar == '(' )
		{
			currentChoiceWithWeight.weight = 1;
			m_choicesWithWeightings.push_back( currentChoiceWithWeight );
			m_sumWeightings += 1;

			weightAsString.clear();
		}

		if( nextChar == '(' )
		{
			if( weightAsString.size() > 0 )
			{
				additionalErrors = ValidateIsInt( weightAsString );
				errors.insert( additionalErrors.begin(), additionalErrors.end() );

				if( errors.size() == 0 )
					currentChoiceWithWeight.weight = static_cast<int>(strtol(weightAsString.c_str(), 0, 10));

				weightAsString.clear();
			}
			else
			{
				currentChoiceWithWeight.weight = 1;
			}

			m_choicesWithWeightings.push_back( currentChoiceWithWeight );
			m_sumWeightings += currentChoiceWithWeight.weight;
		}
	}

	errors.insert( additionalErrors.begin(), additionalErrors.end() );

	vectorOfStrings = SplitStringBySingleCharacterDelimiters( finalStringToParse, "()" );
	std::vector<std::string> splitStrings;

	for( int i = 0; i < static_cast<int>( vectorOfStrings.size() ); ++ i )
	{
		m_choicesWithWeightings[i].choice = vectorOfStrings[i];
	}

	return errors;
}

//-----------------------------------------------------------------------------------------------
std::set<ErrorType> ValidateIsVector2( const std::vector<std::string>& valueAsVectorOfStrings )
{
	std::set<ErrorType> errors;
	bool shouldLookForClosingParenthesis = false;
	int openParenthesisCount = 0;
	int closingParenthesisCount = 0;
	int commaCount = 0;

	if(valueAsVectorOfStrings[0][0] == '(')
		shouldLookForClosingParenthesis = true;

	for(auto stringIter = valueAsVectorOfStrings.begin(); stringIter != valueAsVectorOfStrings.end(); ++ stringIter)
	{
		for(auto iter = stringIter->begin(); iter != stringIter->end(); ++ iter)
		{
			if((*iter) == '(')
			{
				if(openParenthesisCount == 0)
					++ openParenthesisCount;
				else
					errors.insert(TOO_MANY_PARENTHESIS);
			}
			else if((*iter) == ',')
			{
				if(commaCount == 0)
					++ commaCount;
				else
					errors.insert(TOO_MANY_COMMAS);
			}
			else if((*iter) == ')')
			{
				if(!shouldLookForClosingParenthesis)
				{
					errors.insert(MISSING_OPENING_PARENTHESIS);
					errors.insert(TOO_MANY_PARENTHESIS);
				}

				if(closingParenthesisCount == 0)
					++ closingParenthesisCount;
				else
					errors.insert(TOO_MANY_PARENTHESIS);
			}
		}
	}

	if(openParenthesisCount > 0 && closingParenthesisCount == 0)
	{
		errors.insert(MISSING_CLOSING_PARENTHESIS);
		errors.insert(TOO_MANY_PARENTHESIS);
	}

	if(commaCount == 0)
		errors.insert(MISSING_COMMA);

	return errors;
}

//-----------------------------------------------------------------------------------------------
std::set<ErrorType> ValidateIsVector3( const std::vector<std::string>& valueAsVectorOfStrings )
{
	std::set<ErrorType> errors;
	bool shouldLookForClosingParenthesis = false;
	int openParenthesisCount = 0;
	int closingParenthesisCount = 0;
	int commaCount = 0;

	if(valueAsVectorOfStrings[0][0] == '(')
		shouldLookForClosingParenthesis = true;

	for(auto stringIter = valueAsVectorOfStrings.begin(); stringIter != valueAsVectorOfStrings.end(); ++ stringIter)
	{
		for(auto iter = stringIter->begin(); iter != stringIter->end(); ++ iter)
		{
			if((*iter) == '(')
			{
				if(openParenthesisCount == 0)
					++ openParenthesisCount;
				else
					errors.insert(TOO_MANY_PARENTHESIS);
			}
			else if((*iter) == ',')
			{
				if(commaCount <= 1)
					++ commaCount;
				else
					errors.insert(TOO_MANY_COMMAS);
			}
			else if((*iter) == ')')
			{
				if(!shouldLookForClosingParenthesis)
				{
					errors.insert(MISSING_OPENING_PARENTHESIS);
					errors.insert(TOO_MANY_PARENTHESIS);
				}

				if(closingParenthesisCount == 0)
					++ closingParenthesisCount;
				else
					errors.insert(TOO_MANY_PARENTHESIS);
			}
		}
	}

	if(openParenthesisCount > 0 && closingParenthesisCount == 0)
	{
		errors.insert(MISSING_CLOSING_PARENTHESIS);
		errors.insert(TOO_MANY_PARENTHESIS);
	}

	if(commaCount < 2)
		errors.insert(MISSING_COMMA);

	return errors;
}

//-----------------------------------------------------------------------------------------------
std::set<ErrorType> ValidateIsVector4( const std::vector<std::string>& valueAsVectorOfStrings )
{
	std::set<ErrorType> errors;
	bool shouldLookForClosingParenthesis = false;
	int openParenthesisCount = 0;
	int closingParenthesisCount = 0;
	int commaCount = 0;

	if(valueAsVectorOfStrings[0][0] == '(')
		shouldLookForClosingParenthesis = true;

	for(auto stringIter = valueAsVectorOfStrings.begin(); stringIter != valueAsVectorOfStrings.end(); ++ stringIter)
	{
		for(auto iter = stringIter->begin(); iter != stringIter->end(); ++ iter)
		{
			if((*iter) == '(')
			{
				if(openParenthesisCount == 0)
					++ openParenthesisCount;
				else
					errors.insert(TOO_MANY_PARENTHESIS);
			}
			else if((*iter) == ',')
			{
				if(commaCount <= 2)
					++ commaCount;
				else
					errors.insert(TOO_MANY_COMMAS);
			}
			else if((*iter) == ')')
			{
				if(!shouldLookForClosingParenthesis)
				{
					errors.insert(MISSING_OPENING_PARENTHESIS);
					errors.insert(TOO_MANY_PARENTHESIS);
				}

				if(closingParenthesisCount == 0)
					++ closingParenthesisCount;
				else
					errors.insert(TOO_MANY_PARENTHESIS);
			}
		}
	}

	if(openParenthesisCount > 0 && closingParenthesisCount == 0)
	{
		errors.insert(MISSING_CLOSING_PARENTHESIS);
		errors.insert(TOO_MANY_PARENTHESIS);
	}

	if(commaCount < 3)
		errors.insert(MISSING_COMMA);

	return errors;
}

//-----------------------------------------------------------------------------------------------
std::set<ErrorType> ValidateIsRange( const std::vector<std::string>& valueAsVectorOfStrings )
{
	std::set<ErrorType> errors;
	bool shouldLookForClosingParenthesis = false;
	int openParenthesisCount = 0;
	int closingParenthesisCount = 0;
	int tildeCount = 0;

	if(valueAsVectorOfStrings[0][0] == '(')
		shouldLookForClosingParenthesis = true;

	for(auto stringIter = valueAsVectorOfStrings.begin(); stringIter != valueAsVectorOfStrings.end(); ++ stringIter)
	{
		for(auto iter = stringIter->begin(); iter != stringIter->end(); ++ iter)
		{
			if((*iter) == '(')
			{
				if(openParenthesisCount == 0)
					++ openParenthesisCount;
				else
					errors.insert(TOO_MANY_PARENTHESIS);
			}
			else if((*iter) == '~')
			{
				if(tildeCount == 0)
					++ tildeCount;
				else
					errors.insert(TOO_MANY_TILDES);
			}
			else if((*iter) == ')')
			{
				if(!shouldLookForClosingParenthesis)
				{
					errors.insert(MISSING_OPENING_PARENTHESIS);
					errors.insert(TOO_MANY_PARENTHESIS);
				}

				if(closingParenthesisCount == 0)
					++ closingParenthesisCount;
				else
					errors.insert(TOO_MANY_PARENTHESIS);
			}
		}
	}

	if(openParenthesisCount > 0 && closingParenthesisCount == 0)
	{
		errors.insert(MISSING_CLOSING_PARENTHESIS);
		errors.insert(TOO_MANY_PARENTHESIS);
	}

	if( tildeCount == 0 && valueAsVectorOfStrings.size() > 1 )
		errors.insert(MISSING_TILDE);

	return errors;
}