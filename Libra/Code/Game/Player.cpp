#include "Game/Player.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Tile.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

Player::Player(Vec2 const& position, EntityFaction faction, EntityType type, Map* map) 
	: Entity(position, 0.0f, faction, type, map)
{
	m_cosmeticRadius					= g_gameConfigBlackboard.GetValue("playerCosmeticRadius", m_cosmeticRadius);
	m_physicsRadius						= g_gameConfigBlackboard.GetValue("playerPhysicsRadius", m_physicsRadius);
	m_speed								= g_gameConfigBlackboard.GetValue("playerSpeed", m_speed);
	m_angularVelocity					= g_gameConfigBlackboard.GetValue("playerTankAngularVelocity", m_angularVelocity);
	m_turretLength						= g_gameConfigBlackboard.GetValue("playerTurretLength", m_turretLength);
	m_turretLocalPosition				= g_gameConfigBlackboard.GetValue("playerTurretLocalPosition", m_turretLocalPosition);
	m_turretLocalOrientationDegrees		= g_gameConfigBlackboard.GetValue("playerTurretLocalOrientationDegrees", m_turretLocalOrientationDegrees);
	m_turretAngularVelocity				= g_gameConfigBlackboard.GetValue("playerTurretAngularVelocity", m_turretAngularVelocity);
	m_turretCooldownTimeSeconds			= g_gameConfigBlackboard.GetValue("playerTurretCooldownTimeSeconds", m_turretCooldownTimeSeconds);
	m_turretCooldownTimer				= 0.0f;
	m_health							= g_gameConfigBlackboard.GetValue("playerHealth", m_health);
	m_maxHealth							= m_health;
}


Player::~Player()
{
}


void Player::CheckInputForTank(float deltaSeconds)
{
	CheckKeyboardInputForTank(deltaSeconds);
	CheckControllerInputForTank(deltaSeconds);
}


void Player::CheckKeyboardInputForTank(float deltaSeconds)
{
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		m_isPushedByWalls = !m_isPushedByWalls;
	}

	IntVec2 gridDirection;
	if (g_inputSystem->IsKeyPressed('E'))
	{
		gridDirection += NORTH;
	}
	else if (g_inputSystem->IsKeyPressed('D'))
	{
		gridDirection += SOUTH;
	}

	if (g_inputSystem->IsKeyPressed('S'))
	{
		gridDirection += WEST;
	}
	else if (g_inputSystem->IsKeyPressed('F'))
	{
		gridDirection += EAST;
	}

	Vec2 orientationVector = Vec2((float) gridDirection.x, (float) gridDirection.y);
	if (orientationVector.GetLengthSquared() > 0.0f)
	{
		orientationVector.Normalize();
		float targetOrientationDegrees	= orientationVector.GetOrientationDegrees();
		m_orientationDegrees			= GetTurnedTowardDegrees(m_orientationDegrees, targetOrientationDegrees, m_angularVelocity * deltaSeconds);
		m_velocity						= GetForwardNormal() * m_speed;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F6))
	{
		m_godMode = !m_godMode;
	}
}


void Player::CheckControllerInputForTank(float deltaSeconds)
{
	XboxController playerController = g_inputSystem->GetController(0);
	float leftJoystickMagnitude		= playerController.GetLeftStick().GetMagnitude();
	float leftJoystickOrientation	= playerController.GetLeftStick().GetOrientationDegrees();

	if (leftJoystickMagnitude > 0.0f)
	{
		float targetOrientationDegrees	= leftJoystickOrientation;
		m_orientationDegrees			= GetTurnedTowardDegrees(m_orientationDegrees, targetOrientationDegrees, m_angularVelocity * deltaSeconds);
		m_velocity						= GetForwardNormal() * m_speed * leftJoystickMagnitude;
	}

	if (playerController.WasButtonJustPressed(XboxController::XBOX_BUTTON_Y))
	{
		m_godMode = !m_godMode;
	}
}


void Player::CheckInputForTurret(float deltaSeconds)
{
	CheckKeyboardInputForTurret(deltaSeconds);
	CheckControllerInputForTurret(deltaSeconds);
}


void Player::FireTurret()
{
	if (m_turretCooldownTimer <= 0.0f)
	{
		m_turretCooldownTimer			= m_turretCooldownTimeSeconds;
		float turretWorldOrientation	= m_turretLocalOrientationDegrees + m_orientationDegrees;
		Vec2 turretWorldPosition		= m_position + m_turretLocalPosition;
		Vec2 turretMuzzlePosition		= turretWorldPosition + Vec2::MakeFromPolarDegrees(turretWorldOrientation, m_turretLength);
		SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
		g_audioSystem->StartSound(sfxId, false, 0.3f);
		m_map->SpawnEntityOfType(ENTITY_TYPE_GOOD_BULLET, turretMuzzlePosition, turretWorldOrientation);
		Vec2 turretForwardDirection = Vec2::MakeFromPolarDegrees(turretWorldOrientation);
		Vec2 muzzleFlashPosition = turretMuzzlePosition - turretForwardDirection * 0.1f;
		m_map->SpawnEntityOfType(ENTITY_TYPE_BULLET_EXPLOSION, muzzleFlashPosition, 0.0f);
	}
}


void Player::CheckKeyboardInputForTurret(float deltaSeconds)
{
	IntVec2 gridDirection;
	if (g_inputSystem->IsKeyPressed('I'))
	{
		gridDirection += NORTH;
	}
	else if (g_inputSystem->IsKeyPressed('K'))
	{
		gridDirection += SOUTH;
	}

	if (g_inputSystem->IsKeyPressed('J'))
	{
		gridDirection += WEST;
	}
	else if (g_inputSystem->IsKeyPressed('L'))
	{
		gridDirection += EAST;
	}

	Vec2 orientationVector = Vec2((float) gridDirection.x, (float) gridDirection.y);
	if (orientationVector.GetLengthSquared() > 0.0f)
	{
		float turretWorldOrientationDegrees;
		float targetOrientationDegrees	= orientationVector.GetOrientationDegrees();
		turretWorldOrientationDegrees	= m_orientationDegrees + m_turretLocalOrientationDegrees; 
		turretWorldOrientationDegrees	= GetTurnedTowardDegrees(turretWorldOrientationDegrees, targetOrientationDegrees, m_turretAngularVelocity * deltaSeconds);
		m_turretLocalOrientationDegrees	= turretWorldOrientationDegrees - m_orientationDegrees;
	}

	if (g_inputSystem->IsKeyPressed(KEYCODE_SPACE_BAR))
	{
		FireTurret();
	}
}


void Player::CheckControllerInputForTurret(float deltaSeconds)
{
	XboxController playerController = g_inputSystem->GetController(0);
	float rightJoystickMagnitude	= playerController.GetRightStick().GetMagnitude();
	float rightJoystickOrientation	= playerController.GetRightStick().GetOrientationDegrees();

	if (rightJoystickMagnitude > 0.0f)
	{
		float turretWorldOrientationDegrees;
		float targetOrientationDegrees	= rightJoystickOrientation;
		turretWorldOrientationDegrees	= m_orientationDegrees + m_turretLocalOrientationDegrees;
		turretWorldOrientationDegrees	= GetTurnedTowardDegrees(turretWorldOrientationDegrees, targetOrientationDegrees, m_turretAngularVelocity * deltaSeconds);
		m_turretLocalOrientationDegrees = turretWorldOrientationDegrees - m_orientationDegrees;
	}

	if (playerController.GetRightTrigger() == 1.0f)
	{
		FireTurret();
	}
}


void Player::CheckInput(float deltaSeconds)
{	
	CheckInputForTank(deltaSeconds);
	CheckInputForTurret(deltaSeconds);
}


void Player::Update(float deltaSeconds)
{
	if(m_isDead)
		return;

	m_velocity = Vec2(0.0f, 0.0f);

	CheckInput(deltaSeconds);
	
	m_position += m_velocity * deltaSeconds;
	m_turretCooldownTimer -= deltaSeconds;
}


void Player::Render() const
{
	if (g_debugMode)
	{
		DebugRender();
	}
	
	if (m_isDead)
		return;

	AABB2 localSpaceBoundingBox = AABB2(-0.5f,-0.5, 0.5f,0.5f);

	std::vector<Vertex_PCU> tankVertexes;
	Texture* tankTexture = g_theRenderer->CreateOrGetTexture("Data/Images/PlayerTankBase.png");
	AddVertsForAABB2ToVector(tankVertexes, localSpaceBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) tankVertexes.size(), tankVertexes.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(0, tankTexture);
	g_theRenderer->DrawVertexArray((int) tankVertexes.size(), tankVertexes.data());

	std::vector<Vertex_PCU> turretVertexes;
	Texture* turretTexture			= g_theRenderer->CreateOrGetTexture("Data/Images/PlayerTankTop.png");
	Vec2 turretWorldPosition		= m_position + m_turretLocalPosition;
	float turretWorldOrientation	= m_turretLocalOrientationDegrees + m_orientationDegrees;
	AddVertsForAABB2ToVector(turretVertexes, localSpaceBoundingBox, Rgba8::WHITE);
	TransformVertexArrayXY3D((int) turretVertexes.size(), turretVertexes.data(), 1.0f, turretWorldOrientation, turretWorldPosition);
	g_theRenderer->BindTexture(0, turretTexture);
	g_theRenderer->DrawVertexArray((int) turretVertexes.size(), turretVertexes.data());
}


void Player::Die()
{
	SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	g_audioSystem->StartSound(sfxId, false, 0.4f);
	m_isDead = true;

	float randomOrientation = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
	m_map->SpawnEntityOfType(ENTITY_TYPE_PLAYER_EXPLOSION, m_position, randomOrientation);
}


void Player::TakeDamage(int damage)
{
	if(m_godMode)
		return;

	SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	g_audioSystem->StartSound(sfxId, false, 0.05f);

	m_health -= damage;
	if (m_health <= 0)
	{
		Die();
	}
}