/////////////////////
/Objective          /
/////////////////////
Destroy all asteroids

/////////////////////
/How to Use         /
/////////////////////
To run the game, just open the .exe file in the Run folder and make sure it has the fmod dll files.

The game has two modes: Attract and Play Mode

Global Controls:
	Keyboard:
		F8 key: restarts the entire game from the attract mode

Attract Mode: Idle screen before the game starts and after the game ends (Player wins or loses)
Controls:
	Keyboard:
		Enter Key: Starts the game
		Esc Key: Quits the game
	X-box Controller:
		Start Button: Starts the game
		Left Bumber button: Quits the game

Play Mode: This mode contains the actual game with the player ship and asteroids. Asteroids are divided into waves. If you destroy all enemies in a wave
	   then a new wave will start. If you defeat all waves, then you win. If you exhaust all your lives then you lose. 
	   You, as the spaceship, have a total of 3 lives.
	   
	  On Game end (win/loss), the game will play the win/loss music and the game will remain idle for some seconds before transitioning back to the attract mode
Controls:
	Keyboard:
		A and D keys: rotate the ship in counter clockwise and clockwise directions respectively
		N key: respawn ship
		Space key: fire bullet
		Shift key: Slow
		Ctrl key: Enter Slo-mo
		P key: pauses the game
		O key: runs a single frame of the game
		T key(pressed down): slows down the game (can be used along with the O key to run a single frame in slow motion)
		F1 key: turns oun debugger mode
		Esc key: go back to attract mode
	Controller:
		Left Joystick: aim
		A button: fire bullet
		X button: slow down
		Left Bumber button: Enter Slo-mo
		Start button: respawn ship
		Select button: Go back to attract mode
		Y button: Pause game

Bonus Features:
	- You move using the bullet's recoil
	-Camera shakes when asteroids are hit and when bullets are fired
	-Firing a bullet applies some recoil to the ship
	-Enemies flash white for a single frame when hit
	-Player is immune and will flicker when spawning for some time
/////////////////////
/Known Issues       /
/////////////////////
The enemies sometimes don't flash white when hit with a bullet.

/////////////////////
/Deep Learning      /
/////////////////////

Insight: Simple code is better code
This may seem like a trite insight considering how oftem Prof. Squirrel has mentioned it in class but working on Starship made me realize what better code actually meant and why
simplicity lead to my code being better. Coming into guildhall, based on things other developers told me and things I read online, I thought having really structured code was
useful because it made it easier to look for things in the code and easy to modify it. If you have a function for each small action then editing that one action becomes easier 
and you can use it in many places. However, structure also meant being rigid and I hadn't really understood that until I programmed my EnemyWave class. 

When programming it, it seemed like a great idea and worth the excessive amount of time I spent creating individual functions for everything and shifting my enemies from the 
game class to the enemy wave class. However, when our assignments progressed and we had to add more features and test those features, the problem became abudantly clear.
Modifying the enemy class took time because now I had to travel to multiple places making edits. Structure made the code so rigid that I actively chose not to edit it further
lest I find myself spending too much time trying to fix whatever numerous bugs it ends up causing. Even for the gold assignment, I thought of making changes to the wave system 
but the rigidity made the task too daunting in comparison to other smaller changes that were easier to implement.

In comparison, once I understood this and started writing simpler code, iteration became much easier. In the time I would've spent fixing/editing my enemy class, I was able to add
multiple features to my game and the final improvement of the game was still quite satisfactory.

Better code is code that doesn't scare you when you want to modify it. Better code is simply simpler code.