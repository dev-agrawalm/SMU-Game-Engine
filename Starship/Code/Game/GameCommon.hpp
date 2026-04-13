#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"

class App;
class Renderer;
class InputSystem;
class AudioSystem;
class Window;

//GLOBAL VARIABLES
extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern Window* g_window;

//GAME CONSTANTS
constexpr float WINDOW_ASPECT_RATIO = 2.0f;
constexpr float GAME_OVER_IDLE_TIME = 3.0f;

//ATTRACT MODE CONSTANTS
constexpr int ATTRACT_MODE_NUM_STARS = 50;
constexpr int NUM_STAR_TRIANGLES = 4;
constexpr float ATTRACT_MODE_SCREEN_SHAKE_MAGNITUDE = 5.0f;

//ENEMY WAVE CONSTANTS
constexpr int NUM_WAVES = 5;
// { asteroid count, wasp count, beetle count}
constexpr int WAVE_ONE_ENEMY_COUNT[3] = {5, 0, 0};
constexpr int WAVE_TWO_ENEMY_COUNT[3] = {10, 0, 0};
constexpr int WAVE_THREE_ENEMY_COUNT[3] = {12, 0, 0};
constexpr int WAVE_FOUR_ENEMY_COUNT[3] = {16, 0, 0};
constexpr int WAVE_FIVE_ENEMY_COUNT[3] = {20, 0, 0};
constexpr char const* WAVE_ONE_BG_MUSIC = "Data/Audio/CelesteMusic1.wav";
constexpr char const* WAVE_TWO_BG_MUSIC = "Data/Audio/CelesteMusic3.wav";
constexpr char const* WAVE_THREE_BG_MUSIC = "Data/Audio/CelesteMusic4.wav";
constexpr char const* WAVE_FOUR_BG_MUSIC = "Data/Audio/CelesteMusic5.wav";
constexpr char const* WAVE_FIVE_BG_MUSIC = "Data/Audio/CelesteMusic6.wav";
constexpr char const* ENEMY_WAVE_BG_MUSIC[NUM_WAVES] = {WAVE_ONE_BG_MUSIC, WAVE_TWO_BG_MUSIC, WAVE_THREE_BG_MUSIC, WAVE_FOUR_BG_MUSIC, WAVE_FIVE_BG_MUSIC};

//WORLD CONSTANTS
constexpr float WORLD_SIZE_X = 200.0f;
constexpr float WORLD_SIZE_Y = 100.0f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.0f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.0f;

//UI CANVAS CONSTANTS
constexpr float UI_CANVAS_SIZE_X = 1600.0f;
constexpr float UI_CANVAS_SIZE_Y = 800.0f;
constexpr float UI_CANVAS_CENTER_X = UI_CANVAS_SIZE_X / 2.0f;
constexpr float UI_CANVAS_CENTER_Y = UI_CANVAS_SIZE_Y / 2.0f;

//SHIP CONSTANTS
constexpr int PLAYER_SHIP_LIVES = 4;
constexpr int NUM_SHIP_VERTS = 15;
constexpr int PLAYER_SHIP_NUM_BOMBS = 30;
constexpr float PLAYER_SHIP_STARTING_X = 100.0f;
constexpr float PLAYER_SHIP_STARTING_Y = 50.0f;
constexpr float PLAYER_SHIP_ACCELRATION = 80.0f;
constexpr float PLAYER_SHIP_MAX_SPEED = 100.0f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.0f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr float PLAYER_SHIP_THRUST_FLAME_MAX_LENGTH = 3.0f;
constexpr float PLAYER_SHIP_FIRE_RATE = 0.30f;
constexpr float PLAYER_SHIP_DEATH_SCREEN_SHAKE_MAGNITUDE = 1.3f;
constexpr float PLAYER_SHIP_DEATH_SCREEN_SHAKE_DURATION = 1.0f;
constexpr float PLAYER_SHIP_INVINCIBILITY_TIME = 1.5f;

//ASTEROID CONSTANTS
constexpr int NUM_STARTING_ASTEROIDS = 6;
constexpr int MAX_ASTEROIDS = 12;
constexpr int ASTEROID_HEALTH = 1;
constexpr int NUM_ASTEROID_VERTS = 48;
constexpr float ASTEROID_SPEED = 17.0f;
constexpr float MAX_ASTEROID_ANGULAR_VELOCITY = 200.0f;
constexpr float MIN_ASTEROID_ANGULAR_VELOCITY = -200.0f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.60f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;

//BULLET CONSTANTS
constexpr int MAX_BULLETS = 200000;
constexpr int BULLET_DAMAGE = 3;
constexpr int NUM_BULLET_VERTS = 6;
constexpr float BULLET_LIFETIME_SECONDS = 7.0f;
constexpr float BULLET_SPEED = 50.0f;
constexpr float BULLET_PHYSICS_RADIUS = 1.5f;
constexpr float BULLET_COSMETIC_RADIUS = 4.0f;
constexpr float BULLET_RECOIL = 10.0f;
constexpr float BULLET_SPAWN_SCREEN_SHAKE_MAGNITUDE = 0.6f;
constexpr float BULLET_SPAWN_SCREEN_SHAKE_DURATION = 0.3f;
constexpr float BULLET_IMPACT_SCREEN_SHAKE_MAGNITUDE = 1.0f;
constexpr float BULLET_IMPACT_SCREEN_SHAKE_DURATION = 0.3f;

//WASP CONSTANTS
constexpr int NUM_WASP_VERTS = 12;
constexpr int WASP_HEALTH = 12;
constexpr float WASP_PHYSICS_RADIUS = 2.1f;
constexpr float WASP_COSMETIC_RADIUS = 2.3f;
constexpr float WASP_ACCELERATION = 60.0f;
constexpr float WASP_MAX_SPEED = 40.0f;
constexpr float WASP_SCALE = 1.5f;

//BEETLE CONSTANTS
constexpr int NUM_BEETLE_VERTS = 21;
constexpr int BEETLE_HEALTH = 10;
constexpr float BEETLE_PHYSICS_RADIUS = 1.4f;
constexpr float BEETLE_COSMETIC_RADIUS = 1.6f;
constexpr float BEETLE_SPEED = 15.0f;
constexpr float BEETLE_SCALE = 2.0f;

//DEBRIS CONSTANTS
constexpr int NUM_DEBRIS_VERTS = 30;
constexpr int DEBRIS_COUNT_SPACESHIP = 20;
constexpr int DEBRIS_COUNT_ASTEROID = 12;
constexpr int DEBRIS_COUNT_BULLET = 2;
constexpr int DEBRIS_COUNT_WASP = 5;
constexpr int DEBRIS_COUNT_BEETLE = 5;
constexpr int MAX_DEBRIS = 5000;
constexpr float DEBRIS_MIN_SPEED = 4.0f;
constexpr float DEBRIS_MIN_COSMETIC_RADIUS = 0.05f;
constexpr float DEBRIS_FADE_OUT_TIME = 2.0f;
constexpr float DEBRIS_MAX_ALPHA = 127.0f;
constexpr float DEBRIS_MAX_ANGULAR_VELOCITY = 250.0f;

//BOMB CONSTANTS
constexpr int NUM_BOMB_QUADS = 8;
constexpr int BOMB_DAMAGE = 1;
constexpr float BOMB_WAIT_TIMER_SECONDS = 2.0f;
constexpr float BOMB_EXPLOSION_DURATION = 1.0f;
constexpr float BOMB_COSMETIC_RADIUS = 1.05f;
constexpr float BOMB_EXPLOSION_RADIUS = BOMB_COSMETIC_RADIUS * 18.0f;
constexpr float BOMB_EXPLOSION_SCREEN_SHAKE_MAGNITUDE = 3.0f;
constexpr float BOMB_EXPLOSION_SCREEN_SHAKE_DURATION = BOMB_EXPLOSION_DURATION;

//DEBUG RENDERING
void DebugDrawLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color);
void DebugDrawRing(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color);
void DrawDottedLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color, int numDots);

constexpr int NUM_LINE_VERTS = 6;
constexpr int NUM_QUAD_VERTS = 6;
constexpr int NUM_TRIANGLES_QUAD = 2;
constexpr int NUM_RING_QUADS = 32;
constexpr float DEBUG_LINE_WIDTH = 0.2f;