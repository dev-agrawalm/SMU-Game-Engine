#pragma once
#include "Game/Entity.hpp"

struct BlockDef;
struct WorldRaycastResult;

class Player : public Entity
{
public:
	Player();
	virtual ~Player();
	Player(Vec3 const& position);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	
	void SetMovementSpeed(float moveSpeed);
	void SetMouseSensitivity(float mouseSens);
	void SetSprintSpeed(float sprintSpeed);

	float GetMoveSpeed() const;
	float GetSprintSpeed() const;
	float GetMouseSensitivity() const;
	BlockDef const* GetPlacementBlockDef() const;
	uint8_t GetPlacementBlockId() const;

protected:
	void CheckKeyboardAndMouseInput(float deltaSeconds);
	void CheckGamepadInput(float deltaSeconds);
	void DisplayDebugInformationOnScreen();
	void MovePlayer(float deltaSeconds);
	void CalculateWorldPhysicsForces();
private:
	float m_movementSpeed	= 4.0f;
	float m_sprintSpeed		= 9.0f;
	float m_mouseSensitivity = 0.1f;

	uint8_t m_placementBlockDefId = 0;
};
