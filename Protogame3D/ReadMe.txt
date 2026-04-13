A BASIC GENERIC 3D GAME INTERFACE

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
	FV/Gamepad Bumper 		: Move the player/cam in the absolute up(F/Right Bumper) and down(V/Left Bumper) direction
	EQ/Gamepad Trigger 		: Roll the player around its forward direction in the right(E/Right trigger) and left(Q/Left Trigger) direction
	Shift/Left Joystick Button 	: Sprint (increase the movement speed of WASD/LeftJoystick)
	Ctrl/Y 				: Reset the player/cam at the game's origin (0,0,0)
	Left Mouse Button		: Spawns a wire sphere 2 units in front of the player
	Right Mouse button		: Spawns a wire cylinder at player position and spawns billboarded text that displays the player position at player position
	L				: Spawns a line from origin to player position
	B				: Spawns the player's basis at player position
	M				: Adds a message at the top left of the screen displaying player's euler angles

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
	GameSetPlayerSpeed move=_float value_ 		: sets the player's WASD movement speed
		           roll=_float value_ 		: sets the player's EQ rolling speed
		           sprint=_float value_ 	: sets the player's sprinting speed
		           rotate=_float value_ 	: sets the player's yaw and pitch rotating speed (only useful if using the gamepad)
	
	GameSetTimeScale scale=_float value_		: sets the game's time scale (overall game, not game state). If scale is not provided then it will set game time scale to 1.0
	DebugRenderClear				: clears all debug rendering objects on screen
	DebugRenderToggleVisibility			: toggles the visibility of debug rendering
	DebugRenderSetTimeScale	scale=_float value_ 	: sets the debug rendering clock's time scale. If scale is not provided then it will set game time scale to 1.0



/////////////////////
/Known Issues       /
/////////////////////
None