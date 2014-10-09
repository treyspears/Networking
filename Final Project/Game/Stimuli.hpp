#ifndef STIMULI
#define STIMULI

#include <string>

#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Primitives/Color.hpp"

//think about implementing string hashing here

class Stimuli
{
public:

	enum TypeOfStimuli
	{
		STIMULI_NONE,
		STIMULI_ZEBRA,
		STIMULI_LION,
		STIMULI_SIGHT,
		STIMULI_SOUND,
		STIMULI_GOAL,

		NUM_TYPE_OF_STIMULI,
		STIMULI_ALL
	};

	Stimuli( TypeOfStimuli stimuliType, const Vector3f& position, const Color& debugRenderColor );

	static TypeOfStimuli GetTypeForStimuliName( const std::string& stimuliName );

	void Render() const;
	void Update();

	TypeOfStimuli	m_type;
	Vector3f		m_position;
	Color			m_debugRenderColor;
};

#endif