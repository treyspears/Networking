#ifndef GAME
#define GAME

//-----------------------------------------------------------------------------------------------
#include <vector>
#include "Engine/Utilities/InputHandler.hpp"
#include "Engine/Utilities/NamedProperties.hpp"
#include "Engine/External/Headers/fmod.hpp"
#include "Engine/Utilities/EventSystem.hpp"


#include "Client.hpp"
#include "Server.hpp"


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

	static void CreateSound( const char* songPath, FMOD::Sound* &sound );
	static void PlaySound( FMOD::Sound* &sound );
	static void PlayRandomSound( std::vector<FMOD::Sound*> &sounds );

	InputHandler	m_inputHandler;
	//Client			m_client;
	Server			m_server;

	static FMOD::System *m_soundSystem;
	static Clock* s_appClock;
};

inline void Game::RegisterForEvents()
{
	//m_client.RegisterForEvents();
}

#endif