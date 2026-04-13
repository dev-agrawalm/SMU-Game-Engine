#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class Game;
class XboxController;

class SpaceShip : public Entity
{
	static const Rgba8 SPACESHIP_BLUE;
	static const Rgba8 SPACESHIP_THRUST_ORANGE;
public:
	SpaceShip(); //does nothing
	SpaceShip(Game* game, Vec2 const& position, XboxController const* controller);
	~SpaceShip(); //does nothing

	virtual void	Update(float deltaSeconds) override;
	virtual void	Render() const override;
	virtual void	Die() override;
	void			OnCollisionWithWorld();
	void			Respawn(Vec2 const& position);
	void			FireBullet();
	void			LaunchBomb();
	void			Deaccelerate();

	//Getters
	int				GetLives();
	Vec2			GetWorldFiringPosition() const;
	virtual Rgba8	GetDebrisColor() const override;
	bool			IsSlowingDown() const;
protected:
	virtual void	InitLocalVertexes() override;

private:
	void			CheckInput(float deltaSeconds);
	void			CheckInputFromController(float deltaSeconds);
	void			CheckInputFromKeyboard(float deltaSeconds);
	void			UpdateShip(float deltaSeconds);

private:
	Rgba8		m_shipColor;
	int			m_numLives = 0;
	int			m_numBombs = 0;
	Vec2		m_localFiringPosition;
	float		m_timeSinceLastBulletFired = 0.0f;

	float		m_invincibilityTime = 0.0f;
	float		m_previousInvincibilityTime = 0.0f;
	float		m_deltaInvincibilityTime = 0.0f;
	float		m_invincibilityPhasingInterval = 0.0f;

	Vec2		m_playerAcceleration;
	bool		m_isSlowingDown = false;
	bool		m_isDeaccelerating = false;

	XboxController const* m_spaceshipController = nullptr;
	SoundPlaybackID m_slowDownSfxPlaybackId;
};