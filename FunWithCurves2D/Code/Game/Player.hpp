#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/BezierCurve2D.hpp"

enum Tween
{
	TWEEN_LINEAR,
	TWEEN_SMOOTH_STOP_2,
	TWEEN_SMOOTH_START_2,
	TWEEN_SMOOTH_STOP_3,
	TWEEN_SMOOTH_START_3,
	TWEEN_SMOOTH_STEP_3,
	TWEEN_HESISTATE_3,
	TWEEN_HESISTATE_4
};


class Weapon;

class Player : public Entity
{
public:
	Player();
	~Player();
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void TakeDamage(int damage) override;
	virtual void Die() override;
	void CheckInput(float deltaSeconds);
	void Attack();

	Weapon* GetActiveWeapon();
	int GetNextWeaponIndex();
	int GetPrevWeaponIndex();
private:
	std::vector<Weapon*> m_weapons;
	int m_activeWeaponIndex = -1;
	float m_timeSinceDeath = 0.0f;
	float m_acceleration = 0.0f;
	float m_sidewaysMovementDrag = 0.0f;
	Vec2 m_movementDirection;
};
