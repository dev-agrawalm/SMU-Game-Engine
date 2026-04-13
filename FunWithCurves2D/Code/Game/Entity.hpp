#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class Game;

enum EntityFaction : int
{
	ENTITY_FACTION_INVALID = -1,
	ENTITY_FACTION_NEUTRAL,
	ENTITY_FACTION_PLAYER,
	ENTITY_FACTION_ENEMY,
	ENTITY_FACTION_COUNT
};

enum EntityType : int
{
	ENTITY_TYPE_INVALID = -1,
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_ENEMY,
	ENTITY_TYPE_PLAYER_BULLET,
	ENTITY_TYPE_ENEMY_BULLET,
	ENTITY_TYPE_BG_STAR
};

//class representing an entity (an object) in the game
//it is owned by Game
class Entity
{
public: //methods
	Entity(); //does nothing
	Entity(Vec2 const& startingPosition);
	Entity(Vec2 const& startingPos, EntityFaction faction);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0; //updates the entity
	virtual void Render() const = 0; //draws the entity
	void DebugRender() const;

	virtual void Die();
	virtual void TakeDamage(int damage);
	virtual void MakeGarbage();

	bool IsOffScreen() const;
	bool IsAlive() const;
	bool IsGarbage() const;

	void SetFaction(EntityFaction faction);
	void SetPosition(Vec2 const& newPos);
	void SetOrientation(float newOrientation);

	//getters
	int GetDamageOnCollision() const;
	Vec2 GetForwardNormal() const;
	Vec2 GetPosition() const;
	bool IsDead() const;
	bool DieOnImpact() const;
	float GetPhysicsRadius() const;
	float GetCosmeticRadius() const;
	float GetOrientationDegrees() const;
	float GetSpeed() const;
	EntityFaction GetFaction() const;
	bool IsBullet() const;
	int GetHealth() const;
	
	void SetLocalVerts(std::vector<Vertex_PCU> verts);
protected: //methods

protected: //members
	//linear movement
	Vec2 m_position;
	Vec2 m_velocity;
	float m_speed = 0.0f;

	//angular movement
	float m_orientationDegrees = 0.0f;
	float m_angularVelocity = 0.0f;

	//physics components
	float m_physicsRadius = 0.0f;
	float m_cosmeticRadius = 0.0f;

	bool m_dieOnImpact = false;
	int m_health = 1;
	float m_lifetime = -1.0f;
	float m_age = 0.0f;

	int m_damageOnCollision = 0;

	//Rendering components
	Vertex_PCU* m_localVertexes = nullptr; //pointer to a dynamic array of vertexes for rendering

	bool m_isDead = false; //flag to check if the object is active i.e. can interact with the other alive entities
	bool m_isGarbage = false; //flag to check if the object has to be deleted from memory or not

	EntityFaction m_faction = ENTITY_FACTION_INVALID;
	bool m_isBullet = false;
};
