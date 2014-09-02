//-----------------------------------------------------------------------------------------------
First person camera system with constraints.

How to run:
	First, extract project to the location you desire.
	Then, you may run the executable by launching Airboat.exe, found in the Air Boat/Run_Windows_32 folder.
	
	Optionally, you may open the VS2010 solution and Select Build -> Build Solution to build an executable.
	The built executable will be found in the Air Boat/Run_Windows_32 folder.
	The application is unstable in Debug. Building the solution in Release mode will give optimal performance.

Controls:
	Esc		- exit the application.
	W,A,S,D - moves the player forward, left, right, and backwards, respectively.
	Space   - jump, when in walk mode.
	Q,E	    - pitch up/down while in flying/no clip mode.
	Mouse   - move to rotate camera.

	F1      - switch to walk mode, if not already in it.
	F2		- switch to fly mode, if not already in it.
	F3		- switch to no-clip mode, if not already in it.
	F4		- toggle on/off render camera debug information.

	R		- reload XML definitions. Explained below.

XML Camera Definitions
	This file allows one to edit constraints and properties on cameras without having to exit game and rebuild solution.
	
	CameraDefinitions.xml is located in Air Boat/Data/CameraData folder, or Air Boat/Run_Windows_32/Data/CameraData folder.
	Edit the file located in Air Boat/Run_Windows_32/Data/CameraData folder if you wish to make edits to the pre-built exe.
	Edit the file located in Air Boat/Data/CameraData folder if you plan on re-building the solution. The file will be copied to the Run_Windows_32/Data/CameraData folder when built.
	
	Make changes to the values of the various named variables to change the behavior of the ingame cameras.
	If edits are made while running the application. Save the file and simply press 'R' while in game to reload the new definitions.
	