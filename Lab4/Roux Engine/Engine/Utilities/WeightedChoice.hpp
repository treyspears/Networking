//#include "CommonUtilities.hpp"
//
////-----------------------------------------------------------------------------------------------
//class WeightedChoice
//{
//public:
//
//	struct ChoiceAndWeight
//	{
//		ChoiceAndWeight() {}
//		ChoiceAndWeight( std::string c, int w ) : choice( c ), weight( w ) {}
//
//		std::string choice;
//		int weight;
//	};
//
//
//	inline void AddChoiceWithWeight( const ChoiceAndWeight& choiceWithWeight );
//	inline void	AddChoiceWithWeight( float choice, int weight );
//	inline std::string	GetRandomValueFromChoices() const;
//
//private:
//
//	inline std::set<ErrorType> ValidateIsWeightedChoice( const std::string& asString );
//	inline std::set<ErrorType> GetStringToParseAndAddWeights( const std::string& inString, std::string& outString );
//	virtual std::set<ErrorType> BuildWeightedChoiceFromString( const std::string& asString );
//
//	int m_sumWeightings;
//	std::vector< ChoiceAndWeight > m_choicesWithWeightings;
//};
//
////-----------------------------------------------------------------------------------------------
//class WeightedChoiceInt
//{
//public:
//
//	WeightedChoiceInt( const std::string& asString );
//	WeightedChoiceInt() : m_sumWeightings( 0 ) {}
//
//	struct ChoiceAndWeight
//	{
//		ChoiceAndWeight() {}
//		ChoiceAndWeight( float c, int w ) : choice( c ), weight( w ) {}
//
//		float choice;
//		int weight;
//	};
//
//
//	inline void AddChoiceWithWeight( const ChoiceAndWeight& choiceWithWeight );
//	inline void	AddChoiceWithWeight( float choice, int weight );
//	inline float	GetRandomValueFromChoices() const;
//
//private:
//
//	inline std::set<ErrorType> ValidateIsWeightedChoice( const std::string& asString );
//	inline std::set<ErrorType> BuildWeightedChoiceFromString( const std::string& asString, TypeEnum typeToParse );
//
//	int m_sumWeightings;
//	std::vector< ChoiceAndWeight > m_choicesWithWeightings;
//};
//
////-----------------------------------------------------------------------------------------------
//class WeightedChoiceString
//{
//public:
//
//	WeightedChoiceString( const std::string& asString );
//	WeightedChoiceString() : m_sumWeightings( 0 ) {}
//
//	struct ChoiceAndWeight
//	{
//		ChoiceAndWeight() {}
//		ChoiceAndWeight( float c, int w ) : choice( c ), weight( w ) {}
//
//		float choice;
//		int weight;
//	};
//
//
//	inline void AddChoiceWithWeight( const ChoiceAndWeight& choiceWithWeight );
//	inline void	AddChoiceWithWeight( float choice, int weight );
//	inline float	GetRandomValueFromChoices() const;
//
//private:
//
//	inline std::set<ErrorType> ValidateIsWeightedChoice( const std::string& asString );
//	inline std::set<ErrorType> BuildWeightedChoiceFromString( const std::string& asString, TypeEnum typeToParse );
//
//	int m_sumWeightings;
//	std::vector< ChoiceAndWeight > m_choicesWithWeightings;
//};
//
////-----------------------------------------------------------------------------------------------
//class WeightedChoiceVec2Int
//{
//public:
//
//	WeightedChoiceVec2Int( const std::string& asString );
//	WeightedChoiceVec2Int() : m_sumWeightings( 0 ) {}
//
//	struct ChoiceAndWeight
//	{
//		ChoiceAndWeight() {}
//		ChoiceAndWeight( float c, int w ) : choice( c ), weight( w ) {}
//
//		float choice;
//		int weight;
//	};
//
//
//	inline void AddChoiceWithWeight( const ChoiceAndWeight& choiceWithWeight );
//	inline void	AddChoiceWithWeight( float choice, int weight );
//	inline float	GetRandomValueFromChoices() const;
//
//private:
//
//	inline std::set<ErrorType> ValidateIsWeightedChoice( const std::string& asString );
//	inline std::set<ErrorType> BuildWeightedChoiceFromString( const std::string& asString, TypeEnum typeToParse );
//
//	int m_sumWeightings;
//	std::vector< ChoiceAndWeight > m_choicesWithWeightings;
//};
//
////-----------------------------------------------------------------------------------------------
//class WeightedChoiceVec2Float
//{
//public:
//
//	WeightedChoiceVec2Float( const std::string& asString );
//	WeightedChoiceVec2Float() : m_sumWeightings( 0 ) {}
//
//	struct ChoiceAndWeight
//	{
//		ChoiceAndWeight() {}
//		ChoiceAndWeight( float c, int w ) : choice( c ), weight( w ) {}
//
//		float choice;
//		int weight;
//	};
//
//
//	inline void AddChoiceWithWeight( const ChoiceAndWeight& choiceWithWeight );
//	inline void	AddChoiceWithWeight( float choice, int weight );
//	inline float	GetRandomValueFromChoices() const;
//
//private:
//
//	inline std::set<ErrorType> ValidateIsWeightedChoice( const std::string& asString );
//	inline std::set<ErrorType> BuildWeightedChoiceFromString( const std::string& asString, TypeEnum typeToParse );
//
//	int m_sumWeightings;
//	std::vector< ChoiceAndWeight > m_choicesWithWeightings;
//};