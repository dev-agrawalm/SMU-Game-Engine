Doomenstein

/////////////////////
/How to Use         /
/////////////////////
To run the game, just open the .exe file in the Run folder and make sure it has the fmod dll files.

Global Keyboard Controls:
		F8 key: restarts the entire game from the attract mode (deletes existing game and creates a new one)

The game has two modes: Attract and Play Mode

#########################################################################################
# Attract Mode: Idle screen before the game starts					#
#########################################################################################
Keyboard/Gamepad Controls:
	Spacebar/Start	: Goes to play mode
	Esc/B		: Quits the game

#########################################################################################
# Game Over Mode: Screen when you win the game						#
#########################################################################################
Keyboard/Gamepad Controls:
	Esc/Controller Select/back	: Go to attract screen

#########################################################################################
# Game Won Mode: Screen when you lose the game						#
#########################################################################################
Keyboard/Gamepad Controls:
	Esc/Controller Select/back	: Go to attract screen

#########################################################################################
# Play Mode: The game									#
#########################################################################################
Game Mode Rules:
Demons will keep spawning at a fixed interval in the map
Kill the demons so that they don't reach a max count
If the number of demons increases more than the max count, then you lose
If the number of demons are within the max count for a fixed duration of the mode, then you win
There is a fixed respawn time, only after which can a dead player respawn
All parameters are defined in the game config xml

Keyboard&Mouse/Gamepad Controls:
	P				: pauses the game 
	O				: runs a single frame of the game
	R				: slows down the game i.e. sets time scale to 0.2 (can be used along with the O key to run a single frame in slow motion)
	T				: returns the game to original speed i.e. sets time scale to 1.0 (can be used along with the O key to run a single frame in slow motion)
	Y				: speeds up the game i.e. sets time scale to 5.0 (can be used along with the O key to run a single frame while sped up)
	F2				: toggles between clipping player controller to it's possessed actor vs not doing that
	Esc/Controller B		: go back to attract mode
	WASD/Leftjoystick 		: Move the player in the player's forward(W), backward(S), right(D) and left(A) direction
	Mouse/RightJoystick 		: Rotate the player around its yaw(horizontal axis) and pitch(vertical axis) 
	Mouse Left Click/Right Bumper	: Shoot forward
	Shift/Left Joystick Button	: Sprint (when player controller has possesses an actor
	1/Controller Y			: Equip Pistol
	2/Controller X			: Equip Plasma Rifle
	Spacebar/Controller A		: Respawn

Note: If mouse movement feels awkward then try adjusting the mouse sensitivity parameter in the game config. Same goes for the controller joysticks
#########################################################################################
# Editor Mode: During play mode								#
#########################################################################################
Keyboard/Mouse Controls:
	WASD/Mouse			: Move camera
	Shift				: Move faster
	Right Mouse Click		: Move possessed actor to the position that the camera is looking at if it is a valid floor tile
	F3				: Try to possess the actor looked at by the camera
	F4				: Mark the targeted actor as garbage
	QE/Gamepad Bumper 		: Move the cam in the absolute up(E/Right Bumper) and down(Q/Left Bumper) direction
	Shift/Left Joystick Button 	: Sprint (increase the movement speed of WASD/LeftJoystick)
	Ctrl/Y 				: Reset the cam at the game's origin (0,0,0)
	WASD/Leftjoystick 		: Move the cam in the cam's forward(W), backward(S), right(D) and left(A) direction
	Mouse/RightJoystick 		: Rotate the cam around its yaw(horizontal axis) and pitch(vertical axis)

##########################################################################################
# Dev Console									         #
##########################################################################################
Controls:
	~ 			: Show/Hide dev console
	Esc 			: Delete currently typed input / Close dev console
	Backspace 		: delete characters behind the cursor
	Delete 			: delete characters in front of the cursor
	Left/Right arrow 	: Move cursor left and right
	Home/End		: Moves cursor to the beginning/end of the currently typed input
	Up/Down arrow 		: traverse through previously entered commands/strings
	Tab 			: autocomplete your command if it matches an existing command
	Enter			: Execute typed command

Commands:
	Quit 						: Quits the game
	Help filter=_text_ 				: Lists out all the available events. If filter is provided then it will list out events containing the filter
	Clear 						: Clears the log	
	GameSetTimeScale scale=_float value_		: sets the game's time scale (overall game, not game state). If scale is not provided then it will set game time scale to 1.0
	DebugRenderClear				: clears all debug rendering objects on screen
	DebugRenderToggleVisibility			: toggles the visibility of debug rendering
	DebugRenderSetTimeScale	scale=_float value_ 	: sets the debug rendering clock's time scale. If scale is not provided then it will set game time scale to 1.0
	LoadMap map=_map name_				: If in attract mode, starts the game with the specified map. If in game, swaps out the current map for the specified map
	ToggleEditorMode				: Toggles bw enabling and disabling editor mode
	ToggleRaycastDebugging				: Toggles bw showing and hiding debugging artifacts for raycasts
	ToggleActorDebugging				: Toggles bw showing actor forward and collision cylinder
	ToggleAIMovement				: Toggles bw the AI moving and not moving. (AI will still animate and rotate, it just does all of that in one place)
	SpawnActor type=_type name_			: Spawns an actor of the given type if you're looking at the floor


/////////////////////
/Known Issues       /
/////////////////////
Game may not work well if you try to connect more than 2 players. The viewports may not be calculated correctly.
Spatial Audio is not implemented at all