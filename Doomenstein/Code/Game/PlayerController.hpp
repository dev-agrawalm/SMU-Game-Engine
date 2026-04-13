#pragma once
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/Controller.hpp"
#include "Engine/Core/Stopwatch.hpp"

class World;
class Weapon;

class PlayerController : public Controller
{
public:
	PlayerController(World* world, int playerIndex, int controllerIndex);
	~PlayerController();
	virtual void PreMapUpdate(float deltaSeconds) override;
	virtual void PostMapUpdate(float deltaSeconds) override;
	virtual void OnPossessedActorDamaged() override;
	virtual void OnPossessedActorKilled() override;

	void RenderHUD();
	void RenderWeaponHUD(Weapon* weapon);
	void TintHUD();
	void ApplyTint(Rgba8 const& startTint, Rgba8 const& endTint, float duration);
	void ApplyTint(Rgba8 const& endTint, float duration);

	void SetMovementSpeed(float moveSpeed);
	void SetWorldCameraViewport(AABB2 const& viewport);
	void SetScreenCameraViewport(AABB2 const& viewport);
	void SetNormalisedViewport(AABB2 const& normalisedViewport);
	void SetRespawnTime(float respawnTime);

	float			GetMoveSpeed() const;
	Camera const&	GetWorldCamera() const;
	Camera const&	GetScreenCamera() const;
	Mat44 const		GetModelMatrix() const;
	Vec3 const		GetPosition() const;
	Vec3			GetForward() const;
	int				GetPlayerIndex() const;
	Rgba8			GetCurrentTint() const;
	int				GetControllerIndex() const;
	Vec2			GetHUDDims() const;

	virtual void Think(float deltaSeconds) override;

	void SpawnAndPossessRandomActorFromSpawnPoint();
	void OnActorPossessed(Actor* actor) override;

protected:
	void RenderDeadHud();
	void CheckKeyboardAndMouseInput_EditorMode(float deltaSeconds);
	void CheckGamepadInput_EditorMode(float deltaSeconds);
	void EditorModeUpdate();

	void CheckKeyboardAndMouseInput_Possessed(float deltaSeconds);
	void CheckGamepadInput_Possessed(float deltaSeconds);

private:
	int m_playerIndex = -1;
	int m_controllerIndex = -999;
	Vec3 m_position;
	EulerAngles m_orientation;
	float m_movementSpeed = 0.0f;
	
	AABB2 m_normalisedViewport;
	Camera m_worldCamera;
	Camera m_screenCamera;
	Vec2 m_hudDims;

	Rgba8 m_startTint = Rgba8::WHITE_TRANSPARENT;
	Rgba8 m_endTint = Rgba8::WHITE_TRANSPARENT;
	Stopwatch m_tintTimer;
	Stopwatch m_respawnTimer;
};
