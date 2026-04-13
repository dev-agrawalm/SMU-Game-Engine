#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Game;

//class representing an entity (an object) in the game
//it is owned by Game
class Entity
{
public: //methods
	Entity(); //does nothing
	Entity(Game* game, Vec2 const& startingPosition);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0; //updates the entity
	virtual void Render() const = 0; //draws the entity
	void DebugRender() const;
	virtual void Die();
	bool IsOffScreen() const;
	bool IsAlive() const;
	bool IsGarbage() const;
	virtual void TakeDamage(int damage);

	//getters
	Vec2 GetForwardNormal() const;
	Vec2 GetPosition() const;
	float GetPhysicsRadius() const;
	float GetCosmeticRadius() const;
	float GetAcceleration() const;
	float GetOrientationDegrees() const;
	float GetSpeed() const;
	Vertex_PCU* const GetLocalVertexes() const;
	virtual Rgba8 GetDebrisColor() const;
protected: //methods
	virtual void InitLocalVertexes() = 0; //method to initialize an entity's vertexes

protected: //members
	//linear movement
	Vec2 m_position; //position of the entity in the world
	Vec2 m_velocity;  //world units per second
	float m_acceleration = 0.0f; //world unit per (second*second)
	//angular movement
	float m_orientationDegrees = 0.0f; //signed angle of the forward direction from the x+ east direction (1,0) // measured counter clockwise
	float m_angularVelocity = 0.0f; //degrees per second

	//physics components
	float m_physicsRadius = 0.0f; //radius of the circle used for the physical interaction
	float m_cosmeticRadius = 0.0f; //radius of the smallest circle that contains all vertexes

	int m_health = 1;
	float m_age = 0.0f; //lifetime in seconds for which the entity will exist
	
	//Rendering components
	Vertex_PCU* m_localVertexes = nullptr; //pointer to a dynamic array of vertexes for rendering

	bool m_isDead = false; //flag to check if the object is active i.e. can interact with the other alive entities
	bool m_isGarbage = false; //flag to check if the object has to be deleted from memory or not

	Game* m_game = nullptr; //pointer to the game that owns the entity
};