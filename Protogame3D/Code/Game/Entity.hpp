#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

class Game;

class Entity
{
public: //methods
	Entity(); //does nothing
	Entity(Vec3 const& startingPosition);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0; //updates the entity
	virtual void Render() const = 0; //draws the entity
	virtual void DebugRender() const = 0;

	Vec3 GetPosition() const;
	EulerAngles GetEulerAngles() const;
	Mat44 GetModelMatrix() const;
protected: //methods

protected: //members
	Vec3 m_position;
	EulerAngles m_orientation;
};
