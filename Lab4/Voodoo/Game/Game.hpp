#ifndef GAME
#define GAME

//-----------------------------------------------------------------------------------------------
#include <vector>
#include "Engine/Utilities/InputHandler.hpp"
#include "Engine/Utilities/NamedProperties.hpp"
#include "Engine/External/Headers/fmod.hpp"
#include "Engine/Utilities/EventSystem.hpp"

//-----------------------------------------------------------------------------------------------

struct playerID
{
	playerID()
		: r( 0 )
		, g( 0 )
		, b( 0 )

	{

	}
	playerID( unsigned char red, unsigned char green, unsigned char blue )
		: r( red )
		, g( green )
		, b( blue )
	{

	}

	bool operator==( const playerID& other )
	{
		return r == other.r && g == other.g && b == other.b;
	}

	unsigned char r;
	unsigned char g;
	unsigned char b;

};

struct player
{
	player()
		: id()
		, currentPos( Vector2f::Zero() )
		, currentVelocity( Vector2f::Zero() )
		, desiredPos( Vector2f::Zero() )
		, desiredVelocity( Vector2f::Zero() )
		, orientationAsDegrees( 0.f )
	{

	}

	void SeekTarget();

	playerID id;
	Vector2f currentPos;
	Vector2f currentVelocity;
	Vector2f desiredPos;
	Vector2f desiredVelocity;
	float orientationAsDegrees;
};

class Clock;

class Game
{
public:

	Game();
	~Game();

	inline void RegisterForEvents();
	void Initialize( HINSTANCE applicationInstanceHandle );
	void InitializeLog();
	void Run();
	void Update();
	void UpdateClients();
	void Render();
	void RenderPlayers();
	void RenderPlayer( player& playerToRender );
	void ProcessInput();
	void UpdateConsoleLogOnInput();
	void CreateOpenGLWindow( HINSTANCE applicationInstanceHandle );

	void PotentiallyRequestGameStartFromServer();
	void PotentiallySendUpdatePacketToServer();
	void CheckCollisionAndSendVictoryIfSuccessfulTag();
	
	void SetPlayerParameters( NamedProperties& parameters );
	void SetServerIpFromParameters( NamedProperties& parameters );
	void SetServerPortFromParameters( NamedProperties& parameters );
	//void UpdateDelayedPosition( NamedProperties& parameters );

	static void CreateSound( const char* songPath, FMOD::Sound* &sound );
	static void PlaySound( FMOD::Sound* &sound );
	static void PlayRandomSound( std::vector<FMOD::Sound*> &sounds );

	InputHandler	m_inputHandler;

	static FMOD::System *m_soundSystem;
	static Clock* s_appClock;
};

//-----------------------------------------------------------------------------------------------
void Game::RegisterForEvents()
{
	EventSystem& eventSystem = EventSystem::GetInstance();
	eventSystem.RegisterEventWithCallbackAndObject( "playerColor", &Game::SetPlayerParameters, this );
	eventSystem.RegisterEventWithCallbackAndObject( "ip", &Game::SetServerIpFromParameters, this );
	eventSystem.RegisterEventWithCallbackAndObject( "port", &Game::SetServerPortFromParameters, this );
	//eventSystem.RegisterEventWithCallbackAndObject( "delayedInput", &Game::UpdateDelayedPosition, this );
}
#endif