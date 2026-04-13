#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class Player : public Entity
{
public:
	Player();
	virtual ~Player();
	Player(Vec3 const& position);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() override;
	virtual void DebugRender() const override;
	void Initialise();

public:
	Rgba8 m_baseColor;
	Rgba8 m_handleColor;
};
