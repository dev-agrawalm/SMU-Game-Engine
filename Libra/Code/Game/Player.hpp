#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"

class Player : public Entity
{
public:
	Player(Vec2 const& position, EntityFaction faction, EntityType type, Map* map);
	~Player();

	virtual void Update(float deltaSeconds)			override;
	virtual void Render()					const	override;
	virtual void Die()								override;
	virtual void TakeDamage(int damage)				override;
			void CheckInput(float deltaSeconds);

protected:
	void CheckInputForTank(float deltaSeconds);
	void CheckKeyboardInputForTank(float deltaSeconds);
	void CheckControllerInputForTank(float deltaSeconds);
	void CheckInputForTurret(float deltaSeconds);
	void FireTurret();
	void CheckKeyboardInputForTurret(float deltaSeconds);
	void CheckControllerInputForTurret(float deltaSeconds);

protected:
	bool	m_godMode							= false;
	float	m_turretLength						= 0.0f;
	float	m_turretLocalOrientationDegrees		= 0.0f;
	float	m_turretAngularVelocity				= 0.0f;
	float   m_turretCooldownTimeSeconds			= 0.0f;
	float	m_turretCooldownTimer				= 0.0f;
	Vec2	m_turretLocalPosition				= Vec2::ZERO;
	float	m_speed								= 0.0f;

};
