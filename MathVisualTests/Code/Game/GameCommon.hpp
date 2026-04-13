#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"

class App;
class Renderer;
class InputSystem;
class Window;
class RandomNumberGenerator;
class Game;

//GLOBAL VARIABLES
extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_inputSystem;
extern Window* g_window;
extern RandomNumberGenerator* g_rng;
extern Game* g_game;

//DEBUG RENDERING
void DrawLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color);
void DrawRing(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color);

constexpr int NUM_VERTS_LINE = 6;
constexpr int NUM_VERTS_QUAD = 6;
constexpr int NUM_TRIANGLES_QUAD = 2;
constexpr int NUM_QUADS_RING = 32;
constexpr float DEBUG_LINE_WIDTH = 0.2f;