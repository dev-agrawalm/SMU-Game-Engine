#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <vector>
#include "Engine/Audio/AudioSystem.hpp"

struct Vertex_PCU;
class Texture;

class Prop : public Entity
{

public:
	Prop();
	~Prop();
	Prop(Vec3 const& position);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() override;
	virtual void DebugRender() const override;

	void SetVertexArray(std::vector<Vertex_PCU> const& vertices);
	void SetTint(Rgba8 const& tint);
	void SetTexture(Texture* texture);

	void InitLocalVerts();
private:
private:
	Texture* m_texture = nullptr;
	Rgba8 m_tint;
	bool m_showPuckControls = false;

	Vec3 m_velocityToApply = Vec3::ZERO;
	float m_angularVelocityToApply = 0.0f;

	SoundPlaybackID m_puckSlidePlaybackId = 0;
};
