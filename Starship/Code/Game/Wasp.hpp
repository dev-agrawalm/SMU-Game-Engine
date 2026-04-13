#pragma once
#include "Game/Entity.hpp"

class Wasp : public Entity
{
public:
	Wasp();
	Wasp(Game* game, Vec2 const&  position, Entity* targetEntity);
	~Wasp();

	void Init(Game* game, Vec2 const& position, Entity* targetEntity);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void TakeDamage(int damage) override;

	virtual	Rgba8 GetDebrisColor() const override;
protected:
	virtual void InitLocalVertexes() override;
	void WarpAroundTheScreen();
private:
	Entity* m_targetEntity = nullptr;
	bool m_wasHit = false;
};