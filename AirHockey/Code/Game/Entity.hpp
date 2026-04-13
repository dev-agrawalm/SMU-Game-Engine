#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include "Engine/Core/CircularBuffer.hpp"
#include "Engine/Math/Spline2D.hpp"

enum EntityType
{
	ENTITY_TYPE_NONE = -1,
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_PUCK,
	ENTITY_TYPE_COUNT
};

class Entity
{
public: //methods
	Entity(); //does nothing
	Entity(Vec3 const& startingPosition);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0; //updates the entity
	virtual void Render() = 0; //draws the entity
	virtual void DebugRender() const = 0;

	void SetPosition(Vec3 const& position);
	void SetForward(Vec3 const& newFwd);
	void SetVelocity(Vec3 const& newVel);
	void SetAngularVelocityDegrees(float angularVelocity);

	Vec3 GetForward() const;
	Vec3 GetPosition() const;
	EulerAngles GetEulerAngles() const;
	Mat44 GetModelMatrix() const;
	float GetPhysicsRadius() const;
	Vec3 GetVelocity() const;
protected: //methods

public:
	CircularBuffer<Vec3> m_neonTrailPositions;
	bool m_pushedByEntity = false;
	bool m_pushesEntity = false;
	bool m_bouncesOffWalls = false;

	float m_speed = 0.0f;
	float m_angularVelocityDegrees;
	Vec3 m_velocity;
	Vec3 m_curPosition;
	Vec3 m_prevPos;
	float m_orientationDegrees = 0.0f;

	EntityType m_type;
	int m_score = 0;

	bool m_isDead = false;
	bool m_wasHit = false;

	Spline2D m_neonGlowSpline;
protected: //members
	float m_physicsRadius = 0.0f;

	std::vector<Vertex_PCU> m_localVerts;
};
