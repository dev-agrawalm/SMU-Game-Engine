#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class Texture;

class Explosion : public Entity
{
public:
	Explosion(Vec2 const& startingPosition, float orientationDegrees, EntityFaction faction, EntityType type, Map* map);
	~Explosion();

	void Update(float deltaSeconds) override;
	void Render() const override;

private:
	float m_aabb2SideLength = 0.0f;
	float m_lifetime = 0.0f;
	float m_duration = 0.0f;
	SpriteAnimDefinition* m_explosionAnimDef = nullptr;
};