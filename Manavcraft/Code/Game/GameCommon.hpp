#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <thread>
#include <atomic>

class App;
class Renderer;
class InputSystem;
class AudioSystem;
class Window;
class RandomNumberGenerator;
class Game;
class World;

struct IntVec2;
struct WorldRaycastResult;

//GLOBAL VARIABLES
extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern Window* g_window;
extern RandomNumberGenerator* g_rng;
extern Game* g_game;
extern World* g_world;

//DEBUG RENDERING
void DrawLine2D(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color);
void DrawRing2D(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color);

extern std::atomic<int> g_isQuitting;

//CHUNK CONSTANTS
constexpr int CHUNK_BITS_X = 4;
constexpr int CHUNK_BITS_Y = 4;
constexpr int CHUNK_BITS_Z = 7;

constexpr int CHUNK_DIMS_X = 1 << CHUNK_BITS_X;
constexpr int CHUNK_DIMS_Y = 1 << CHUNK_BITS_Y;
constexpr int CHUNK_DIMS_Z = 1 << CHUNK_BITS_Z;

constexpr int CHUNK_BLOCK_COUNT_SINGLE_LAYER = CHUNK_DIMS_X * CHUNK_DIMS_Y;
constexpr int CHUNK_BLOCK_COUNT_TOTAL = CHUNK_DIMS_X * CHUNK_DIMS_Y * CHUNK_DIMS_Z;

constexpr int CHUNK_BITMASK_X = CHUNK_DIMS_X - 1;
constexpr int CHUNK_BITMASK_Y = (CHUNK_DIMS_Y - 1) << CHUNK_BITS_X;
constexpr int CHUNK_BITMASK_Z = (CHUNK_DIMS_Z - 1) << (CHUNK_BITS_X + CHUNK_BITS_Y);

constexpr int BLOCK_OUTDOOR_LIGHT_MASK = 15 << 4;
constexpr int BLOCK_INDOOR_LIGHT_MASK = 15;
constexpr int BLOCK_BITS_PER_LIGHT = 4;
constexpr int BLOCK_MAX_LIGHT_EXPOSURE = 15;

//global physics variables
extern float g_gravity;
extern float g_horizontalFriction;


enum Directions : int
{
	DIRECTION_NORTH,
	DIRECTION_SOUTH,
	DIRECTION_EAST,
	DIRECTION_WEST,
	DIRECTION_UP,
	DIRECTION_DOWN
};

constexpr int FILE_FORMAT_VERSION = 2;
constexpr int WORLD_SEED = 45;

//miscellaneous
bool operator<(IntVec2 const& a, IntVec2 const& b);
float GetMin(float a, float b, float c);
WorldRaycastResult GetClosestImpactRaycast(WorldRaycastResult* raycasts, int numRaycasts);
void VisualiseRaycast(WorldRaycastResult const& raycastToVisualise, float duration = 0.0f);


//threads
constexpr int NUM_CHUNKS_TO_CREATE_PER_FRAME = 4;
constexpr int NUM_CHUNKS_TO_ACTIVATE_PER_FRAME = 1;
constexpr int NUM_CHUNK_FOR_PERLIN_GENERATION_PER_FRAME = 3;
constexpr int NUM_CHUNKS_TO_SAVE_PER_FRAME = 5;
constexpr int NUM_SAVED_CHUNKS_TO_DEACTIVATE_PER_FRAME = 3;
constexpr int NUM_CHUNKS_TO_DEACTIVATE_PER_FRAME = 3;
constexpr int NUM_THREADS = 10;
extern std::thread* g_threads[NUM_THREADS];
