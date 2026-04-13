#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"

class Sagittarius : public Entity
{
public:
	Sagittarius(Vec2 const& position, float orientationDegrees, EntityFaction faction, EntityType type, Map* map);
	~Sagittarius();

	virtual void Update(float deltaSeconds)			override;
	virtual void Render()					const	override;
	virtual void Die()								override;
	virtual void TakeDamage(int damage)				override;

protected:
	bool CheckAndFollowTargetInSight(float deltaSeconds);
	void MoveTowardsTarget(float deltaSeconds);
	void CheckAndFireTurretAtTargetInRange(float deltaSeconds);

protected:
	float	m_maxWanderingTime				= 0.0f;
	float	m_turretLength					= 0.0f;
	float	m_turretLocalOrientationDegrees = 0.0f;
	float	m_turretAngularVelocity			= 0.0f;
	float   m_turretCooldownTimeSeconds		= 0.0f;
	float	m_turretCooldownTimer			= 0.0f;
	Vec2	m_turretLocalPosition			= Vec2::ZERO;
	Vec2	m_targetEntityPosition			= Vec2::ZERO;
	float	m_targetOrientationDegrees		= 0.0f;
	float	m_speed							= 0.0f;
	float	m_wanderingTime					= 0.0f;
	Entity* m_targetEntity					= nullptr;

};
