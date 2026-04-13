#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Game;

//class representing an entity (an object) in the game
//it is owned by Game
class Entity
{
public: //methods
	Entity(); //does nothing
	Entity(Vec3 const& startingPosition);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0; //updates the entity
	virtual void Render() const = 0; //draws the entity
	virtual void DebugRender() const = 0;
// 	virtual void Die();
// 	bool IsOffScreen() const;
// 	bool IsAlive() const;
// 	bool IsGarbage() const;
// 	virtual void TakeDamage(int damage);

	//mutators
	void SetPhysicsRadius(float radius);
	void SetCosmeticRadius(float radius);

	//getters
// 	Vec2 GetForwardNormal() const;
 	Vec3 GetPosition() const;
	EulerAngles GetEulerAngles() const;
	Mat44 GetModelMatrix() const;
	float GetPhysicsRadius() const;
	float GetCosmeticRadius() const;
// 	float GetAcceleration() const;
// 	float GetOrientationDegrees() const;
// 	float GetSpeed() const;
protected: //methods
	//virtual void InitLocalVertexes() = 0; //method to initialize an entity's vertexes

protected: //members
	Vec3 m_position;
	EulerAngles m_orientation;

	float m_cosmeticRadius = 0.0f;
	float m_physicsRadius = 0.0f;
};
