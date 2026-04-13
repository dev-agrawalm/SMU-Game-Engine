#include "Game/SpaceShip.hpp"
#include "Game/GameCommon.hpp"
#include "ENgine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"

const Rgba8 SpaceShip::SPACESHIP_BLUE	= Rgba8(102, 153, 204, 255);
const Rgba8 SpaceShip::SPACESHIP_THRUST_ORANGE		= Rgba8(255, 100, 0, 255);

SpaceShip::SpaceShip(Game* game, Vec2 const&  position, XboxController const* controller) : Entity(game, position)
{
	m_shipColor = SpaceShip::SPACESHIP_BLUE;
	m_numBombs = PLAYER_SHIP_NUM_BOMBS;
	m_angularVelocity = PLAYER_SHIP_TURN_SPEED;
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_localFiringPosition = Vec2(1.0f, 0.0f);
	m_timeSinceLastBulletFired = PLAYER_SHIP_FIRE_RATE;
	m_invincibilityTime = PLAYER_SHIP_INVINCIBILITY_TIME;
	m_previousInvincibilityTime = m_invincibilityTime;
	m_invincibilityPhasingInterval = 0.05f;

	m_spaceshipController = controller;

	m_numLives = PLAYER_SHIP_LIVES;
	m_numLives--; //Subtracting a life on start up because you use a life to start up

	InitLocalVertexes();
}

SpaceShip::SpaceShip() : Entity()
{
	//empty
}

SpaceShip::~SpaceShip()
{
	m_spaceshipController = nullptr;
}

void SpaceShip::Update(float deltaSeconds)
{
	if (m_isDead)
		return;

	if (m_isDeaccelerating)
	{
		float speed = m_velocity.GetLength() * 1.9f;
		float orientation = m_velocity.GetOrientationDegrees();
		float orientationDelta = g_rng->GetRandomFloatInRange(-2.5f, 2.5f);
		orientation += orientationDelta;
		float fadeoutTime = RangeMap(speed, 0.0f, PLAYER_SHIP_MAX_SPEED, 0.3f, 0.6f);
		fadeoutTime = g_rng->GetRandomFloatInRange(0.35f, fadeoutTime);
		m_game->SpawnParticles(m_position, 75, Rgba8::GREY, Vec2::MakeFromPolarDegrees(orientation), speed, fadeoutTime, 0.3f, 0.6f);
		m_game->SetWorldCamScreenShakeParameters(0.05f, 0.1f);
	}

	m_acceleration = 0.0f;
	m_playerAcceleration = Vec2::ZERO;
	m_isSlowingDown = false;
	m_isDeaccelerating = false;
	CheckInput(deltaSeconds);
	UpdateShip(deltaSeconds);
	m_timeSinceLastBulletFired += deltaSeconds;
	
	m_invincibilityTime -= deltaSeconds;
	m_deltaInvincibilityTime = m_previousInvincibilityTime - m_invincibilityTime;
	if (m_deltaInvincibilityTime >= m_invincibilityPhasingInterval)
	{
		m_previousInvincibilityTime = m_invincibilityTime;
		m_shipColor = m_shipColor == Rgba8::BLACK ? SpaceShip::SPACESHIP_BLUE : Rgba8::BLACK;
	}

}

void SpaceShip::CheckInput(float deltaSeconds)
{
	CheckInputFromController(deltaSeconds);
	CheckInputFromKeyboard(deltaSeconds);
}

void SpaceShip::CheckInputFromController(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (!m_spaceshipController->IsConnected())
		return;

	if (m_spaceshipController->IsButtonDown(XboxController::XBOX_BUTTON_A) && 
		m_timeSinceLastBulletFired >= PLAYER_SHIP_FIRE_RATE)
	{
		FireBullet();
	}

	if (m_spaceshipController->WasButtonJustPressed(XboxController::XBOX_BUTTON_X))
	{
		SoundID slowDownSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/FireExtinguisherTest1_looped.wav");
		m_slowDownSfxPlaybackId = g_audioSystem->StartSound(slowDownSfxId, true, 1.0f, 0.0f, 1.0f);
	}

	if (m_spaceshipController->IsButtonDown(XboxController::XBOX_BUTTON_X))
	{
		Deaccelerate();
	}

	if (m_spaceshipController->WasButtonJustReleased(XboxController::XBOX_BUTTON_X))
	{
		g_audioSystem->StopSound(m_slowDownSfxPlaybackId);
	}

	if (m_spaceshipController->IsButtonDown(XboxController::XBOX_BUTTON_LEFT_BUMPER))
	{
		m_isSlowingDown = true;
	}

	float thrustFraction;
	AnalogJoystick const& movementJoystick = m_spaceshipController->GetLeftStick();
	thrustFraction = movementJoystick.GetMagnitude();
	m_orientationDegrees = thrustFraction > 0.05f ? movementJoystick.GetOrientationDegrees() : m_orientationDegrees;
	//m_acceleration = Lerp(0.0f, PLAYER_SHIP_ACCELRATION, thrustFraction);
}

void SpaceShip::CheckInputFromKeyboard(float deltaSeconds)
{
	//apply thrust to the ship when player presses E
	if (g_inputSystem->IsKeyPressed('W'))
	{
		//m_acceleration = PLAYER_SHIP_ACCELRATION;
	}

	bool isKeyDownS = g_inputSystem->IsKeyPressed('A');
	bool isKeyDownF = g_inputSystem->IsKeyPressed('D');

	if (!isKeyDownF || !isKeyDownS)
	{
		//rotate counter clockwise
		if (isKeyDownS)
		{
			m_orientationDegrees += m_angularVelocity * deltaSeconds;
		}

		//rotate clockwise
		if (isKeyDownF)
		{
			m_orientationDegrees -= m_angularVelocity * deltaSeconds;
		}
	}

	//shoot a bullet
	if (g_inputSystem->IsKeyPressed(KEYCODE_SPACE_BAR) && 
		m_timeSinceLastBulletFired >= PLAYER_SHIP_FIRE_RATE)
	{
		FireBullet();
	}


	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SHIFT))
	{
		SoundID slowDownSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/FireExtinguisherTest1_looped.wav");
		m_slowDownSfxPlaybackId = g_audioSystem->StartSound(slowDownSfxId, true, 1.0f, 0.0f, 1.0f);
	}

	if (g_inputSystem->IsKeyPressed(KEYCODE_SHIFT))
	{
		Deaccelerate();
	}

	if (g_inputSystem->WasKeyJustReleased(KEYCODE_SHIFT))
	{
		g_audioSystem->StopSound(m_slowDownSfxPlaybackId);
	}

	if (g_inputSystem->IsKeyPressed(KEYCODE_CTRL))
	{
		m_isSlowingDown = true;
	}
}



void SpaceShip::UpdateShip(float deltaSeconds)
{
	//update velocity and position
	Vec2 m_thrustDirection = GetForwardNormal();
	//m_velocity += /*m_thrustDirection * m_acceleration*/m_playerAcceleration * deltaSeconds;
	m_velocity.ClampLength(PLAYER_SHIP_MAX_SPEED);
	m_position += m_velocity * deltaSeconds;

	if (m_game->CheckCollisionWithWorldBoundary(this))
	{
		OnCollisionWithWorld();
		size_t shipBounceSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/Bounce.wav");
		g_audioSystem->StartSound(shipBounceSfxId, false, 0.1f);
	}
}

void SpaceShip::Render() const
{
	if (m_isDead)
		return;

	Vertex_PCU thrustFlameTriangle[3];
	Vertex_PCU tempVerts[NUM_SHIP_VERTS];
	
	for (int index = 0; index < NUM_SHIP_VERTS; index++)
	{
		tempVerts[index] = m_localVertexes[index];

		//flicker animation for ship's invincibility state
		if (m_invincibilityTime > 0.0f)
		{
			tempVerts[index].m_color = m_shipColor;
		}
	}

	if (m_acceleration > 0.0f)
	{
		float thrustFlameLength = ( RangeMap(m_acceleration, 0.0f, PLAYER_SHIP_ACCELRATION, 0.0f, PLAYER_SHIP_THRUST_FLAME_MAX_LENGTH) * 
								   g_rng->GetRandomFloatInRange(0.5f,1.0f) );
		thrustFlameTriangle[0].m_position = Vec3(-2.0f, -1.0f);
		thrustFlameTriangle[1].m_position = Vec3(-2.0f, 1.0f);
		thrustFlameTriangle[2].m_position = Vec3(-2.0f - thrustFlameLength, 0.0f);
		thrustFlameTriangle[0].m_color = SpaceShip::SPACESHIP_THRUST_ORANGE;
		thrustFlameTriangle[1].m_color = SpaceShip::SPACESHIP_THRUST_ORANGE;
		thrustFlameTriangle[2].m_color = SpaceShip::SPACESHIP_THRUST_ORANGE;
		TransformVertexArrayXY3D(3, thrustFlameTriangle, 1.0f, m_orientationDegrees, m_position);
		g_theRenderer->DrawVertexArray(3, thrustFlameTriangle);
	}

	TransformVertexArrayXY3D(NUM_SHIP_VERTS, tempVerts, 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, tempVerts);

	Vec2 aimIndicatorStartPos = GetWorldFiringPosition() + GetForwardNormal() * 0.2f;
	float aimIndicatorLineLength = 3.0f;
	Vec2 aimIndicatorLine = Vec2::MakeFromPolarDegrees(m_orientationDegrees, aimIndicatorLineLength);
	Rgba8 aimIndicatorLineColor = Rgba8(255, 255, 255, 100);
	DrawDottedLine(aimIndicatorStartPos, aimIndicatorLine, 0.25f, aimIndicatorLineColor, 5);
}

void SpaceShip::Die()
{
	if (m_invincibilityTime > 0.0f)
		return;

	m_isDead = true;
	size_t playerDeathSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/PlayerDied.wav");
	g_audioSystem->StartSound(playerDeathSfxId, false, 0.3f);
	m_game->SetWorldCamScreenShakeParameters(PLAYER_SHIP_DEATH_SCREEN_SHAKE_MAGNITUDE, PLAYER_SHIP_DEATH_SCREEN_SHAKE_DURATION);
}

void SpaceShip::OnCollisionWithWorld()
{
	if ((WORLD_SIZE_X - m_position.x) <= m_physicsRadius)
	{
		m_position.x = WORLD_SIZE_X - m_physicsRadius;
		m_velocity.x = -m_velocity.x;
	}
	else if ((m_position.x - 0.0f) <= m_physicsRadius)
	{
		m_position.x = 0.0f + m_physicsRadius;
		m_velocity.x = -m_velocity.x;
	}
	
	if ((WORLD_SIZE_Y - m_position.y) <= m_physicsRadius)
	{
		m_position.y = WORLD_SIZE_Y - m_physicsRadius;
		m_velocity.y = -m_velocity.y;
	}
	else if ((m_position.y - 0.0f) <= m_physicsRadius)
	{
		m_position.y = 0.0f + m_physicsRadius;
		m_velocity.y = -m_velocity.y;
	}
}

void SpaceShip::Respawn(Vec2 const& spawnPosition)
{
	if (m_numLives > 0)
	{
		m_isDead = false;
		m_position = spawnPosition;
		m_orientationDegrees = 0.0f;
		m_velocity = Vec2(0.0f, 0.0f);
		m_acceleration = 0.0f;
		m_shipColor = SpaceShip::SPACESHIP_BLUE;
		m_invincibilityTime = PLAYER_SHIP_INVINCIBILITY_TIME;
		m_previousInvincibilityTime = m_invincibilityTime;
		m_numBombs = PLAYER_SHIP_NUM_BOMBS;
		m_numLives--;

		size_t respawnSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/Respawn.wav");
		g_audioSystem->StartSound(respawnSfxId, false, 0.5f);
	}
}

void SpaceShip::FireBullet()
{
	size_t bulletFireSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
	g_audioSystem->StartSound(bulletFireSfxId, false, 0.8f);
	m_game->FireBullet(GetWorldFiringPosition(), m_orientationDegrees);
	
	if (!m_isSlowingDown)
	{
		m_velocity -= Vec2::MakeFromPolarDegrees(m_orientationDegrees, BULLET_RECOIL * 2.0f);
	}
	//m_playerAcceleration -= Vec2::MakeFromPolarDegrees(m_orientationDegrees, BULLET_RECOIL * 2.0f);
	//m_acceleration = BULLET_RECOIL;
	//m_playerAcceleration = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	//m_position -= Vec2::MakeFromPolarDegrees(m_orientationDegrees, BULLET_RECOIL);
	m_timeSinceLastBulletFired = 0.0f;
	m_game->SetWorldCamScreenShakeParameters(BULLET_SPAWN_SCREEN_SHAKE_MAGNITUDE, BULLET_SPAWN_SCREEN_SHAKE_DURATION);
}

void SpaceShip::LaunchBomb()
{
	if (m_numBombs > 0)
	{
		size_t bombLaunchSfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
		g_audioSystem->StartSound(bombLaunchSfxId, false, 0.5f);
		m_game->SpawnBomb(m_position);
		m_numBombs--;
	}
}


void SpaceShip::Deaccelerate()
{
	m_isDeaccelerating = true;
	float speed = m_velocity.GetLength();
	float slowDownPercentage = 0.99f;
	m_velocity.SetLength(speed * slowDownPercentage);
}


int SpaceShip::GetLives()
{
	return m_numLives;
}

Vec2 SpaceShip::GetWorldFiringPosition() const
{
	Vec2 firingPos = m_localFiringPosition;

	TransformPos2D(firingPos, 1.0f, m_orientationDegrees, m_position);

	return firingPos;
}

Rgba8 SpaceShip::GetDebrisColor() const
{
	return SpaceShip::SPACESHIP_BLUE;
}


bool SpaceShip::IsSlowingDown() const
{
	return m_isSlowingDown;
}


void SpaceShip::InitLocalVertexes()
{
	m_localVertexes = new Vertex_PCU[NUM_SHIP_VERTS];

	for (int index = 0; index < NUM_SHIP_VERTS; index++)
	{
		m_localVertexes[index].m_color = SpaceShip::SPACESHIP_BLUE;
	}

	//left wing triangle
	m_localVertexes[0].m_position = Vec3(2.0f,1.0f);
	m_localVertexes[1].m_position = Vec3(0.0f,2.0f);
	m_localVertexes[2].m_position = Vec3(-2.0f,1.0f);
	//backside top left triangle
	m_localVertexes[3].m_position = Vec3(0.0f,1.0f);
	m_localVertexes[4].m_position = Vec3(-2.0f,1.0f);
	m_localVertexes[5].m_position = Vec3(-2.0f,-1.0f);
	//backside bottom right triangle
	m_localVertexes[6].m_position = Vec3(0.0f,1.0f);
	m_localVertexes[7].m_position = Vec3(-2.0f,-1.0f);
	m_localVertexes[8].m_position = Vec3(0.0f,-1.0f);
	//right wing triangle
	m_localVertexes[9].m_position = Vec3(-2.0f,-1.0f);
	m_localVertexes[10].m_position = Vec3(0.0f,-2.0f);
	m_localVertexes[11].m_position = Vec3(2.0f,-1.0f);
	//front nose triangle
	m_localVertexes[12].m_position = Vec3(1.0f,0.0f); 
	m_localVertexes[13].m_position = Vec3(0.0f,1.0f);
	m_localVertexes[14].m_position = Vec3(0.0f,-1.0f);
}
