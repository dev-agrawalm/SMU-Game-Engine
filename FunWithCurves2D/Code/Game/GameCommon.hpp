#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"

class App;
class Renderer;
class InputSystem;
class AudioSystem;
class Window;
class RandomNumberGenerator;
class Game;
class SpriteSheet;

//GLOBAL VARIABLES
extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern Window* g_window;
extern RandomNumberGenerator* g_rng;
extern Game* g_game;

//DEBUG RENDERING
void DrawLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color);
void DrawRing(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color);

extern SpriteSheet* g_gameSpriteSheet;

void ShakeScreenAndPlaySfx(float screenShakeMagnitude, float screenShakeDuration, std::string sfxPath, float sfxVolume, float sfxSpeed = 1.0f);