#pragma once
#include "Game/GameCommon.hpp"

//#include "Engine/Core/BitUtility.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include <string>
#include <vector>
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Game/Weapon.hpp"

class Shader;
class Texture; 
class SpriteSheet;


//------------------------------------------------------------------------------------------------
// Options are static and come from data
enum ActorOptionBitMask : uint32_t
{
	ACTOR_OPTION_NONE					= 0,

	ACTOR_OPTION_CAN_BE_POSSESSED		= 1 << 0,	//BIT_FLAG(0),	// Controller can own this
	ACTOR_OPTION_COLLIDES_WITH_MAP		= 1 << 1,	//BIT_FLAG(1), 
	ACTOR_OPTION_COLLIDES_WITH_ACTORS	= 1 << 2,	//BIT_FLAG(2), 
	ACTOR_OPTION_IMMOVABLE  			= 1 << 3,	//BIT_FLAG(3),  // may push others, but can not be pushed
	ACTOR_OPTION_PHYSICS_UPDATE			= 1 << 4,	//BIT_FLAG(4),	// Actor moves from physics (vel/accel)
	ACTOR_OPTION_HAS_AI					= 1 << 5,
	ACTOR_OPTION_HAS_DEFAULT_ANIM		= 1 << 6,
	ACTOR_OPTION_DIE_ON_SPAWN			= 1 << 7,
	ACTOR_OPTION_CAN_FLY				= 1 << 8,
	ACTOR_OPTION_DIE_ON_COLLISION		= 1 << 9,
	ACTOR_OPTION_DAMAGE_ON_TOUCH		= 1 << 10,
	ACTOR_OPTION_IMPULSE_ON_TOUCH		= 1 << 11,
	// more may be added
	// ...
}; 
typedef uint32_t ActorOptions; 


enum ActorFaction : int
{
	ACTOR_FACTION_UNKNOWN = -1,
	ACTOR_FACTION_MARINES,
	ACTOR_FACTION_DEMONS,
	ACTOR_FACTION_NEUTRAL,
	ACTOR_FACTION_PROJECTILE,
	ACTOR_FACTION_MARINE_SPAWN,
	ACTOR_FACTION_DEMON_SPAWN,
	NUM_ACTOR_FACTION
};


//------------------------------------------------------------------------------------------------
enum class BillboardType 
{
	NONE,					// just use model matrix of the actor
	CAMERA_FACING_XY,		// plane perpendicular to vector extended toward the object from the camera
	CAMERA_ALIGNED_XY,		// plane parallel to camera right
	CAMERA_FACING
}; 


//------------------------------------------------------------------------------------------------
// Prep for A09 - Animating the sprites
enum class LoopMode 
{
	CLAMP,
	REPEAT,
}; 


//------------------------------------------------------------------------------------------------
class SpriteAnimation
{
public:
	SpriteAnimation() {}
	SpriteAnimation( std::string const& framesCSV ); 

	bool FromCSV( std::string const& framesCSV ); 

public:
	std::vector<int> m_frameIndices; 
}; 


//------------------------------------------------------------------------------------------------
struct ViewDependentSpriteAnimationGroup
{
public: 
	bool LoadFromXmlElement( XmlElement const& elem ); 
	bool LoadSpriteAnimationForDirectionFromXmlElement(XmlElement const& elem, std::string dirString, Vec3 const& direction);
	SpriteAnimation const* GetBestAnimationForDirection( Vec3 const& dir ) const;

public:
	std::string m_name = ""; 
	bool m_scaleBySpeed = false;
	float m_secondsPerFrame = 0.25f; // "frameTime"
	float m_framesPerSecond = 4.0f;  // derived from above

	std::vector<Vec3> m_directions; 
	std::vector<SpriteAnimation> m_animations; 
}; 


//------------------------------------------------------------------------------------------------
// Optional - just utility for getting the info I need to render an object
// in the form I need it.  Pick something that fits your setup. 
struct SpriteInfo
{
	Shader const* m_shader = nullptr;	// shader to use to draw
	Texture const* m_texture = nullptr; // texture to use to draw
	AABB2 m_uvs;						// uv of this sprite
	AABB2 m_localCoordinates;			// local space coordinates used for drawing this billboard
}; 


//------------------------------------------------------------------------------------------------
class ActorAppearance
{
public:
	~ActorAppearance(); 

	bool LoadFromXmlElement( XmlElement const& elem ); 

	bool IsValid() const						{ return nullptr != m_spritesheet; }

	Mat44 GetBillboardMatrix( Vec3 const& objPos, Camera const& viewer ) const; 
	SpriteInfo GetSpriteInfo( Vec3 const& facing ) const; 
	ViewDependentSpriteAnimationGroup GetAnimationGroupByName(std::string const& name) const;

public:
	Shader const* m_shader						= nullptr; 
	SpriteSheet* m_spritesheet					= nullptr; 
	Vec2 m_size									= Vec2( 1.0f, 1.0f ); 
	Vec2 m_pivot								= Vec2( 0.5f, 0.0f ); 
	BillboardType m_billboard					= BillboardType::CAMERA_FACING_XY; 
	std::string m_defaultAnimName				= "";

	std::vector<ViewDependentSpriteAnimationGroup> m_animationGroups; 
}; 


//------------------------------------------------------------------------------------------------
class ActorDefinition
{
public:
	static void LoadActorDefinitionsFromXMLFile(std::string filePath);
	static ActorDefinition const* GetActorDefinition(std::string definitionName);
	static bool DoesDefinitionExist(std::string definitionName);
	static void DeleteAllActorDefinitions();

public:
	ActorDefinition() {}
	bool LoadFromXmlElement( XmlElement const& node );

public:
	
	// options
	std::string m_name; 
	ActorOptions m_options		= ACTOR_OPTION_NONE; 
	
	// visuals
	ActorAppearance m_appearance; 

	// collision
	float m_physicsRadius		= -1.0f;	// negative means no collision
	float m_physicsHeight		= -1.0f;
	float m_physicsBase			= 0.0f;
	FloatRange m_damageOnTouch	= FloatRange::ZERO;
	float m_impulseOnTouch		= -1.0f;

	// physics
	float m_walkSpeed			= 4.0f;		// default maxSpeed
	float m_runSpeed			= 8.0f;		
	float m_drag				= 8.0f; 
	EulerAngles m_turnSpeed		= EulerAngles(180.0f, 120.0f, 0.0f );

	// Camera Options (when possessed)
	float m_eyeHeight			= 1.75f; 
	float m_cameraFOVDegrees	= 60.0f;
	float m_deadEyeHeight		= 0.0f;

	//misc
	int m_health = -1;
	float m_lingerTime = -1.0f;

	//AI shit
	std::string m_aiType = "";
	float m_sightRadius = 0.0f;
	float m_sightAngle = 0.0f;

	std::string m_painSfx;
	std::string m_deathSfx;

	std::vector<WeaponDefinition const*> m_weaponInventory;
protected:
	static std::vector<ActorDefinition const*> s_actorDefinitions;
};
