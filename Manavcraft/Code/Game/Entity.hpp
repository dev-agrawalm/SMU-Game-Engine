#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/AABB3.hpp"

struct WorldRaycastResult;
class GameCamera;
class World;

enum EntityPhysicsMode
{
	ENTITY_PHYSICS_MODE_WALKING = 0,
	ENTITY_PHYSICS_MODE_FLYING,
	ENTITY_PHYSICS_MODE_NO_CLIP,
	ENTITY_PHYSICS_MODE_COUNT
};

class Entity
{
protected:
// 	enum
// 	{
// 		TOP_NORTH_EAST_RAYCAST = 0, TOP_NORTH_WEST_RAYCAST, TOP_SOUTH_EAST_RAYCAST, TOP_SOUTH_WEST_RAYCAST, 
// 		MID_NORTH_EAST_RAYCAST, MID_NORTH_WEST_RAYCAST, MID_SOUTH_EAST_RAYCAST, MID_SOUTH_WEST_RAYCAST,
// 		BOTTOM_NORTH_EAST_RAYCAST, BOTTOM_NORTH_WEST_RAYCAST, BOTTOM_SOUTH_EAST_RAYCAST, BOTTOM_SOUTH_WEST_RAYCAST,
// 		DOWNWARD_
// 		CLOSEST_HIT_PHYSICS_RAYCAST, PHYSICS_RAYCAST_COUNT,
// 	};

public: //methods
	Entity(); //does nothing
	Entity(Vec3 const& startingPosition);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0; //updates the entity
	virtual void Render() const = 0; //draws the entity
	virtual void DebugRender() const = 0;

	void SetOrientation(EulerAngles newOrientation);
	virtual void OnCameraDetached();
	virtual void OnCameraAttached(GameCamera* attachedCamera);

	void AddAcceleration(Vec3 const& forceAcceleration);
	void ResetFrameAcceleration();
	void SetFrameAcceleration(Vec3 const& newFrameAcceleration);

	AABB3 GetLocalBounds() const;
	AABB3 GetWorldBounds() const;
	Vec3 GetEyePos() const;
	Vec3 GetPosition() const;
	EulerAngles GetOrientation() const;
	Mat44 GetModelMatrix() const;
	Vec3 GetForward() const;
	GameCamera* GetCamera();
	EntityPhysicsMode GetPhysicsMode() const;
	Vec3 GetVelocity();
	bool IsOnGround();
protected: //methods

protected: //members
	Vec3 m_bottomCenterPos;
	float m_eyeHeight = 0.0f;
	AABB3 m_localBounds;
	EulerAngles m_orientation;

	//physics variable
	Vec3 m_frameAcceleration;
	Vec3 m_velocity;
	EntityPhysicsMode m_physicsMode = ENTITY_PHYSICS_MODE_WALKING;
	bool m_isOnGround = false;

	GameCamera* m_attachedCamera = nullptr;
};
