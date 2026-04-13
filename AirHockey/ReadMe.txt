A BASIC GENERIC 3D GAME INTERFACE

/////////////////////
/How to Use         /
/////////////////////
To run the game, just open the .exe file in the Run folder and make sure it has the fmod dll files.

Global Keyboard Controls:
		F8 key: restarts the entire game from the attract mode (deletes existing game and creates a new one)

The game has two modes: Attract and Launcher

#########################################################################################
# Attract Mode: Idle screen before the game starts					#
#########################################################################################
Keyboard/Gamepad Controls:
	Spacebar/Start	: Goes to game launcher mode
	Esc/B		: Quits the game


#########################################################################################
# Launcher: The game									#
#########################################################################################
Keyboard&Mouse/Gamepad Controls:
	P				: pauses the game 
	O				: runs a single frame of the game
	F3				: turns on debugger mode
	Esc				: go back to attract mode
	S				: Opens/Closes server window to start a server
	C				: Opens/Closes client window to start a client and join a server
	Spacebar when game has not started : Once connected to another player, spacebar starts the game
	Mouse				: Move the puck around
	F9				: Show/Hide debug controls for puck
	Spacebar when game has started	: Reset position to center of table and reset angular and linear velocity to zero

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