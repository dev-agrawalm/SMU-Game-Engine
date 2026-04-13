Galaga Bullet Hell with a spline editor

/////////////////////
/How to Use         /
/////////////////////
To run the game, just open the .exe file in the Run folder and make sure it has the fmod dll files.

The game has two modes: Attract and Play Mode

/////////////////////
/Global Controls    /
/////////////////////
	Keyboard:
		F8 key: restarts the entire game from the attract mode (deletes existing game and creates a new one)

Spline Editor: Spline Editor before the game starts
Controls:
	Keyboard:
		Enter Key: Starts the game
		Esc Key: Quits the game

Play Mode: The game
Controls:
	Keyboard:
		P key: pauses the game
		O key: runs a single frame of the game
		T key(pressed down): slows down the game (can be used along with the O key to run a single frame in slow motion)
		Y key(pressed down): speeds up the game	(can be used along with the O key to run a single frame while sped up)
		F1 key: turns on debugger mode
		Esc key: go back to spline editor
		WASD: Move the player (shown in blue)
		Spacebar: Fire bullet (shown in green)
		Q/E: Cycle through weapons

////////////////////////////
/Spline Editor Controls    /
////////////////////////////
Spacebar		: Add a curve at the end of the spline
Backspace		: Delete a curve at the end of the spline
Left Mouse Button	: Move the highlight control point on the spline (shown in red)
P			: Save the spline to an xml path

You can specify the path to write to in the game config

/////////////////////
/Known Issues       /
/////////////////////
Right now since you can't name your splines, all splines will be saved with the same name. You have to either manually rename the splines in the xml or have only one spline
otherwise the game will throw an error

/////////////////////
/Deep Learning      /
/////////////////////
