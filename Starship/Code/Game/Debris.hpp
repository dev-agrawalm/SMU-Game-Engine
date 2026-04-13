#pragma once
#include "Game/Entity.hpp"

class RandomNumberGenerator;

class Debris : public Entity
{

public:
	Debris();
	Debris(Game* game, Vec2 const& position, Rgba8 color, Entity* fromEntity);
	Debris(Game* game, Vec2 const& position, Rgba8 color, Vec2 const& direction, float speed, float fadeoutTime, float minCosmetic = 0.05f, float maxCosmetic = 0.2f);
	~Debris();
	void Update(float deltaSeconds) override;
	void Render() const override;


	Rgba8 GetDebrisColor() const override;

protected:
	void InitLocalVertexes() override;
	void InitOrientationAngle(Entity* fromEntity);
protected:
	Rgba8 m_color;
	float m_timeAlive = 0.0f;
	float m_fadeOutTime = 0.0f;
	bool m_canBeOffscreen = false;
};