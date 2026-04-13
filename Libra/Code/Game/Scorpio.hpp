#pragma once
#include "Game/Entity.hpp"

struct Vec2;

class Scorpio : public Entity
{
public:
	Scorpio();
	~Scorpio();
	Scorpio(Vec2 const& position, float orientationDegrees, EntityFaction faction, EntityType type, Map* map);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void TakeDamage(int damage) override;

protected:
	bool CheckAndFollowTargetInSight(float deltaSeconds);
	void MoveTurretTowardsTarget(float deltaSeconds);
	void CheckAndFireTurretAtTarget();

private:
	float	m_turretLength					 = 0.0f;
	float	m_turretLocalOrientationDegrees	 = 0.0f;
	float	m_turretAngularVelocity			 = 0.0f;
	float	m_turretCooldownTimer			 = 0.0f;
	float	m_turretCooldownTimeSeconds		 = 0.0f;
	float	m_targetTurretOrientationDegrees = 0.0f;
	Vec2	m_turretLocalPosition			 = Vec2::ZERO;
	Vec2	m_targetPosition				 = Vec2::ZERO;
	Entity* m_targetEntity = nullptr;
};
