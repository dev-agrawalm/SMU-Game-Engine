A BASIC GENERIC GAME INTERFACE

/////////////////////
/How to Use         /
/////////////////////
To run the game, just open the .exe file in the Run folder and make sure it has the fmod dll files.

The game is a scene filled with convex objects

Global Controls:
	Keyboard:
		F8 key: restarts the entire game from the attract mode (deletes existing game and creates a new one)

Play Mode: The game
Controls:
	Keyboard:
		P key: pauses the game
		O key: runs a single frame of the game
		T key(pressed down): slows down the game (can be used along with the O key to run a single frame in slow motion)
		Y key(pressed down): speeds up the game	(can be used along with the O key to run a single frame while sped up)
		F1 key: view controls (Rest are detailed in game)
		Esc key: quit

/////////////////////
/Discoveries       /
/////////////////////
 - Without BSP i was able to do about 2^17 queries without the bounding disk check and 2^19 queries with the bounding disk check
 - With BSP i had many interesting observations. I did not using the bounding disk check because the it would make the raycasting faster by a neglegible amount
	- The scoring scheme for the partition matters. With my first implementation I was able to do 2^17 at best. With my second scheme I was able to do roughly 2^19 and with my third one
	  I was able to do roughly 2^20 - 2^21
	- At high number of objects, adding new partitions improved the speed dramatically in the beginning until it reached its fastest after which adding new partitions starts to slow 
	  down the raycasting after which the speed plateaus
	- I haven't experimented this properly but it seems that increasing the size of the play area reduces the performance of raycasting. I'm not sure if this is because of bsp in some way
	  or if this cause by the fact that the randomly generated raycasts are also longer as a consequence and so they just require more time
	- If the number of queries are really high (2^19-2^22) then having very small trees (depth of 1-3) can make raycasting really slow (300ms frames in one scenario) but still faster then
	  manually checking against each object

/////////////////////
/Known Issues       /
/////////////////////
Sometimes, while raycasting using BSP, its possible for the program to hang at big numbers (>=128 objects, >=1024 raycasts).

/////////////////////
/Deep Learning      /
/////////////////////
