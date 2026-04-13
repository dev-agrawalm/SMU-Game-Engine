#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Game;
class Map;
struct AABB2;

enum EntityType
{
	ENTITY_TYPE_INVALID_TYPE = -1,
	ENTITY_TYPE_GOOD_PLAYER,
	ENTITY_TYPE_EVIL_SCORPIO,
	ENTITY_TYPE_EVIL_LEO,
	ENTITY_TYPE_EVIL_ARIES,
	ENTITY_TYPE_EVIL_CAPRICORN,
	ENTITY_TYPE_EVIL_SAGITTARIUS,
	// ADD ANY AND ALL TANK ENTITIES ABOVE THIS LINE AND UPDATE THE TANK ENUM COUNT

	ENTITY_TYPE_GOOD_BULLET,
	ENTITY_TYPE_EVIL_BULLET,
	ENTITY_TYPE_EVIL_GUIDED_MISSILE,
	//ADD ANY AND ALL BULLET ENTITIES ABOVE THIS LINE AND UPDATE THE BULLET ENUM COUNT

	ENTITY_TYPE_TANK_EXPLOSION,
	ENTITY_TYPE_BULLET_EXPLOSION,
	ENTITY_TYPE_PLAYER_EXPLOSION,
	//ADD ALL OTHER KIND OF ENTITIES ABOVE THIS LINE AND UPDATE THE OTHERS ENUM COUNT
	NUM_ENTITY_TYPES,
	TANK_ENUM_COUNT = 6,
	BULLET_ENUM_COUNT = 9,
	EXPLOSIONS_ENUM_COUNT = 12
};


enum EntityFaction
{
	ENTITY_FACTION_INVALID_FACTION = -1,
	ENTITY_FACTION_GOOD,
	ENTITY_FACTION_EVIL,
	ENTITY_FACTION_NEUTRAL,
	NUM_ENTITY_FACTIONS
};


//class representing an entity (an object) in the game
class Entity
{
	friend class Map;

public: //methods
	Entity(); //does nothing
	Entity(Vec2 const& startingPosition, float orientationDegrees, EntityFaction faction, EntityType type, Map* map);
	virtual ~Entity();

	virtual void		Die();
	virtual void		TakeDamage(int damage);
	virtual void		Update(float deltaSeconds)		= 0;
	virtual void		Render()				const	= 0;
	void				DebugRender()			const;

	//getters
	bool				IsAlive()				const;
	bool				IsGarbage()				const;
	Vec2				GetForwardNormal()		const;
	Vec2				GetPosition()			const;
	float				GetPhysicsRadius()		const;
	float				GetCosmeticRadius()		const;
	float				GetAcceleration()		const;
	float				GetOrientationDegrees() const;
	float				GetSpeed()				const;
	EntityType			GetType()				const;
	EntityFaction		GetFaction()			const;

	//mutators
	void				SetPosition(Vec2 const& position);
	void				SetMap(Map* map);
	void				SetLifeStatus(bool isAlive);
	void				SetHealth(int health);

protected: //members
	Vec2			m_position;							//position of the entity in the world
	Vec2			m_velocity;							//world units per second
	float			m_acceleration			= 0.0f;		//world unit per (second*second)
	
	float			m_orientationDegrees	= 0.0f;		//signed angle of the forward direction from the x+ east direction (1,0) // measured counter clockwise
	float			m_angularVelocity		= 0.0f;		//degrees per second

	float			m_physicsRadius			= 0.0f;		//radius of the circle used for the physical interaction
	float			m_cosmeticRadius		= 0.0f;		//radius of the smallest circle that contains all vertexes
	
	int				m_health				= 1;		//health of the entity
	int				m_maxHealth				= 1;
	float			m_age					= 0.0f;		//lifetime in seconds for which the entity will exist
	bool			m_isDead				= false;	//flag to check if the object is active i.e. can interact with the other alive entities
	bool			m_isGarbage				= false;	//flag to check if the object has to be deleted from memory or not
	
	//physics flags
	bool			m_isPushedByWalls		= true;
	bool			m_isPushedByEntities	= true;
	bool			m_doesPushEntities		= true;
	bool			m_canSwim				= false;

	EntityFaction	m_faction				= EntityFaction::ENTITY_FACTION_INVALID_FACTION;
	EntityType		m_type					= EntityType::ENTITY_TYPE_INVALID_TYPE;
	Map*			m_map					= nullptr;
};
