#ifndef GAME
#define GAME

//-----------------------------------------------------------------------------------------------
#include <vector>
#include "Engine/Utilities/InputHandler.hpp"
#include "Engine/Utilities/NamedProperties.hpp"
#include "Engine/External/Headers/fmod.hpp"
#include "Engine/Utilities/EventSystem.hpp"

#include "Camera3D.hpp"
#include "Client.hpp"


//-----------------------------------------------------------------------------------------------
class Clock;

//-----------------------------------------------------------------------------------------------
class Game
{
public:

	Game();
	~Game();

	void Initialize( HINSTANCE applicationInstanceHandle );
	void SetupCameraPositionAndOrientation( const Camera3D& camera ) const;
	void ResetCameraPositionAndOrientation() const;

	void InitializeLog();
	void Run();
	void Update();
	void Render();
	void ProcessInput();
	void UpdateConsoleLogOnInput();

	void		SetScreenResolutionEvent( NamedProperties& parameters );
	void		SetToServerEvent( NamedProperties& parameters ); 
	inline void RegisterForEvents();

	static void CreateSound( const char* songPath, FMOD::Sound* &sound );
	static void PlaySound( FMOD::Sound* &sound );
	static void PlayRandomSound( std::vector<FMOD::Sound*> &sounds );


	InputHandler	m_inputHandler;
	bool			m_isClient;

	Client			m_client;
	//Server			m_server;

	static FMOD::System *m_soundSystem;
	static Clock* s_appClock;

private:

	void RenderArena();
	void RenderWall( int wallCardinalDirection ); //0 - East, 1 - North, 2 - West, 3 - South
	void RenderFloor();
};


//-----------------------------------------------------------------------------------------------
inline void Game::RegisterForEvents()
{
	EventSystem& eventSystem = EventSystem::GetInstance();

	eventSystem.RegisterEventWithCallbackAndObject( "setRes", &Game::SetScreenResolutionEvent, this );
	eventSystem.RegisterEventWithCallbackAndObject( "server", &Game::SetToServerEvent, this );

	m_client.RegisterForEvents();
	//m_server.RegisterForEvents(); 
}

#endif