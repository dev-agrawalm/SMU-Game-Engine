#pragma once
#include "Game/ActorDefinition.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

//------------------------------------------------------------------------------------------------
class Controller; 
class Map; 
class World; 
class SpriteDefinition;
class Weapon;

struct SpawnInfo; 


//------------------------------------------------------------------------------------------------
// Flags are dynamic and may be changed at runtime
enum ActorFlagBit : uint32_t
{
	ACTOR_FLAG_IS_ALIVE							= 0,
	ACTOR_FLAG_IS_GARBAGE						= 1 << 0,	// actor should be cleaned up next garbage collection pass
	ACTOR_FLAG_IS_DEAD							= 1 << 1,	// actor has been killed (but may want to leave corpse)
	ACTOR_FLAG_IS_HIDDEN						= 1 << 2,	// hide even if it has visuals
	ACTOR_FLAG_NO_CLIP 							= 1 << 3,	// do not push even if it has collision (will skip physics update so can be used as a free fly camera)
	ACTOR_FLAG_IS_GROUNDED						= 1 << 4,	// Actor is touching the ground at the end of its last update
	ACTOR_FLAG_IS_POSSESSED_BY_AI				= 1 << 5,
	// more may be added
	// ...
}; 
typedef uint32_t ActorFlags;


//------------------------------------------------------------------------------------------------
class Actor 
{
friend Controller; 

public: 
	Actor( Map* spawningMap, SpawnInfo const& spawnInfo ); 
	~Actor(); 

	// updates
	void Update( float deltaSeconds );				// first tick group (figure out what we want to do)
	void LateUpdate( float deltaSeconds );			// second tick group (apply physics)
	void Render() const;							

	// common
	Map* GetMap() const										{ return m_map; }
	World* GetWorld() const;									
	ActorDefinition const* GetDefinition() const			{ return m_definition; }
	ActorFaction GetFaction() const							{ return m_faction; }
	ActorUID GetActorUID() const							{ return m_uid; }
	void SetActorUID(ActorUID uid);

	Vec3 GetPosition() const; 
	void SetPosition( Vec3 const& pos );
	void MoveBy( Vec3 const& movement ); 

	EulerAngles GetOrientation() const; 
	Vec3 GetForwardVector() const;
	Vec3 GetLeftVector() const;
	Vec3 GetUpVector() const; 

	void SetOrientation( EulerAngles const& orient ); 
	void RotateBy( EulerAngles const degrees ); 

	void EquipWeapon(int weaponIndex);
	Weapon* GetEquippedWeapon() const;
	bool	HasEquippedWeapon() const;

	FloatRange GetCollisionDamage() const;
	bool DoesDamageOnTouch() const;
	bool CreatesImpulseOnTouch() const;
	float GetTouchImpulse() const;
	bool CanFly() const;

	void Attack(bool isFreshAttack);
	bool DieOnSpawn() const;
	bool DieOnCollision() const;
	void Die();
	void TakeDamage(int damage);
	int GetHealth() const;
	bool IsDead() const;
	void MakeGarbage();
	bool IsGarbage() const; 

	// visual
	void SetHidden( bool isHidden ); 
	bool IsHidden() const; 
	inline bool IsVisible() const							{ return !IsHidden(); }			

	void SetAnimationToPlay(std::string animName);
	void PlayAnimation( std::string const& anim, SpriteAnimPlaybackMode playbackMode = SpriteAnimPlaybackMode::LOOP/*, float animSpeed = 1.0f*/ ) const;
	void SetAnimationSpeed(std::string animName, float speedScale = 1.0f );
	bool IsAnimationFinished() const; // if non-looping
	bool HasDefaultAnim() const;

	// collision
	bool HasCollision() const; 
	bool IsImmovable() const; 
	bool HasCollisionWithMap() const;
	bool HasCollisionWithActors() const; 

	void SetCollisionCylinder( float radius, float height ); 
	float GetCollisionRadius() const; 
	float GetCollisionHeight() const; 
	float GetCollisionBase() const;
	float GetLocalCollisionHeight() const;
	float GetLocalCollisionBase() const;

	// physics
	bool HasPhysicsUpdate() const; 

	void SetVelocity( Vec3 const& vel ); 
	void AddVelocity( Vec3 const& vel ); 
	void ApplyImpulse(Vec3 const& impulse);
	Vec3 GetVelocity() const								{ return m_velocity; }

	void SetAngularVelocity( EulerAngles const& angularVel ); 
	void SetTurnVelocity( float const yawVel ); 

	void AddFrameAcceleration( Vec3 const& acc );	
	float GetWalkSpeed() const;
	float GetRunSpeed() const;
	float GetDrag() const;
	
	// controller
	bool IsPossessable() const; 
	bool IsPossessed() const									{ return m_currentController != nullptr; }
	bool IsPossessedBy( Controller const* controller ) const	{ return m_currentController == controller; }
	Controller* GetController() const							{ return m_currentController; }
	Controller* GetAIController() const							{ return m_aiController; }

	void OnPossessed( Controller* controller ); 
	void OnUnpossessed( Controller* oldController ); 

	float GetCameraFOV() const; 
	float GetLocalEyeHeight() const; 
	Vec3 GetEyePosition() const; 
	Vec3 GetDeadEyePosition() const;


	// movement
	void MoveInDirection( Vec3 direction, float moveFraction, float walkSprintRatio ); 
	void TurnByOrientation( EulerAngles orientationDelta ); 
	void TurnTowardsOrientation( EulerAngles goalOrientation ); 

	// Helpers
	Mat44 GetModelMatrix() const; 

	bool HasAI() const;

private:
	void ResetPhysicsFrameData(); 
	void ApplyPhysics( float deltaSeconds ); 

public: 
	// basics
	Map*						m_map					= nullptr; 
	ActorDefinition const*		m_definition			= nullptr;

	ActorUID					m_uid; 
	ActorUID					m_owningActorUID = ActorUID::INVALID;
	Vec3						m_position; 
	EulerAngles					m_orientation;
	EulerAngles					m_goalOrientation;
	ActorFaction				m_faction;

	ActorOptions				m_options; 
	ActorFlags					m_flags = ACTOR_FLAG_IS_DEAD;

	Stopwatch					m_lifetime;
	Clock						m_animationClock;

	// visuals - sprite
	Vec2						m_scale					= Vec2(1.0, 1.0f);

	// visuals - common
	Rgba8						m_tint					= Rgba8::WHITE;  
	
	//visuals - animation
	std::string					m_animationToPlay;

	// collision 
	float						m_physicsRadius			= -1.0f; // no collision
	float						m_physicsHeight			= -1.0f; 
	float						m_physicsBase			= -1.0f;

	int							m_health				= -1;
	int							m_maxHealth				= -1;

	// physics
	Vec3						m_frameAcceleration;	// acceleration added this frame
	Vec3						m_velocity;				// current object velocity
	EulerAngles					m_angularVelocity;		// 

	// possession
	// player control
	Controller*					m_currentController		= nullptr; 
	Controller*					m_aiController			= nullptr;
	float						m_eyeHeight				= 0.0f; 
	float						m_deadEyeHeight			= 0.0f;
	float						m_camFov				= 0.0f;

	int m_equippedWeaponIndex = -1;
	std::vector<Weapon*> m_weapons;
}; 
