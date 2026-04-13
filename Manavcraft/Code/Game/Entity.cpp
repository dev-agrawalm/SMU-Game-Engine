#include "Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCamera.hpp"

Entity::Entity(Vec3 const&  startingPosition) 
	: m_bottomCenterPos(startingPosition) 
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


void Entity::SetOrientation(EulerAngles newOrientation)
{
	m_orientation = newOrientation;
}


void Entity::OnCameraDetached()
{
	m_attachedCamera = nullptr;
}


void Entity::OnCameraAttached(GameCamera* attachedCamera)
{
	m_attachedCamera = attachedCamera;
}


void Entity::AddAcceleration(Vec3 const& forceAcceleration)
{
	m_frameAcceleration += forceAcceleration;
}


void Entity::ResetFrameAcceleration()
{
	m_frameAcceleration = Vec3(0.0f, 0.0f, 0.0f);
}


void Entity::SetFrameAcceleration(Vec3 const& newFrameAcceleration)
{
	m_frameAcceleration = newFrameAcceleration;
}


AABB3 Entity::GetLocalBounds() const
{
	return m_localBounds;
}


AABB3 Entity::GetWorldBounds() const
{
	Mat44 translationMatrix = Mat44::CreateTranslation3D(m_bottomCenterPos);
	AABB3 worldBounds;
	worldBounds.m_mins = translationMatrix.TransformPosition3D(m_localBounds.m_mins);
	worldBounds.m_maxs = translationMatrix.TransformPosition3D(m_localBounds.m_maxs);
	return worldBounds;
}


Vec3 Entity::GetEyePos() const
{
	return Vec3(m_bottomCenterPos.x, m_bottomCenterPos.y, m_bottomCenterPos.z + m_eyeHeight);
}


Vec3 Entity::GetPosition() const
{
	return m_bottomCenterPos;
}


EulerAngles Entity::GetOrientation() const
{
	return m_orientation;
}


Mat44 Entity::GetModelMatrix() const
{
	Mat44 modelMatrix;
	modelMatrix = Mat44::CreateTranslation3D(m_bottomCenterPos);
	Mat44 oritentationMat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.Append(oritentationMat);
	return modelMatrix;
}


Vec3 Entity::GetForward() const
{
	return m_orientation.GetFoward_XFwd_YLeft_ZUp();
}


GameCamera* Entity::GetCamera()
{
	return m_attachedCamera;
}


EntityPhysicsMode Entity::GetPhysicsMode() const
{
	return m_physicsMode;
}


Vec3 Entity::GetVelocity()
{
	return m_velocity;
}


bool Entity::IsOnGround()
{
	return m_isOnGround;
}
