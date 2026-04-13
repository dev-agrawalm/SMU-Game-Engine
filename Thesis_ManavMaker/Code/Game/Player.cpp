#include "Game/Player.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Level.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/TriggerSystem.hpp"

//unsigned char PLAYER_JUMP_KEY = 'W';

// Player::Player(Level* level, Vec2 const& position)
// 	: Entity(level, position)
// {
// 	m_collidesWithTiles = true;
// 	m_type = ENTITY_TYPE_MARIO;
// 	m_localPhysicsBounds = GetPhysicsBoundsForEntity(m_type);
// 	m_localCosmeticBounds = GetCosmeticBoundsForEntity(m_type);
// 	m_acceleration = Vec2::ZERO;
// 	m_health = 3;
// 
// 	Vec2 halfDimsPhysics = m_localPhysicsBounds.GetDimensions() * 0.5f;
// 	m_localBottomLeft = Vec2(-halfDimsPhysics.x, 0.0f);
// 	m_localBottomRight = Vec2(halfDimsPhysics.x, 0.0f);
// 
// 	m_invulnerabiiltyDuration = g_gameConfigBlackboard.GetValue("playerInvulnerabilityDuration", 1.0f);
// 	m_invulnerabiltyIndicatorTimer.SetDuration((float) g_gameConfigBlackboard.GetValue("playerInvulnerableIndicatorTimeInterval", 0.3f));
// 
// // 	EntitySpawnInfo spawnInfo = EntitySpawnInfo(ENTITY_TYPE_GOOMBA);
// // 	//spawnInfo.m_type = ENTITY_TYPE_GOOMBA;
// // 	spawnInfo.m_startingPosition = m_bottomCenterPosition + Vec2(1.0f, 0.0f) * 5.0f;
// // 	TriggerResponse onSpawnTriggerResponse = {};
// // 	onSpawnTriggerResponse.m_triggerType = TRIGGER_TYPE_ON_SPAWN;
// // 	onSpawnTriggerResponse.m_triggerResponseCommand = new EntitySpawnTriggerCommand(m_level, spawnInfo);
// // 	m_triggerResponses[TRIGGER_TYPE_ON_SPAWN] = onSpawnTriggerResponse;
// }


Player::~Player()
{

}


void Player::CheckInput(float deltaSeconds)
{
	UNUSED(deltaSeconds);
// 	Vec2 movementDir = Vec2::ZERO;
// 
// 	static int jumpFrameAllowance = g_gameConfigBlackboard.GetValue("playerJumpFrameAllowance", 1);
// 	if (g_inputSystem->WasKeyJustPressed(PLAYER_JUMP_KEY) && (m_isGrounded || g_currentFrameNumber - m_lostGroundOnFrame <= (unsigned int) jumpFrameAllowance) )
// 	{
// 		m_isGrounded = false;
// 		m_lostGroundOnFrame = 0;
// 		SoundID playerJumpSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/PlayerJump.wav");
// 		g_audioSystem->StartSound(playerJumpSoundId);
// 		m_acceleration += PLAYER_JUMP_IMPULSE * Vec2(0.0f, 1.0f);
// 	}
// 
// 	if (g_inputSystem->IsKeyPressed(PLAYER_JUMP_KEY) && m_velocity.y > 0)
// 	{
// 		static float sustainThrust = g_gameConfigBlackboard.GetValue("playerJumpSustainThrust", PLAYER_JUMP_SUSTAIN_THRUST);
// 		m_acceleration += sustainThrust * Vec2(0.0f, 1.0f);
// 	}
// 
// 	if (g_inputSystem->IsKeyPressed('D'))
// 	{
// 		movementDir += Vec2(1.0f, 0.0f);
// 	}
// 
// 	if (g_inputSystem->IsKeyPressed('A'))
// 	{
// 		movementDir += Vec2(-1.0f, 0.0f);
// 	}
// 
// 	if (movementDir.GetLength() > 0)
// 	{
// 		float acceleration = PLAYER_WALKING_ACCELERATION;
// 		if (g_inputSystem->IsKeyPressed(KEYCODE_SHIFT))
// 		{
// 			acceleration = PLAYER_SPRINTING_ACCELERATION;
// 		}
// 		m_acceleration += acceleration * movementDir;
// 	}
// 
// 	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F9))
// 	{
// 		m_bottomCenterPosition = Vec2(32.0f, 30.0f);
// 		m_velocity = Vec2::ZERO;
// 	}
// 	UNUSED(deltaSeconds);
}


// void Player::Update(float deltaSeconds)
// {
// 	if (m_isDead)
// 	{
// 		return;
// 	}
// 
// 	if (m_isGrounded)
// 	{
// 		float groundedThreshold = 0.1f;
// 		LevelRaycastResult bottomLeftRaycast = m_level->RaycastLevel(m_bottomCenterPosition + m_localBottomLeft, Vec2(0.0f, -1.0f), groundedThreshold, true);
// 		LevelRaycastResult bottomRightRaycast = m_level->RaycastLevel(m_bottomCenterPosition + m_localBottomRight, Vec2(0.0f, -1.0f), groundedThreshold, true);
// 		LevelRaycastResult bottomCenterRaycast = m_level->RaycastLevel(m_bottomCenterPosition, Vec2(0.0f, -1.0f), groundedThreshold, true);
// 		if (!bottomLeftRaycast.m_didHit && !bottomCenterRaycast.m_didHit && !bottomRightRaycast.m_didHit)
// 		{
// 			if (m_isGrounded)
// 			{
// 				m_lostGroundOnFrame = g_currentFrameNumber;
// 			}
// 			m_isGrounded = false;
// 		}
// 
// 		IntVec2 bottomTileCoords = m_level->GetTileCoords(m_bottomCenterPosition + Vec2(0.0f, -groundedThreshold));
// 		Tile tileBelow = m_level->GetTile(bottomTileCoords);
// 		if (tileBelow.DoesDamage())
// 		{
// 			int damage = tileBelow.GetDamage();
// 			TakeDamage(damage);
// 		}
// 	}
// 
// 	CheckInput(deltaSeconds);
// 	CalculateWorldForces();
// 	MovePlayer(deltaSeconds);
// 	m_acceleration = Vec2::ZERO;
// 
// 	if (abs(m_velocity.x) > 0.0f)
// 	{
// 		if (!m_isWalking)
// 		{
// 			m_walkingAnimationTime = 0.0f;
// 		}
// 		m_isWalking = true;
// 	}
// 
// 	if (abs(m_velocity.x) < 1.0f)
// 	{
// 		m_isWalking = false;
// 	}
// 
// 	if (m_isWalking)
// 	{
// 		m_walkingAnimationTime += deltaSeconds;
// 	}
// 
// 	if (m_isInvulnerable)
// 	{
// 		float currentTime = (float) GetCurrentTimeSeconds();
// 		if (currentTime - m_invulnerabilityStartTime > m_invulnerabiiltyDuration)
// 		{
// 			m_isInvulnerable = false;
// 			m_isInvisible = false;
// 		}
// 
// 		if (m_invulnerabiltyIndicatorTimer.CheckAndDecrement())
// 		{
// 			m_isInvisible = !m_isInvisible;
// 		}
// 	}
// }

// 
// bool Player::TakeDamage(int damage)
// {
// 	if (Entity::TakeDamage(damage))
// 	{
// 		m_isInvulnerable = true;
// 		m_invulnerabilityStartTime = (float) GetCurrentTimeSeconds();
// 		m_invulnerabiltyIndicatorTimer.Restart();
// 		return true;
// 	}
// 
// 	return false;
// }


void Player::CollectCoins(int coinsToAdd)
{
	m_collectedCoinCount += coinsToAdd;
}


void Player::AddImpulseVelocity(Vec2 const& forceVec)
{
	m_acceleration += forceVec;
}


// void Player::Render() const
// {
// 	if (g_game->IsDebugModeActive())
// 	{
// 		DebugRender();
// 	}
// 
// 	if (m_isDead)
// 	{
// 		return;
// 	}
// 
// 	static float duration = 0.25f;
// 	static SpriteAnimDefinition animation = SpriteAnimDefinition(*g_spriteSheet_128x64, 5, 7, duration, SpriteAnimPlaybackMode::PING_PONG);
// 
// 	AABB2 cosmeticBox = GetCosmeticBoundsForEntity(m_type, m_bottomCenterPosition);
// 	static std::vector<Vertex_PCU> verts;
// 	verts.clear();
// 	Vec2 maxs;
// 	Vec2 mins;
// 	if (m_isGrounded && m_isWalking)
// 	{
// 		SpriteDefinition sprite = animation.GetSpriteDefAtTime(m_walkingAnimationTime);
// 		sprite.GetUVs(mins, maxs);
// 	}
// 	else if (!m_isGrounded)
// 	{
// 		if (m_velocity.y > 0.0f)
// 		{
// 			g_spriteSheet_128x64->GetSpriteUVs(mins, maxs, 8);
// 		}
// 		else if (m_velocity.y <= 0.0f)
// 		{
// 			g_spriteSheet_128x64->GetSpriteUVs(mins, maxs, 9);
// 		}
// 	}
// 	else
// 	{
// // 		SpriteDefinition sprite = animation.GetSpriteDefAtTime(0.0f);
// // 		sprite.GetUVs(mins, maxs);
// 		g_spriteSheet_128x64->GetSpriteUVs(mins, maxs, IntVec2(5, 0));
// 	}
// 
// 	if (DotProduct2D(m_velocity, Vec2(-1.0f, 0.0f)) > 0.0f)
// 	{
// 		Vec2 maxsCopy = maxs;
// 		Vec2 minsCopy = mins;
// 		mins = Vec2(maxsCopy.x, minsCopy.y);
// 		maxs = Vec2(minsCopy.x, maxsCopy.y);
// 	}
// 	
// 	Rgba8 tint = Rgba8::WHITE;
// 	if (m_isInvulnerable)
// 	{
// 		if (m_isInvisible)
// 		{
// 			tint.a = 0;
// 		}
// 	}
// 	AddVertsForAABB2ToVector(verts, cosmeticBox, tint, mins, maxs);
// 	g_theRenderer->BindTexture(0, g_spriteSheetTexture);
// 	g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
// }


void Player::CalculateWorldForces()
{
	static int jumpFrameAllowance = g_gameConfigBlackboard.GetValue("playerJumpFrameAllowance", 1);
	if (!m_isGrounded && g_currentFrameNumber - m_lostGroundOnFrame > (unsigned int) jumpFrameAllowance)
	{
		m_acceleration += GRAVITY * Vec2(0.0f, -1.0f);
	}
	float friction = m_isGrounded ? GROUND_FRICTION : AIR_FRICTION;
	Vec2 horizontalVelocity = GetProjectedOnto2D(m_velocity, Vec2(1.0f, 0.0f));
	m_acceleration -= friction * horizontalVelocity;
}


void Player::MovePlayer(float deltaSeconds)
{
	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
}
