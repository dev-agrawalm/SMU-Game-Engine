#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"

class Entity;

enum GameCameraMode
{
	GAME_CAMERA_MODE_FIRST_PERSON = 0,
	GAME_CAMERA_MODE_OVER_SHOULDER,
	GAME_CAMERA_MODE_FIXED_ANGLE_TRACKING,
	GAME_CAMERA_MODE_SPECTATOR,
	GAME_CAMERA_MODE_COUNT,
};


class GameCamera : public Camera
{
public:
	GameCamera();

	void Initialise();
	void Update(float deltaSeconds);
	void CheckInput(float deltaSeconds);

	void AttachToEntity(Entity* entity);
	void DetachFromEntity();

	void SetCameraMode(GameCameraMode newCameraMode);
	GameCameraMode GetCameraMode() const;
private:
	void DisplayDebugInfo();

private:
	GameCameraMode m_cameraMode = GAME_CAMERA_MODE_FIRST_PERSON;
	Entity* m_attachedEntity = nullptr;
	EulerAngles m_orientaion;
	Vec3 m_position;
};
