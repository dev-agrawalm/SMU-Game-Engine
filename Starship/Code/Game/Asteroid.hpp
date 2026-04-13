#pragma once
#include "Game/Entity.hpp"

class Game;

class Asteroid : public Entity
{
	static const Rgba8 ASTEROID_GREY;
public:
	Asteroid();
	Asteroid(Game* game, Vec2 const& position, float orientationAngle, Vec2 const& velocity);
	~Asteroid();
	
	void Init(Game* game, Vec2 const& position, float orientationAngle, Vec2 const& velocity);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void TakeDamage(int damage) override;

	virtual Rgba8 GetDebrisColor() const override;
protected:
	virtual void InitLocalVertexes() override;
	void InitAngularVelocity();
	void WarpAroundTheScreen();
private:
	bool m_wasHit = false;
};