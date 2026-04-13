#pragma once
#include "Game/Entity.hpp"

class Game;

class Bullet : public Entity
{

public:
	Bullet(); //does nothing
	Bullet(Game* game, Vec2 const&  position, float orientationDegrees);
	~Bullet(); //does nothing

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;

	int GetDamage() const;
	Rgba8 GetDebrisColor() const override;
protected:
	virtual void InitLocalVertexes() override;
private:
	int m_damage = 0;
};