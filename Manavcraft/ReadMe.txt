A MINECRAFT CLONE

Note: Player movement variables (speed/mouse sensitivities) are exposed in the game config as well as have console commands for them and are configurable if need be.

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
# Play Mode: The game									#
#########################################################################################
Keyboard&Mouse/Gamepad Controls:
	P				: pauses the game 
	O				: runs a single frame of the game
	R				: slows down the game i.e. sets time scale to 0.2 (can be used along with the O key to run a single frame in slow motion)
	T				: returns the game to original speed i.e. sets time scale to 1.0 (can be used along with the O key to run a single frame in slow motion)
	Y				: speeds up the game i.e. sets time scale to 5.0 (can be used along with the O key to run a single frame while sped up)
	F1				: turns on debugger mode (not being used by the game)
	Esc/B				: go back to attract mode
	WASD/Leftjoystick 		: Move the player/cam in the player's/cam's forward(W), backward(S), right(D) and left(A) direction
	Mouse/RightJoystick 		: Rotate the player/cam around its yaw(horizontal axis) and pitch(vertical axis) 
	EQ/Gamepad Bumper 		: Move the player/cam in the absolute up(Q/Right Bumper) and down(E/Left Bumper) direction
	Spacebar/Left Joystick Button 	: Sprint (increase the movement speed of WASD/LeftJoystick)
	Ctrl/Y 				: Reset the player/cam at the game's origin (0,0,0)
	U				: Deactivate all chunks
	Left Mouse Click		: Dig the highlighted block
	Right Mouse Click		: Place a block on the side of the block that is highlighted
	1				: Set placement block to brick
	2				: Set placement block to glowstone
	R				: Lock/Unlock raycast
	M				: Dirty all meshes
	F2				: Cycle through camera modes
	F3				: Cycle through physics modes

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
	GameSetPlayerSpeed move=_float value_ 		: sets the player's WASD movement
		           sprint=_float value_ 	: sets the player's sprinting speed
		           mouse=_float value_ 		: sets the player's mouse sensitivity
	
	GameSetTimeScale scale=_float value_		: sets the game's time scale (overall game, not game state). If scale is not provided then it will set game time scale to 1.0
	DebugRenderClear				: clears all debug rendering objects on screen
	DebugRenderToggleVisibility			: toggles the visibility of debug rendering
	DebugRenderSetTimeScale	scale=_float value_ 	: sets the debug rendering clock's time scale. If scale is not provided then it will set game time scale to 1.0



/////////////////////
/Known Issues       /
/////////////////////
None