#ifndef COMMON_UTILITIES
#define COMMON_UTILITIES

#define NOMINMAX

#define XINPUT_GAMEPAD_LEFT_TRIGGER   0x0400
#define XINPUT_GAMEPAD_RIGHT_TRIGGER  0x0800
#define XINPUT_GAMEPAD_LEFT_JOY       0xA000
#define XINPUT_GAMEPAD_RIGHT_JOY	  0xB000

#define MOUSE_LEFT	 0
#define MOUSE_RIGHT  1
#define MOUSE_MIDDLE 2
#define MOUSE_L_DOUBLE_CLICK 3
#define MOUSE_R_DOUBLE_CLICK 4
#define MOUSE_M_DOUBLE_CLICK 5

#include <string>
#include <vector>
#include <cfloat>
#include <cassert>
#include <math.h> 
#include <set>

#include "Engine/Primitives/Vector2.hpp"

typedef unsigned int uint; 
typedef unsigned char uchar;

typedef Vector2i TileCoords;
typedef Vector2f WorldCoords;

const float PI = 3.1415926535897932384626433832795f;
const float TWO_PI = 2.f * PI;
const float MAX_FLOAT_VALUE = FLT_MAX;
const float PI_OVER_180 = PI / 180.f;
const float _180_OVER_PI = 180.f / PI;
const float RAND_MAX_INVERSE = 1.f / (float)RAND_MAX;

enum TypeEnum
{
	TYPE_BOOL,
	TYPE_STRING,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_VEC2I,
	TYPE_VEC2F,
	TYPE_VEC3I,
	TYPE_VEC3F,
	TYPE_RGBA
};

//-----------------------------------------------------------------------------------------------
enum ErrorType
{
	TOO_MANY_VALUES,
	NOT_BOOL_VALUE,
	HAS_NON_DIGIT_VALUES,
	VALUE_OUT_OF_RANGE,
	ILLEGAL_DECIMAL,
	TOO_MANY_PARENTHESIS,
	MISSING_COMMA,
	MISSING_TILDE,
	MISSING_PARENTHESIS,
	MISSING_CLOSING_PARENTHESIS,
	MISSING_OPENING_PARENTHESIS,
	TOO_MANY_COMMAS,
	TOO_MANY_TILDES,
	TOO_MANY_DECIMALS,
	INCORRECT_CASE_FOR_ATTRIBUTE,
	ATTRIBUTE_NOT_FOUND,
	DUPLICATE_ATTRIBUTE_NAME,
	MISSING_REQUIRED_ATTRIBUTE,
	INCORRECT_CASE_FOR_CHILD_NODE,
	CHILD_NODE_NOT_FOUND,
	DUPLICATE_CHILD_NODE_NAME,
	MISSING_REQUIRED_CHILD_NODE
};

inline size_t GetFileSize( FILE* file )
{
	size_t sizeOfFile;

	fseek( file, 0, SEEK_END );
	sizeOfFile = ftell( file );
	fseek( file, 0, SEEK_SET );

	return sizeOfFile;
}

inline float GetRandomValueBetweenZeroAndOne()
{
	return static_cast<float>( rand() ) * RAND_MAX_INVERSE;
}

inline float ConvertRadiansToDegrees(float angle)
{
	angle *= _180_OVER_PI;
	return angle;
}

inline float ConvertDegreesToRadians(float angle)
{
	angle *= PI_OVER_180;
	return angle;
}

inline float SmoothStep(float x)
{
	return (3.f * (x * x) - 2.f * (x * x * x));
}

inline float GetFloatMin( float a, float b )
{
	if ( a > b )
		return b;
	else 
		return a;
}

inline float GetFloatMax( float a, float b )
{
	if ( a < b )
		return b;
	else 
		return a;
}

inline TileCoords ToTileCoords( const WorldCoords& worldCoords )
{
	TileCoords result;

	result.x = static_cast<int>( floor( worldCoords.x ) );
	result.y = static_cast<int>( floor( worldCoords.y ) );

	return result;
}

inline WorldCoords ToWorldCoords( const TileCoords& tileCoords )
{
	return WorldCoords( static_cast<float>( tileCoords.x ), static_cast<float>( tileCoords.y ) );
}

inline WorldCoords GetWorldNorth()
{
	return WorldCoords( 0.f, 1.f );
}

inline WorldCoords GetWorldEast()
{
	return WorldCoords( 1.f, 0.f );
}

inline WorldCoords GetWorldSouth()
{
	return WorldCoords( 0.f, -1.f );
}

inline WorldCoords GetWorldWest()
{
	return WorldCoords( -1.f, 0.f );
}

inline TileCoords GetTileNorth()
{
	return TileCoords( 0, 1 );
}

inline TileCoords GetTileEast()
{
	return TileCoords( 1, 0 );
}

inline TileCoords GetTileSouth()
{
	return TileCoords( 0, -1 );
}

inline TileCoords GetTileWest()
{
	return TileCoords( -1, 0 );
}

inline std::vector<std::string> SplitStringByStringDelimiter(const std::string& stringToSplit, const std::string& delimiter)

{
	std::vector<std::string> strings;
	size_t pos = 0;
	std::string token;
	std::string s = stringToSplit;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		strings.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	strings.push_back(s);

	return strings;
}

inline std::vector<std::string> SplitStringBySingleCharacterDelimiters(const std::string& stringToSplit, const std::string& delimiters)
{
	std::vector<std::string> strings; //Use vector to add the words

	std::size_t prev_pos = 0, pos;
	while ((pos = stringToSplit.find_first_of(delimiters, prev_pos)) != std::string::npos)
	{
		if (pos > prev_pos)
			strings.push_back(stringToSplit.substr(prev_pos, pos-prev_pos));
		prev_pos= pos+1;
	}
	if (prev_pos< stringToSplit.length())
		strings.push_back(stringToSplit.substr(prev_pos, std::string::npos));

	return strings;
}

//-----------------------------------------------------------------------------------------------
inline void ToLowerCaseString( std::string& outString )
{
	for(int i = 0; i < (int)outString.length(); ++ i)
	{
		outString[i] = static_cast<char>(tolower(static_cast<int>(outString[i])));
	}
}

//-----------------------------------------------------------------------------------------------
inline void ToUpperCaseString( std::string& outString )
{
	for(int i = 0; i < (int)outString.length(); ++ i)
	{
		outString[i] = static_cast<char>(toupper(static_cast<int>(outString[i])));
	}
}

inline std::set<ErrorType>		ValidateIsBool( std::string& valueAsString );
inline std::set<ErrorType>		ValidateIsInt( const std::string& valueAsString );
inline std::set<ErrorType>		ValidateIsFloat( const std::string& valueAsString );
inline std::set<ErrorType>		ValidateIsUChar( const std::string& valueAsString );

std::set<ErrorType>		ValidateIsVector2( const std::vector<std::string>& valueAsVectorOfStrings );
std::set<ErrorType>		ValidateIsVector3( const std::vector<std::string>& valueAsVectorOfStrings );
std::set<ErrorType>		ValidateIsVector4( const std::vector<std::string>& valueAsVectorOfStrings );
std::set<ErrorType>		ValidateIsRange( const std::vector<std::string>& valueAsVectorOfStrings );

//-----------------------------------------------------------------------------------------------
struct FloatRange
{
	FloatRange() {}
	FloatRange( float min, float max )
	{
		if( max < min )
		{
			minValue = max;
			maxValue = min;
		}
		else
		{
			minValue = min;
			maxValue = max;
		}
	}

	inline float GetRandomFloatInRange() const;
	inline float GetMin() const;
	inline float GetMax() const;

	inline void SetMin( float value );
	inline void SetMax( float value );

private:

	float minValue;
	float maxValue;
};

//-----------------------------------------------------------------------------------------------
struct IntRange
{
	IntRange() {}
	IntRange( int min, int max )
	{
		if( max < min )
		{
			minValue = max;
			maxValue = min;
		}
		else
		{
			minValue = min;
			maxValue = max;
		}
	}

	inline int GetRandomIntInRange() const;
	inline int GetMin() const;
	inline int GetMax() const;

	inline void SetMin( int value );
	inline void SetMax( int value );

private:
	int minValue;
	int maxValue;
};

class WeightedChoice
{
public:

	WeightedChoice( const std::string& asString );
	WeightedChoice() : m_sumWeightings( 0 ) {}

	struct ChoiceAndWeight
	{
		ChoiceAndWeight() {}
		ChoiceAndWeight( const std::string& c, int w ) : choice( c ), weight( w ) {}

		std::string choice;
		int weight;
	};


	inline void					AddChoiceWithWeight( const ChoiceAndWeight& choiceWithWeight );
	inline void					AddChoiceWithWeight( const std::string& choice, int weight );
	inline const std::string	GetRandomValueFromChoices() const;

private:

	std::set<ErrorType> ValidateIsWeightedChoice( const std::string& asString );
	std::set<ErrorType> BuildWeightedChoiceFromString( const std::string& asString );

	int m_sumWeightings;
	std::vector< ChoiceAndWeight > m_choicesWithWeightings;
};


//-----------------------------------------------------------------------------------------------

void WeightedChoice::AddChoiceWithWeight( const ChoiceAndWeight& choiceWithWeight )
{
	assert( choiceWithWeight.weight >= 0 );
	m_sumWeightings += choiceWithWeight.weight;

	m_choicesWithWeightings.push_back( choiceWithWeight );
}

void WeightedChoice::AddChoiceWithWeight( const std::string& choice, int weight )
{
	assert( weight >= 0 );

	m_sumWeightings += weight;

	m_choicesWithWeightings.push_back( ChoiceAndWeight( choice, weight ) );
}

const std::string WeightedChoice::GetRandomValueFromChoices() const
{
	int randomInt = rand() % m_sumWeightings;
	
	int index = 0;

	for( ; index < m_sumWeightings; ++ index )
	{
		randomInt -= m_choicesWithWeightings[ index ].weight;

		if( randomInt <= 0 )
		{
			break;
		}
	}
	const std::string result = m_choicesWithWeightings[ index ].choice;

	return result;
}

//-----------------------------------------------------------------------------------------------
float FloatRange::GetRandomFloatInRange() const
{
	float randomNumber = GetRandomValueBetweenZeroAndOne();
	
	return minValue + ( maxValue - minValue ) * randomNumber;
}

float FloatRange::GetMin() const
{
	return minValue;
}

float FloatRange::GetMax() const
{
	return maxValue;
}

void FloatRange::SetMin( float value )
{
	if( value > maxValue )
	{
		maxValue = minValue;

	}
	minValue = value;
}

void FloatRange::SetMax( float value )
{
	if( value < minValue )
	{
		minValue = maxValue;
	}
	maxValue = value;
}

//-----------------------------------------------------------------------------------------------

int IntRange::GetRandomIntInRange() const
{
	return minValue + rand() % ( maxValue - minValue + 1);
}

int IntRange::GetMin() const
{
	return minValue;
}

int IntRange::GetMax() const
{
	return maxValue;
}

void IntRange::SetMin( int value )
{
	if( value > maxValue )
	{
		maxValue = minValue;

	}
	minValue = value;
}

void IntRange::SetMax( int value )
{
	if( value < minValue )
	{
		minValue = maxValue;
	}
	maxValue = value;
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
inline std::set<ErrorType> ValidateIsBool( std::string& valueAsString )
{
	std::set<ErrorType> errors;

	ToLowerCaseString( valueAsString );

	if( valueAsString[0] == 't' && valueAsString.size() == 4 )
	{
		bool result = valueAsString[1] == 'r' && valueAsString[2] == 'u' && valueAsString[3] == 'e';

		if( result )
			valueAsString = "1";
		else
			errors.insert(NOT_BOOL_VALUE);
	}
	else if( valueAsString[0] == 'f' && valueAsString.size() == 5)
	{
		bool result = valueAsString[1] == 'a' && valueAsString[2] == 'l' && valueAsString[3] == 's' && valueAsString[4] == 'e';

		if( result )
			valueAsString = "0";
		else
			errors.insert(NOT_BOOL_VALUE);
	}
	else
	{
		errors.insert(NOT_BOOL_VALUE);
	}

	return errors;
}

//-----------------------------------------------------------------------------------------------
inline std::set<ErrorType> ValidateIsInt( const std::string& valueAsString )
{
	std::set<ErrorType> errors;

	for(auto iter = valueAsString.begin(); iter != valueAsString.end(); ++ iter)
	{
		if(!isdigit( *iter ) && *iter != '-' )
		{
			errors.insert(HAS_NON_DIGIT_VALUES);

			if(*iter == '.')
				errors.insert(ILLEGAL_DECIMAL);
		}
	}

	return errors;
}

//-----------------------------------------------------------------------------------------------
inline std::set<ErrorType> ValidateIsFloat( const std::string& valueAsString )
{
	std::set<ErrorType> errors;
	int decimalCount = 0;

	for(auto iter = valueAsString.begin(); iter != valueAsString.end(); ++ iter)
	{
		if(!isdigit(*iter) && *iter != '-' )
		{
			if(*iter == '.')
			{
				if(decimalCount == 0)
				{
					++ decimalCount;
					continue;
				}
				else
				{
					errors.insert(TOO_MANY_DECIMALS);
				}
			}
			errors.insert(HAS_NON_DIGIT_VALUES);
		}
	}

	return errors;
}

//-----------------------------------------------------------------------------------------------
inline std::set<ErrorType> ValidateIsUChar( const std::string& valueAsString )
{
	std::set<ErrorType> errors;
	char currentCharacter;

	for(auto iter = valueAsString.begin(); iter != valueAsString.end(); ++ iter)
	{
		currentCharacter = *iter;
		if(!isdigit(currentCharacter))
		{
			errors.insert(HAS_NON_DIGIT_VALUES);

			if(currentCharacter == '.')
				errors.insert(ILLEGAL_DECIMAL);
		}
	}

	if(errors.empty())
	{
		int asInteger = static_cast<int>(strtol(valueAsString.c_str(), 0, 10));
		if(asInteger > 255 || asInteger < 0)
		{
			errors.insert(VALUE_OUT_OF_RANGE);
		}
	}

	return errors;
}


#endif