#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

class App;
class Renderer;
class InputSystem;
class AudioSystem;
class Window;
class RandomNumberGenerator;
class Game;
class BitmapFont;

//GLOBAL VARIABLES
extern App*						g_theApp;
extern Renderer*				g_theRenderer;
extern InputSystem*				g_inputSystem;
extern AudioSystem*				g_audioSystem;
extern Window*					g_window;
extern Game*					g_game;
extern RandomNumberGenerator*	g_rng;
extern BitmapFont*				g_bitmapFont;
extern bool						g_debugMode;

IntVec2 const NORTH		 = IntVec2(0, 1);
IntVec2 const SOUTH		 = IntVec2(0, -1);
IntVec2 const EAST		 = IntVec2(1, 0);
IntVec2 const WEST		 = IntVec2(-1, 0);
IntVec2 const NORTH_WEST = IntVec2(-1, 1);
IntVec2 const NORTH_EAST = IntVec2(1, 1);
IntVec2 const SOUTH_EAST = IntVec2(1, -1);
IntVec2 const SOUTH_WEST = IntVec2(-1, -1);

constexpr unsigned int	NUM_VERTS_LINE		= 6;
constexpr unsigned int	NUM_VERTS_QUAD		= 6;
constexpr unsigned int	NUM_TRIANGLES_QUAD	= 2;
constexpr unsigned int	NUM_QUADS_RING		= 32;
constexpr float			DEFAULT_LINE_WIDTH	= 0.02f;

//DEBUG RENDERING
void DrawLine(Vec2 const& startPos, Vec2 const& line, Rgba8 const& color, float width = DEFAULT_LINE_WIDTH);
void DrawRing(Vec2 const& centerPosition, float radius, Rgba8 const& color, float width = DEFAULT_LINE_WIDTH);
