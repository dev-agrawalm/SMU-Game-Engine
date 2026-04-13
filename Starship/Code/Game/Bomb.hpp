#pragma once
#include "Game/Entity.hpp"

class Game;
struct Vec2;

class Bomb : public Entity
{
	static const Rgba8 BOMB_RED;
	static const Rgba8 BOMB_WHITE;
	static const Rgba8 EXPLOSION_WHITE;
	static const Rgba8 EXPLOSION_BLACK;
public:
	Bomb();
	~Bomb();
	Bomb(Game* game, Vec2 const& position);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

	//getters
	bool IsExploded() const;
	int GetDamage() const;
	float GetExplosionRadius() const;
protected:
	virtual void InitLocalVertexes() override;

private:
	float m_explosionRadius = 0.0f;
	
	Rgba8 m_bombColor;
	float m_waitTime = 0.0f;
	float m_previousWaitTime = 0.0f;
	float m_deltaWaitTime = 0.0f;
	float m_waitTimeBlinkInterval = 0.75f;

	Rgba8 m_explosionColor;
	float m_explosionLifetime = 0.0f;
	float m_previousExplosionLifetime = 0.0f;
	float m_deltaExplosionLifetime = 0.0f;
	float m_explosionBlinkInterval = 0.1f;

	bool m_isExploded = false;
	int m_damage = 0;
};
