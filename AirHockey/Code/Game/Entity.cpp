#include "Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Entity::Entity(Vec3 const&  startingPosition) :
	m_curPosition(startingPosition) 
{
	//m_neonTrailPositions.Resize(NUM_PTS_NEON_TRAIL);
}


Entity::Entity()
{
	//empty
}


Entity::~Entity()
{
}


void Entity::SetPosition(Vec3 const& position)
{
	m_curPosition = position;
}


void Entity::SetForward(Vec3 const& newFwd)
{
	m_orientationDegrees = newFwd.GetVec2().GetOrientationDegrees();
}


void Entity::SetAngularVelocityDegrees(float angularVelocity)
{
	m_angularVelocityDegrees = angularVelocity;
}

void Entity::SetVelocity(Vec3 const& newVel)
{
	m_velocity = newVel;
}


Vec3 Entity::GetForward() const
{
	return GetModelMatrix().GetIBasis3D();
}


Vec3 Entity::GetPosition() const
{
	return m_curPosition;
}


EulerAngles Entity::GetEulerAngles() const
{
	return EulerAngles(m_orientationDegrees, 0.0f, 0.0f);
}


Mat44 Entity::GetModelMatrix() const
{
	Mat44 modelMatrix;
	modelMatrix = Mat44::CreateTranslation3D(m_curPosition);
	EulerAngles eulerAngles = EulerAngles(m_orientationDegrees, 0.0f, 0.0f);
	Mat44 oritentationMat = eulerAngles.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.Append(oritentationMat);
	return modelMatrix;
}


float Entity::GetPhysicsRadius() const
{
	return m_physicsRadius;
}


Vec3 Entity::GetVelocity() const
{
	return GetForward() * m_speed;
}
