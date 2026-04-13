#pragma once
#include "Game/Entity.hpp"

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
	void SetRotationSpeed(float mouseSens);
	void SetRollSpeed(float rollSpeed);
	void SetSprintSpeed(float sprintSpeed);

	float GetMoveSpeed() const;
	float GetRollSpeed() const;
	float GetSprintSpeed() const;
	float GetRotationSpeed() const;

protected:
	void CheckKeyboardAndMouseInput(float deltaSeconds);
	void CheckGamepadInput(float deltaSeconds);
private:
	float m_movementSpeed	= 1.0f;
	float m_sprintSpeed		= 5.0f;
	float m_rotationSpeed	= 30.0f;
	float m_rollSpeed		= 15.0f;
};
