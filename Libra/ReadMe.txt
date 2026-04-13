Libra - SD1 2nd Game Project

/////////////////////
/How to Use         /
/////////////////////
To run the game, just open the .exe file in the Run folder and make sure it has the fmod dll files.

The game contains a procedurally generated map.
The game has four modes: Attract, Play Mode, GameWon, GameOver

Global Controls:
	Keyboard:
		F8 key: 	restarts the entire game from the attract mode (deletes existing game and creates a new one)
		M key: 		mute audio
		N key: 		unmute audio

Attract Mode: Idle screen before the game starts
Controls:
	Keyboard:
		P Key: 		Starts the game
		Esc Key: 	Quits the game
	XBox Controller:
		Start Button: 	Starts the game
		Back Button: 	Quits the game

Play Mode: The game
Controls:
	Keyboard:
		P key: 				toggles the paused state of the game
		O key: 				runs a single frame of the game
		T key(pressed down): 		slows down the game (can be used along with the O key to run a single frame in slow motion)
		Y key(pressed down): 		speeds up the game (can be used along with the O key to run a single frame while sped up)
		F1 key: 			turns on debugger mode
		F3 key: 			Disables collisions for the player
		F4 Key:				Toggle between game camera and debug camera (debug camera zooms out to show the entire map)
		F6 key:				Toggle god mode for the player (take no damage during god mode)
		Esc key while paused: 		go back to attract mode
		Esc key while not paused: 	pause the game
		E,S,D,F Keys: 			control the tank (E = North, S = West, D = South, F = East)
		I,J,K,L Keys: 			control the turret (I = North, J = West, K = South, L = East)
		R key: 				Generate a new grid without creating a new map object or deleting the existing game 
		Space Key:			Fire bullet
	Controller:
		Back Button while paused: 	go back to attract mode
		Back Button while not paused: 	pause the game
		Start Button: 			toggles the paused state of the game
		Left Joystick: 			control the tank
		Right Joystick: 		control the turret
		Right Trigger:			Fire bullet

GameWon Mode: Idle screen after you win the game
Controls:
	Keyboard:
		P Key: 		Goes back to attract mode
		Esc Key: 	Goes back to attract mode
	XBox Controller:
		Start Button: 	Goes back to attract mode
		Back Button: 	Goes back to attract mode

GameOver Mode: Idle screen after you die
Controls:
	Keyboard:
		P Key: 		Respawns the ship and starts the game from the start of the map in which the player died
		Esc Key: 	Goes back to attract mode
	XBox Controller:
		Start Button: 	Respawns the ship and starts the game from the start of the map in which the player died
		Back Button: 	Goes back to attract mode


Notes:
	- If a bullet is fired inside a solid tile then the bullet will die immediately
	- If a bullet is fired inside an enemy then it will cause damage to the enemy
	- The guided missile will be locked onto a single target and it will slowly rotate towards the target if the target is in the bullet's line of sight in any direction. If the target is not in the line of sight then it
	  will continue moving forward in a straight line
	- Sagittarius AI - the sagittarius tank will behave similar to leo but its different in two distinct ways
		- First, it does not try to go to the exact spot of the player. Instead it will stop once its at a fixed distance away from the player in any direction.
		  It only starts moving/rotating in the player's direction once the player is further than the fixed distance
		- Second, it does not aim to hit the player with its bullet. Instead it aims slightly ahead of where the player intends to go (the player's forward direction) and it
		  will fire bullets in that direction. The purpose is to stop the player's movement using bullets.


/////////////////////
/Known Issues       /
/////////////////////
Part of the tire for the tank texture can go through the walls because its outside the physical radius of the tank. The physics radius can be increased to fix this but then the tank might collide with the all before the texture
touches the wall.

Once, after finishing the game, I pressed P from the GameWon screen and it took me to the attract screen. When I pressed P again to start a new game it gave me an error
where somewhere in the code I'm using an index for a vector thats out of its range. This, however, only happened once and the steps to replicate it are unknown as of right now.

/////////////////////
/Deep Learning      /
/////////////////////
Variable names should be meaningful. When I was programming my turret movement, I had initially got my local and world orientations mixed up in my variable names because
I didn't really spend time to think of what local orientation would be and what world orientation would be. This led to a lot of confusion and trial-and-error to get my turret
movement working because I was using the variables incorrectly. When I did infact step back, think of my variables and how they're changing, and rename my variables to what they
represet, it became significantly easier to implement the functionality. So variable names should be meaningful even if you're just testing because they reduce the amount of time
spent tweaking formulas and equations and hoping you got the right one.

Break down your work into small chunks. As small as possible. For libra, for a couple of assignments I've been working on features but I only test/submit my work after its completely finished. This means that hours worth of work goes
untested and is submitted into one large chunk. Another example of this is when refactoring Libra to use xml files to read data instead of in-game constants, I decided to refactor EVERY class and script where any in-game constant was
being used instead of doing it like a normal person and checking the work class by class. So far this has not been a bad strategy because the code ended up working pretty well. However, this is just a mistake waiting to happen and 
I realised this while refactoring the code to use xml data. If there is any mistake then searching for it becomes a time consuming process. Testing becomes a time consuming process because now you have to make sure you test 
every thing at once and if you miss anything then that can lead to time-consuming consequences. This also makes it harder to estimate work because you end up estimating the time it takes to implement every aspect of the feature without
learning the time it takes for every atomic piece of work. All in all, it makes the coding process riskier and harder to roll back changes if ever necessary. So work in smaller chunks.