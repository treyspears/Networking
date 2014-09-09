#ifndef GAME
#define GAME

//-----------------------------------------------------------------------------------------------
#include <vector>
#include "Engine/Utilities/InputHandler.hpp"
#include "Engine/Utilities/NamedProperties.hpp"
#include "Engine/External/Headers/fmod.hpp"
#include "Engine/Utilities/EventSystem.hpp"

//-----------------------------------------------------------------------------------------------
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
	void Render();
	void ProcessInput();
	void UpdateConsoleLogOnInput();
	void CreateOpenGLWindow( HINSTANCE applicationInstanceHandle );

	void UpdatePacketAndPotentiallySendToServer();
	void SendSimplePacketToServer();
	
	void SetPlayerParameters( NamedProperties& parameters );
	void SetServerIpFromParameters( NamedProperties& parameters );
	void SetServerPortFromParameters( NamedProperties& parameters );
	void UpdateDelayedPosition( NamedProperties& parameters );

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
	eventSystem.RegisterEventWithCallbackAndObject( "delayedInput", &Game::UpdateDelayedPosition, this );
}
#endif