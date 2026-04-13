#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Stopwatch.hpp"

struct LevelRaycastResult;



//
//
//
//
//			UNUSED CLASS
//
//
//
//


class Player : public Entity
{
	friend class Level;

public:
	Player(Level* level, Vec2 const& position);
	~Player();

// 	virtual void Render() const override;
// 	virtual void Update(float deltaSeconds) override;
// 	virtual bool TakeDamage(int damage) override;
	void CheckInput(float deltaSeconds);
	void CollectCoins(int coinsToAdd);
	void AddImpulseVelocity(Vec2 const& forceVec);
private:
	void CalculateWorldForces();
	void MovePlayer(float deltaSeconds);

private:
	int m_collectedCoinCount = 0;
	float m_walkingAnimationTime = 0.0f;
	unsigned int m_lostGroundOnFrame = 0;
	//bool m_isJumping = false;

	Stopwatch m_invulnerabiltyIndicatorTimer;
	bool m_isInvisible = false;

	Vec2 m_localBottomLeft;
	Vec2 m_localBottomRight;
};
