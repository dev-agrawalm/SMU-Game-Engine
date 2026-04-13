#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/TriggerSystem.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/EntityID.hpp"

class Level;

class NamedProperties;
typedef NamedProperties EventArgs;


class Entity
{
	friend class Level;

public:
	Entity(Level* level);
	Entity(Level* level, EntityType type, Vec2 const& position);
	Entity(Level* level, EntitySpawnInfo const& spawnInfo);
	~Entity();

	void Update(float deltaSeconds);
	void Render() const;
	void DebugRender() const;

	void MakeGarbage();
	void Die();
	bool TakeDamage(int damage);
	void Heal(int healAmt);
	void OnOverlap(AABB2CollisionData const& collisionData, Entity* otherEntity);
	
	void SetGrounded(bool newGroundedState);
	void SetPosition(Vec2 const& newPos);
	void SetVelocity(Vec2 const& newVel);
	void AddImpulseVelocity(Vec2 const& impulseVel);
	void CollectCoins(int coinsToAdd);
	void SetIsDead(bool isDead);
	void SetAnimationState(AnimationState newState);
	
	Vec2 GetVelocity() const;
	Vec2 GetPosition() const;
	AABB2 GetLocalPhysicsBounds() const;
	AABB2 GetWorldPhysicsBounds() const;
	AABB2 GetLocalCosmeticBounds() const;
	AABB2 GetWorldCosmeticBounds() const;

	bool HasOptions(uint32_t optionsBitmask);
	bool OnSwitchToggle(EventArgs& args);

	bool IsInvulnerable() const;
	bool IsAlive() const;
	bool IsGarbage() const;

protected:
	void OnOverlap_SwitchBlock(AABB2CollisionData const& collisionData, Entity* otherEntity);
	void OnOverlap_Enemy(AABB2CollisionData const& collisionData, Entity* otherEntity);
	void OnOverlap_GreenShell(AABB2CollisionData const& collisionData, Entity* otherEntity);

protected:
	Level* m_level = nullptr;
	EntityType m_type = ENTITY_TYPE_NONE;
	EntityProperties m_properties;
	uint32_t m_options = 0;
	EntityID m_entityId = {};
	EntityDefinition const* m_definition = nullptr;

	bool m_isGoingForward = true;
	EntityID m_prevWaypointId = EntityID::s_INVALID_ID;
	EntityID m_nextWaypointId = EntityID::s_INVALID_ID;

	float m_lifetime = -1.0f;
	int m_damage = 1;
	int m_health = 1;
	bool m_isDead = false;
	bool m_isGarbage = false;

	uint32_t m_collisionLayers = 0;
	bool m_isGrounded = false;
	bool m_isWalking = false;

	EntityFacingDirection m_newDirection = ENTITY_FACING_RIGHT;
	EntityFacingDirection m_direction = ENTITY_FACING_RIGHT;
	Vec2 m_position;	//refers to the bottom center position
	float m_speed = 0.0f;
	Vec2 m_velocity;
	Vec2 m_acceleration;
 
	Vec2 m_localBottomLeft;
	Vec2 m_localBottomRight;
	AABB2 m_localPhysicsBounds;
	AABB2 m_localCosmeticBounds;
	
	bool m_isInvulnerable = false;
	float m_invulnerabilityStartTime = 0.0f;
	float m_invulnerabiiltyDuration = 0.0f;

	//ANIMATION
	float m_animTimer = 0.0f;
	EntityAnimStateData m_animation = {};
	AnimationState m_animState = ANIMATION_STATE_NONE;

	//player variables
	int m_collectedCoinCount = 0;
	Stopwatch m_invulnerabiltyIndicatorTimer;
	bool m_isInvisible = false;
	unsigned int m_lostGroundOnFrame = 0;
	unsigned int m_coyoteJumpFrameCount = 0;

	Vec2 m_displacementDueWaypoint = Vec2::ZERO;
	//debug
	bool m_wasHit = false;

	float m_attackTimerDuration = 0.0f;
	float m_randomMovementTimer = 0.0f;
};
