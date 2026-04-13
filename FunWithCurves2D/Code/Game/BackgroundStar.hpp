#pragma once
#include "Game/Entity.hpp"

class BackgroundStar : public Entity
{
public:
	static Vec2 s_velocity;

public:
	BackgroundStar(Vec2 const& position);
	void Update(float deltaSeconds) override;
	void Render() const override;

private:
	void InitLocalVerts();

private:
};