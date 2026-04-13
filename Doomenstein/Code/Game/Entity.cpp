#include "Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Entity::Entity(Vec3 const&  startingPosition) :
	m_position(startingPosition) 
{
	//empty
}


Entity::Entity()
{
	//empty
}


Entity::~Entity()
{
}


void Entity::SetPhysicsRadius(float radius)
{
	m_physicsRadius = radius;
}


void Entity::SetCosmeticRadius(float radius)
{
	m_cosmeticRadius = radius;
}


Vec3 Entity::GetPosition() const
{
	return m_position;
}


EulerAngles Entity::GetEulerAngles() const
{
	return m_orientation;
}


Mat44 Entity::GetModelMatrix() const
{
	Mat44 modelMatrix;
	modelMatrix = Mat44::CreateTranslation3D(m_position);
	Mat44 oritentationMat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.Append(oritentationMat);
	return modelMatrix;
}


float Entity::GetPhysicsRadius() const
{
	return m_physicsRadius;
}


float Entity::GetCosmeticRadius() const
{
	return m_cosmeticRadius;
}
