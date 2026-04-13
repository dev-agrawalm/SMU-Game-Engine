#pragma once
#include "Game/Entity.hpp"

struct Vec2;

class Aries : public Entity
{
public:
	Aries();
	~Aries();
	Aries(Vec2 const& position, float orientationDegrees, EntityFaction faction, EntityType type, Map* map);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void TakeDamage(int damage) override;

protected:
	bool CheckAndFollowTargetInSight(float deltaSeconds);
	void MoveTowardsTarget(float deltaSeconds);

private:
	float	m_speed							= 0.0f;
	float	m_maxWanderingTime				= 0.0f;
	float	m_wanderingTime					= 0.0f;
	float	m_targetOrientationDegrees		= 0.0f;
	Vec2	m_targetPosition				= Vec2::ZERO;
	Entity* m_targetEntity = nullptr;
};
