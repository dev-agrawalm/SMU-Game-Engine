#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine\Renderer\SpriteAnimDefinition.hpp"
#include "Engine\Math\IntVec2.hpp"

class App;
class Renderer;
class InputSystem;
class AudioSystem;
class Window;
class UISystem;
class RandomNumberGenerator;
class SpriteSheet;
class Texture;
class SpriteDefinition;
class BitmapFont;
class LevelEditor;

struct IntVec2;
struct EntitySpawnInfo;
struct LevelRaycastResult;
struct Tile;
struct AABB2;
enum EntityType : int;

//GLOBAL VARIABLES
extern App* g_app;
extern Renderer* g_theRenderer;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern Window* g_window;
extern UISystem* g_uiSystem;
extern LevelEditor* g_levelEditor;
extern RandomNumberGenerator* g_rng;
extern Game* g_game;
// extern SpriteSheet* g_spriteSheet_128x128;
// extern SpriteSheet* g_spriteSheet_128x64;
// extern SpriteSheet* g_spriteSheet_64x64;
// extern SpriteSheet* g_spriteSheet_32x32;
// extern SpriteSheet* g_spriteSheet_16x16;
extern Texture* g_spriteSheetTexture;
extern BitmapFont* g_bitmapFont;
extern unsigned int g_currentFrameNumber;
extern bool g_muteSfx;
extern float g_editorMusicVolume;
extern float g_gameMusicVolume;

//DEBUG RENDERING
void DebugDrawLine(Vec2 const& startPos, Vec2 const& line, float width, Rgba8 const& color);
void DebugDrawRing(Vec2 const& centerPosition, float radius, float width, Rgba8 const& color);
void DebugDrawRaycast(LevelRaycastResult const& raycast);
void DebugDrawAABB2Outline(AABB2 const& aabb2, float width, Rgba8 const& color);

//Entity Utilities
//AABB2 GetPhysicsBoundsForEntity(EntityType entityType, Vec2 const& entityPos = Vec2::ZERO);		// if position isn't provided then returned bounds can be treated as local bounds
//AABB2 GetCosmeticBoundsForEntity(EntityType entityType, Vec2 const& entityPos = Vec2::ZERO);	// if position isn't provided then returned bounds can be treated as local bounds
//SpriteSheet* GetSpriteSheetForEntity(EntityType entityType);
//IntVec2 GetEditorSpriteCoordsForEntity(EntityType entityType);
//SpriteDefinition const* GetSpriteDefinitionForEntity(EntityType entityType);


int GetSpriteIndex(IntVec2 const& spriteCoords, IntVec2 const& spriteSheetGridLayout);

//MISC
bool operator==(EntitySpawnInfo const& a, EntitySpawnInfo const& b);
bool operator==(Tile const& a, Tile const& b);


constexpr float EDITOR_MUSIC_VOLUME = 1.0f;
constexpr float GAME_MUSIC_VOLUME = 1.0f;

//LEVEL CONSTANTS
constexpr int LEVEL_X_BITS = 7;
constexpr int LEVEL_Y_BITS = 6;

constexpr int LEVEL_SIZE_X = 1 << LEVEL_X_BITS;
constexpr int LEVEL_SIZE_Y = 1 << LEVEL_Y_BITS;
constexpr int LEVEL_TILE_COUNT = LEVEL_SIZE_X * LEVEL_SIZE_Y;

constexpr int LEVEL_INDEX_X_MASK = LEVEL_SIZE_X - 1;
constexpr int LEVEL_INDEX_Y_MASK = (LEVEL_SIZE_Y - 1) << LEVEL_X_BITS;

//PHYSICS CONSTANTS
constexpr float GRAVITY = 35.0f;
constexpr float GROUND_FRICTION = 12.0f;
constexpr float AIR_FRICTION = 10.0f;
constexpr float WAYPOINT_TRAVEL_SPEED = 3.0f;

//PLAYER CONSTANTS
constexpr float PLAYER_BIG_COSMETIC_HEIGHT = 2.0f;
constexpr float PLAYER_BIG_COSMETIC_WIDTH = 1.0f;
constexpr float PLAYER_BIG_PHYSICS_HEIGHT = 1.75f;
constexpr float PLAYER_BIG_PHYSICS_WIDTH = 0.8f;

constexpr float PLAYER_SMALL_COSMETIC_HEIGHT = 2.0f;
constexpr float PLAYER_SMALL_COSMETIC_WIDTH = 1.0f;
constexpr float PLAYER_SMALL_PHYSICS_HEIGHT = 1.25f;
constexpr float PLAYER_SMALL_PHYSICS_WIDTH = 0.65f;

constexpr float PLAYER_WALKING_ACCELERATION = 100.0f;
constexpr float PLAYER_SPRINTING_ACCELERATION = 150.0f;
constexpr float PLAYER_JUMP_IMPULSE_VELOCITY = 10.0f;
constexpr float PLAYER_JUMP_SUSTAIN_THRUST = 100.0f;
constexpr float PLAYER_BOUNCE_IMPULSE_VELOCITY = PLAYER_JUMP_IMPULSE_VELOCITY * 1.1f;

//GOOMA CONSTANTS
constexpr float GOOMBA_COSMETIC_HEIGHT = 1.0f;
constexpr float GOOMBA_COSMETIC_WIDTH = 1.0f;
constexpr float GOOMBA_PHYSICS_HEIGHT = 0.9f;
constexpr float GOOMBA_PHYSICS_WIDTH = 0.9f;
constexpr float GOOMBA_SPEED = 1.0f;

//TURTLE CONSTANTS
constexpr float TURTLE_COSMETIC_HEIGHT = 2.0f;
constexpr float TURTLE_COSMETIC_WIDTH = 1.0f;
constexpr float TURTLE_PHYSICS_HEIGHT = 1.6f;
constexpr float TURTLE_PHYSICS_WIDTH = 0.9f;
constexpr float TURTLE_SPEED = 2.0f;

//TURTLE SHELL
constexpr float GREEN_SHELL_COSMETIC_HEIGHT = 1.0f;
constexpr float GREEN_SHELL_COSMETIC_WIDTH = 1.0f;
constexpr float GREEN_SHELL_PHYSICS_HEIGHT = 0.9f;
constexpr float GREEN_SHELL_PHYSICS_WIDTH = 0.9f;
constexpr float GREEN_SHELL_SPEED = 15.0f;

//RED MUSHROOM
constexpr float RED_MUSHROOM_COSMETIC_HEIGHT = 1.0f;
constexpr float RED_MUSHROOM_COSMETIC_WIDTH = 1.0f;
constexpr float RED_MUSHROOM_PHYSICS_HEIGHT = 1.2f;
constexpr float RED_MUSHROOM_PHYSICS_WIDTH = 1.2f;
constexpr float RED_MUSHROOM_SPEED = 5.0f;

//COIN CONSTANTS
constexpr float COIN_COSMETIC_HEIGHT = 1.0f;
constexpr float COIN_COSMETIC_WIDTH = 0.8f;
constexpr float COIN_PHYSICS_HEIGHT = 1.0f;
constexpr float COIN_PHYSICS_WIDTH = 1.0f;

//LEVEL END FLAG CONSTANTS
constexpr float FLAG_COSMETIC_HEIGHT	= 4.0f;
constexpr float FLAG_COSMETIC_WIDTH		= 4.0f;
constexpr float FLAG_PHYSICS_HEIGHT		= 3.0f;
constexpr float FLAG_PHYSICS_WIDTH		= 2.0f;

//ON/OFF SWITCH CONSTANTS
constexpr float SWITCH_COSMETIC_HEIGHT	= 1.0f;
constexpr float SWITCH_COSMETIC_WIDTH	= 1.0f;
constexpr float SWITCH_PHYSICS_HEIGHT	= 1.0f;
constexpr float SWITCH_PHYSICS_WIDTH	= 1.0f;

//Red block CONSTANTS
constexpr float RED_BLOCK_COSMETIC_HEIGHT	= 1.0f;
constexpr float RED_BLOCK_COSMETIC_WIDTH	= 1.0f;
constexpr float RED_BLOCK_PHYSICS_HEIGHT	= 1.0f;
constexpr float RED_BLOCK_PHYSICS_WIDTH		= 1.0f;

//blue block CONSTANTS
constexpr float BLUE_BLOCK_COSMETIC_HEIGHT	= 1.0f;
constexpr float BLUE_BLOCK_COSMETIC_WIDTH	= 1.0f;
constexpr float BLUE_BLOCK_PHYSICS_HEIGHT	= 1.0f;
constexpr float BLUE_BLOCK_PHYSICS_WIDTH	= 1.0f;

//breakable block constants
constexpr float BREAK_BLOCK_COSMETIC_HEIGHT = 1.0f;
constexpr float BREAK_BLOCK_COSMETIC_WIDTH = 1.0f;
constexpr float BREAK_BLOCK_PHYSICS_HEIGHT = 1.0f;
constexpr float BREAK_BLOCK_PHYSICS_WIDTH = 1.0f;

//flippable block constants
constexpr float FLIP_BLOCK_COSMETIC_HEIGHT = 1.0f;
constexpr float FLIP_BLOCK_COSMETIC_WIDTH = 1.0f;
constexpr float FLIP_BLOCK_PHYSICS_HEIGHT = 1.0f;
constexpr float FLIP_BLOCK_PHYSICS_WIDTH = 1.0f;

//way point constants
constexpr float WAYPOINT_COSMETIC_HEIGHT = 1.0f;
constexpr float WAYPOINT_COSMETIC_WIDTH = 1.0f;
constexpr float WAYPOINT_PHYSICS_HEIGHT = 1.0f;
constexpr float WAYPOINT_PHYSICS_WIDTH = 1.0f;

//bowser constants
constexpr float BOWSER_COSMETIC_HEIGHT = 4.0f;
constexpr float BOWSER_COSMETIC_WIDTH = 4.0f;
constexpr float BOWSER_PHYSICS_HEIGHT = 2.2f;
constexpr float BOWSER_PHYSICS_WIDTH = 2.0f;
constexpr float BOWSER_SPEED = 0.5f;

//fireball constants
constexpr float FIREBALL_COSMETIC_HEIGHT = 1.0f;
constexpr float FIREBALL_COSMETIC_WIDTH = 2.0f;
constexpr float FIREBALL_PHYSICS_HEIGHT = 0.8f;
constexpr float FIREBALL_PHYSICS_WIDTH = 1.75f;
constexpr float FIREBALL_SPEED = 4.0f;

enum EntityType : int
{
	ENTITY_TYPE_NONE = -1,
	ENTITY_TYPE_MARIO,
	ENTITY_TYPE_GOOMBA,
	ENTITY_TYPE_COIN,
	ENTITY_TYPE_LEVEL_END,
	ENTITY_TYPE_SWITCH_BLOCK,
	ENTITY_TYPE_RED_BLOCK,
	ENTITY_TYPE_BLUE_BLOCK,
	ENTITY_TYPE_BREAKABLE_BLOCK,
	ENTITY_TYPE_FLIPPABLE_BLOCK,
	ENTITY_TYPE_KOOPA_TROOPA,
	ENTITY_TYPE_TURTLE_SHELL,
	ENTITY_TYPE_RED_MUSHROOM,
	ENTITY_TYPE_WAYPOINT,
	ENTITY_TYPE_SMALL_MARIO,
	ENTITY_TYPE_BOWSER,
	ENTITY_TYPE_FIREBALL,
	ENTITY_TYPE_COUNT
};

enum EntityFacingDirection
{
	ENTITY_FACING_LEFT = -1,
	ENTITY_FACING_RIGHT = 1
};

enum EntityPhysicsOptions : uint32_t
{
	ENTITY_OPTIONS_NONE							= 0,
	ENTITY_OPTIONS_CAN_BE_STEPPED_ON			= 1 << 0,
	ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT		= 1 << 1,
	ENTITY_OPTIONS_COLLIDES_WHILE_DEAD			= 1 << 2,
	ENTITY_OPTIONS_IS_STATIC					= 1 << 3,
};


enum EntityCollisionLater : uint32_t
{
	ENTITY_COLLISION_LAYER_NONE				= 0,
	ENTITY_COLLISION_LAYER_BLOCKS			= 1 << 0,
	ENTITY_COLLISION_LAYER_PICKUPS			= 1 << 1,
	ENTITY_COLLISION_LAYER_ENEMIES			= 1 << 2,
};


enum AnimationState
{
	ANIMATION_STATE_NONE = -1,
	ANIMATION_STATE_IDLE,
	ANIMATION_STATE_MOVING,
	ANIMATION_STATE_JUMPING_UP,
	ANIMATION_STATE_FALLING_DOWN,
	ANIMATION_STATE_DEATH,
	ANIMATION_STATE_TAKE_DAMAGE,
	ANIMATION_STATE_HEAL,
	ANIMATION_STATE_ATTACK,
	ANIMATION_STATE_COUNT
};


struct EntityAnimStateData
{
public:
	SpriteAnimDefinition GetAsSpriteAnimDefinition() const;

public:
	float m_playbackDuration = 0.0f;
	float m_secondsPerFrame = 0.0f;
	SpriteAnimPlaybackMode m_playbackMode = SpriteAnimPlaybackMode::ONCE;
	IntVec2 m_spriteSheetLayout = IntVec2::ZERO;
	int m_animStartIndex = 0;
	int m_animEndIndex = 0;
	float m_scale = 1.0f;
	Vec2 m_pivot = Vec2::ZERO;
	bool m_blocksOtherAnimations = false;
};


struct EntityAnimationDataContainer
{
	EntityAnimationDataContainer() {}
	EntityAnimationDataContainer(EntityAnimationDataContainer const& copyContainer);

	void AddAnimation(AnimationState state, EntityAnimStateData animation);
public:
	EntityAnimStateData m_animations[ANIMATION_STATE_COUNT] = {};
};


enum TriggerType
{
	TRIGGER_TYPE_NONE = -1,
	TRIGGER_TYPE_ON_DEATH,
	TRIGGER_TYPE_ON_SPAWN,
	TRIGGER_TYPE_ON_DAMAGE,
	TRIGGER_TYPE_COUNT
};


struct EntitySpriteInfo
{
public:
	SpriteDefinition const* GetSpriteDefinition();
public:
	IntVec2 m_spriteSheetLayout = IntVec2::ZERO;
	float m_scale = 1.0f;
	Vec2 m_pivot = Vec2::ZERO;
	IntVec2 m_spriteCoords = IntVec2::ZERO;
};


enum AABB2Side : int
{
	AABB2_TOP_SIDE		= 2,
	AABB2_BOTTOM_SIDE	= -2,
	AABB2_LEFT_SIDE		= -1,
	AABB2_RIGHT_SIDE	= 1
};


struct AABB2CollisionData
{
public:
	AABB2CollisionData GetCollisionDataWithInvertedReference() const;
public:
	AABB2 m_refAABB2;
	AABB2 m_aabb2ToCheck;
	bool m_isTouching = false;
	bool m_isOverlapping = false;
	AABB2Side m_shortestOverlappingSide;
	float m_shortestOverlappingDisplacement = 0.0f;
};

AABB2CollisionData GetCollisionDataForAABB2s(AABB2 const& refAABB2, AABB2 const& aabb2ToCheck);
bool AreAABB2sTouching(AABB2 const& a, AABB2 const& b);