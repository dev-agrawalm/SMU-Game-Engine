#include "Game/Player.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Game.hpp"
#include "Game/Bullet.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/BackgroundStar.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/EventSystem.hpp"

Player::Player()
	: Entity()
{
	m_faction			= ENTITY_FACTION_PLAYER;
	m_position			= g_gameConfigBlackboard.GetValue("playerPos",				Vec2::ZERO);
	m_cosmeticRadius	= g_gameConfigBlackboard.GetValue("playerCosmeticRadius",	1.0f);
	m_physicsRadius		= g_gameConfigBlackboard.GetValue("playerPhysicsRadius",	0.9f);
	m_health			= g_gameConfigBlackboard.GetValue("playerHealth",			15);
	m_orientationDegrees = 90.0f;
	m_acceleration		= g_gameConfigBlackboard.GetValue("playerAcceleration",		30.0f);
	m_sidewaysMovementDrag	= g_gameConfigBlackboard.GetValue("playerSidewaysMovementDrag",	10.0f);
	std::string weapon1 = g_gameConfigBlackboard.GetValue("playerWeapon1", "blaster");
	std::string weapon2 = g_gameConfigBlackboard.GetValue("playerWeapon2", "shotgun");
	std::string weapon3 = g_gameConfigBlackboard.GetValue("playerWeapon3", "shotgun");
	WeaponDefinition const* weaponDef1 = WeaponDefinition::GetWeaponDefinitionByName(weapon1);
	WeaponDefinition const* weaponDef2 = WeaponDefinition::GetWeaponDefinitionByName(weapon2);
	WeaponDefinition const* weaponDef3 = WeaponDefinition::GetWeaponDefinitionByName(weapon3);
	
	if (weaponDef1 != nullptr)
	{
		Weapon* weapon = new Weapon(weaponDef1, this);
		m_weapons.push_back(weapon);
	}

	if (weaponDef2 != nullptr)
	{
		Weapon* weapon = new Weapon(weaponDef2, this);
		m_weapons.push_back(weapon);
	}

	if (weaponDef3 != nullptr)
	{
		Weapon* weapon = new Weapon(weaponDef3, this);
		m_weapons.push_back(weapon);
	}
	m_activeWeaponIndex = 0;
}


Player::~Player()
{
	for (int weaponIndex = 0; weaponIndex < (int) m_weapons.size(); weaponIndex++)
	{
		Weapon*& weapon = m_weapons[weaponIndex];
		if (weapon)
		{
			delete weapon;
			weapon = nullptr;
		}
	}
}


void Player::Update(float deltaSeconds)
{
	if (IsDead())
	{
		m_timeSinceDeath += deltaSeconds;

		static float deathAnimationDuration = g_gameConfigBlackboard.GetValue("playerDeathAnimationDuration", 2.0f);
		if (m_timeSinceDeath > deathAnimationDuration + 0.7f)
		{
			g_eventSystem->FireEvent("PlayerDeathAnimationFinishedEvent");
		}

		return;
	}

	CheckInput(deltaSeconds);
	Vec2 acceleration = m_acceleration * m_movementDirection;
	acceleration -= m_velocity * m_sidewaysMovementDrag;
	m_velocity += acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;

	static Vec2 bgStarDefaultVelocity = g_gameConfigBlackboard.GetValue("bgStarVelocity", Vec2(0.0f, -10.0f));
	Vec2 starsVelocity = bgStarDefaultVelocity;
	int relativeVelocityDirMultiplier = m_velocity.x > 0.0f ? -1 : 1;
	float t = m_velocity.GetLength() / 20.0f;
	t = ClampZeroToOne(t);
	float ease = SmoothStop4(t);
	starsVelocity.x += Lerp(0.0f, 5.0f, ease) * relativeVelocityDirMultiplier;
	BackgroundStar::s_velocity = starsVelocity;

	static float worldX = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
	static float worldY = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);
	m_position.x = Clamp(m_position.x, m_cosmeticRadius, worldX - m_cosmeticRadius);
	m_position.y = Clamp(m_position.y, m_cosmeticRadius, worldY - m_cosmeticRadius);


	Weapon* activeWeapon = GetActiveWeapon();
	if (activeWeapon != nullptr)
	{
		std::string activeWeaponString = Stringf("Active Weapon: %s", activeWeapon->GetWeaponName().c_str());
		DebugAddScreenText(activeWeaponString, Vec2(0.0f, 0.0f), 0.0f, Vec2(0.0f, 0.0f), 7.0f);
	}
}


void Player::Render() const
{
	if (g_game->IsDebugModeActive())
	{
		DebugRender();
	}
	
	Vec2 uvMins;
	Vec2 uvMaxs;
	if (IsDead())
	{
		static float deathAnimationDuration = g_gameConfigBlackboard.GetValue("playerDeathAnimationDuration", 2.0f);
		if (m_timeSinceDeath > deathAnimationDuration)
		{
			return;
		}

		static SpriteAnimDefinition deathAnim = SpriteAnimDefinition(*g_gameSpriteSheet, 3, 6, deathAnimationDuration, SpriteAnimPlaybackMode::ONCE);
		SpriteDefinition deathAnimSpriteDef = deathAnim.GetSpriteDefAtTime(m_timeSinceDeath);
		deathAnimSpriteDef.GetUVs(uvMins, uvMaxs);
	}
	else
	{
		float thresholdValue = 5.0f;
		if (m_velocity.x >= -thresholdValue && m_velocity.x <= thresholdValue)
		{
			g_gameSpriteSheet->GetSpriteUVs(uvMins, uvMaxs, IntVec2(0, 0));
		}
		else if (m_velocity.x < -thresholdValue)
		{
			g_gameSpriteSheet->GetSpriteUVs(uvMins, uvMaxs, IntVec2(1, 0));
		}
		else if (m_velocity.x > thresholdValue)
		{
			g_gameSpriteSheet->GetSpriteUVs(uvMins, uvMaxs, IntVec2(2, 0));
		}
	}

	static float playerSize = g_gameConfigBlackboard.GetValue("playerSize", 10.0f);
	AABB2 playerSpriteBox = AABB2(Vec2::ZERO, playerSize, playerSize);
	std::vector<Vertex_PCU> playerVerts;
	AddVertsForAABB2ToVector(playerVerts, playerSpriteBox, Rgba8::WHITE, uvMins, uvMaxs);
	TransformVertexArrayXY3D((int) playerVerts.size(), playerVerts.data(), 1.0f, m_orientationDegrees, m_position);
	
	Texture const& spriteSheetTexture = g_gameSpriteSheet->GetTexture();
	g_theRenderer->BindTexture(0, &spriteSheetTexture);
	g_theRenderer->DrawVertexArray((int) playerVerts.size(), playerVerts.data());
}


void Player::TakeDamage(int damage)
{
	m_health -= damage;

	if (m_health <= 0)
	{
		Die();
		return;
	}

	static float hitSfxVolume = g_gameConfigBlackboard.GetValue("playerHitSfxVolume", 0.7f);
	ShakeScreenAndPlaySfx(0.15f, 0.25f, "Data/Audio/LaserImpact3_330629__stormwaveaudio__sci-fi-force-field-impact-15.wav", hitSfxVolume);
}


void Player::Die()
{
	m_isDead = true;
	m_timeSinceDeath = 0.0f;
	g_game->SetTimeScale(0.5f);

	static float deathSfxVolume = g_gameConfigBlackboard.GetValue("playerDeathSfxVolume", 0.7f);
	ShakeScreenAndPlaySfx(0.15f, 0.25f, "Data/Audio/ShipExplosion1_393374__eflexmusic__big-sci-fi-explosion-bomb-close-mixed.wav", deathSfxVolume, 1.75f);
}


void Player::CheckInput(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	m_movementDirection = Vec2::ZERO;
	if (g_inputSystem->IsKeyPressed('W'))
	{
		m_movementDirection += Vec2(0.0f, 1.0f);
	}

	if (g_inputSystem->IsKeyPressed('A'))
	{
		m_movementDirection += Vec2(-1.0f, 0.0f);
	}

	if (g_inputSystem->IsKeyPressed('S'))
	{
		m_movementDirection += Vec2(0.0f, -1.0f);
	}

	if (g_inputSystem->IsKeyPressed('D'))
	{
		m_movementDirection += Vec2(1.0f, 0.0f);
	}

	if (m_movementDirection.GetLength() > 0)
	{
		m_movementDirection.Normalize();
	}

	if (g_inputSystem->IsKeyPressed(KEYCODE_SPACE_BAR))
	{
		Attack();
	}

	if (g_inputSystem->WasKeyJustPressed('Q'))
	{
		m_activeWeaponIndex = GetPrevWeaponIndex();
	}

	if (g_inputSystem->WasKeyJustPressed('E'))
	{
		m_activeWeaponIndex = GetNextWeaponIndex();
	}
}


void Player::Attack()
{
	Weapon* activeWeapon = GetActiveWeapon();
	if (activeWeapon->Attack(m_orientationDegrees))
	{
		static float fireSfxVolume = g_gameConfigBlackboard.GetValue("playerFireSfxVolume", 0.7f);
		ShakeScreenAndPlaySfx(0.15f, 0.25f, "Data/Audio/PlayerFireSfx_542579__samsterbirdies__laser.wav", fireSfxVolume);
	}
}


int Player::GetNextWeaponIndex()
{
	int weaponListSize = (int) m_weapons.size();
	int nextWeaponIndex = m_activeWeaponIndex + 1;
	nextWeaponIndex %= weaponListSize;
	return nextWeaponIndex;
}


int Player::GetPrevWeaponIndex()
{
	int weaponListSize = (int) m_weapons.size();
	int prevWeaponIndex = m_activeWeaponIndex - 1;
	if (prevWeaponIndex < 0)
	{
		prevWeaponIndex = weaponListSize - 1;
	}
	return prevWeaponIndex;
}


Weapon* Player::GetActiveWeapon()
{
	if (m_activeWeaponIndex >= 0)
		return m_weapons[m_activeWeaponIndex];

	return nullptr;
}
