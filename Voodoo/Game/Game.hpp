#ifndef GAME
#define GAME

//-----------------------------------------------------------------------------------------------
#include <vector>
#include "Engine/Utilities/InputHandler.hpp"
#include "Engine/Utilities/NamedProperties.hpp"
#include "Engine/External/Headers/fmod.hpp"

//-----------------------------------------------------------------------------------------------
class Clock;

class Game
{
public:

	Game();
	~Game();

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

	static void CreateSound( const char* songPath, FMOD::Sound* &sound );
	static void PlaySound( FMOD::Sound* &sound );
	static void PlayRandomSound( std::vector<FMOD::Sound*> &sounds );

	InputHandler	m_inputHandler;

	static FMOD::System *m_soundSystem;
	static Clock* s_appClock;
};

#endif