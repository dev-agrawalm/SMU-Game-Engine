#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include <deque>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class App;
class Renderer;
class InputSystem;
class AudioSystem;
class Window;
class RandomNumberGenerator;
class GameLauncher;
class NetworkSystem;

enum SoundNames
{
	SFX_TABLE_HIT = 0,
	SFX_SCORED_A_GOAL,
	SFX_PADDLE_HIT_PUCK,
	SFX_PUCK_SLIDE_ON_TABLE,
	SFX_PUCK_RESET,
	MUSIC_GAME_BG_MUSIC,
	MUSIC_LAUNCHER_BG_MUSIC,
	NUM_SOUNDS
};

//GLOBAL VARIABLES
extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern Window* g_window;
extern RandomNumberGenerator* g_rng;
extern GameLauncher* g_gameLauncher;
extern NetworkSystem* g_networkSystem;
extern bool g_debugMode;
extern SoundID g_gameSounds[NUM_SOUNDS];
extern float g_gameSoundVolumes[NUM_SOUNDS];

//DEBUG RENDERING
void DrawLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color);
void DrawRing(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color);

void AddVertsForLine3D(std::vector<Vertex_PCU>& verts, Vec3 const& lineStart, Vec3 const& lineEnd, Rgba8 const& color, float thickness);
void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, Vec3 const& arrowStart, Vec3 const& arrowEnd, Rgba8 const& color, float thickness);
void AddVertsForDisk3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numSegments, Rgba8 const& color);
void AddVertsForNonUniformDisk3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius1, float radius2, float numSegments, Rgba8 const& color);

constexpr int NUM_VERTS_LINE = 6;
constexpr int NUM_VERTS_QUAD = 6;
constexpr int NUM_TRIANGLES_QUAD = 2;
constexpr int NUM_QUADS_RING = 32;
constexpr float DEBUG_LINE_WIDTH = 0.2f;

//GAME CONSTANTS
constexpr float TABLE_SURFACE_FRICTION = 0.25f;
constexpr float WALL_SURFACE_FRICTION = 0.1f;
constexpr float PADDLE_SURFACE_FRICTION = 0.05f;

constexpr float WALL_SURFACE_ELASTICITY = 0.85f;
constexpr float PADDLE_SURFACE_ELASTICITY = 0.95f;

constexpr float PADDLE_FORCE_IN_SAME_DIRECTION = 1.5f;
constexpr int NUM_PTS_FOR_VICTORY = 8;
//constexpr float PADDLE_FORCE = 10.0f;
//constexpr float MAX_ANGULAR_FORCE = 10.0f;

//CAMERA PERSPECTIVE CONSTANTS
constexpr float FOV_ANGLE_DEGREES = 60.0f;
constexpr float Z_NEAR = 0.1f;
constexpr float Z_FAR = 1000.0f;

class AirHockeyTable
{
public:
	AirHockeyTable() {};
	AirHockeyTable(float width, float height, Vec3 const& center) : m_width(width), m_height(height), m_center(center) {}

public:
	float m_width = 0.0f;
	float m_height = 0.0f;
	Vec3 m_center;
};


struct MouseInputData
{
public:
// 	Vec2 m_uiDragStart;
// 	Vec2 m_uiDragEnd;
// 	std::deque<Vec2> m_uiDragPositions;

	Vec3 m_worldDragStart;
	Vec3 m_worldDragEnd;
	std::deque<Vec3> m_worldDragPositions;
};


Vec3 GetCurrentMousePos_ScreenToWorld(Mat44 const& cameraModelMatrix);
Vec3 GetMousePos_ScreenToWorld(Mat44 const& cameraModelMatrix, IntVec2 const& refMousePos);


