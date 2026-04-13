Math Visual Tests
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Math visualization sandbox for testing (and visually verifying)
geometric queries and other engine math functionality.

Game has multiple modes
	- Nearest Point on shapes 2D
	- Raycast vs disks 2D
	- Free roam with shapes 3D


CONTROLS / USAGE
~~~~~~~~~~~~~~~~

##########
# Global #
##########
P   : pauses the game 
O   : runs a single frame of the game
Y   : Speeds up the game
T   : slows down the game
R   : runs game at normal speed (1x)
V/B : zoom in and out of the center of the screen (only applicable in 2d modes)
Esc : Quit Game

F6/F7 : cycle through game modes


################################
# Nearest Point on shapes 2D   #
################################
- Move white dot with WASD
- Press F8 to re-randomize all shapes (note: shapes may end up on top of / inside each other).
- All shapes show nearest point to white dot.
- Shapes with interior areas (AABB, Disc, OBB, Capsule) light up when white dot is inside them.


#################################
# Raycast vs disks 2D           #
#################################
WASD : move start of raycast
IJKL : move end of raycast
F8   : Randomize all disks

A hit raycast should display the following:
- Impact point in cyan
- Impact normal in green at impact point
- raycast turns yellow when hitting a disk
- highlights hit disk


#################################
# Raycast vs AABB2              #
#################################
WASD : move start of raycast
IJKL : move end of raycast
F8   : Randomize all aabb2s

A hit raycast should display the following:
- Impact point in cyan
- Impact normal in green at impact point
- raycast turns yellow when hitting an aabb2
- highlights hit aabb2


#################################
# Raycast vs LineSegments2D     #
#################################
WASD : move start of raycast
IJKL : move end of raycast
F8   : Randomize all line segments

A hit raycast should display the following:
- Impact point in cyan
- Impact normal in green at impact point
- raycast turns yellow when hitting a line segment
- highlights hit line segment

#################################
# Billiard2D	                #
#################################
WASD : move start of raycast
IJKL : move end of raycast
F8   : Randomize all aabb2s
Space: Spawn billiard
F4   : Toggle physics controls

#################################
# Pachinko2D	                #
#################################
WASD : move start of raycast
IJKL : move end of raycast
F8   : Randomize all aabb2s
Space: Spawn billiard
Q/E  : Decrease/Increase billiard elasticity
F2   : Set billiard elasticity to 0.9
F3   : Clamp/Unclamp billiard elasticity
F4   : Toggle physics controls

##################################
# Shapes 3D                      #
##################################
WASD : move camera (moves only in xy plane)
QE   : move camera up and down (along world z axis)
Mouse: Rotate cam (yaw and pitch only)
F8   : Randomize all shapes
CTRL : Reset camera back to origin
Spacebar: Park/Unpark the reference point for nearest points and raycast
Left Mouse Button: Pickup/Drop the object that is being hit by the raycast, if any

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
	DebugRenderClear				: clears all debug rendering objects on screen
	DebugRenderToggleVisibility			: toggles the visibility of debug rendering
	DebugRenderSetTimeScale	scale=_float value_ 	: sets the debug rendering clock's time scale. If scale is not provided then it will set game time scale to 1.0
	SetZoomSpeed zoom=_float value_			: Sets the ortho camera zoom speed
	

KNOWN ISSUES
~~~~~~~~~~~~
none


