#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>

struct Vertex_PCU;
class Texture;

class Prop : public Entity
{

public:
	Prop();
	~Prop();
	Prop(Vec3 const& position);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

	void SetVertexArray(std::vector<Vertex_PCU> const& vertices);
	void SetAngularVelocity(Vec3 const& angularVelocity);
	void SetTint(Rgba8 const& tint);
	void SetTexture(Texture* texture);
private:
	Vec3 m_angularVelocity;
	std::vector<Vertex_PCU> m_localVertexes;
	Texture* m_texture = nullptr;
	Rgba8 m_tint;
};
