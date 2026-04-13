#include "Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Level.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

unsigned char PLAYER_JUMP_KEY = 'W';

Entity::Entity(Level* level)
	: m_level(level)
{
	//empty
}


Entity::Entity(Level* level, EntityType type, Vec2 const& position)
	: m_level(level)
	, m_type(type)
	, m_position(position)
{
	m_definition = EntityDefinition::GetEntityDefinitionByType(m_type);
	m_localPhysicsBounds = m_definition->GetPhysicsBounds();
	m_localCosmeticBounds = m_definition->GetCosmeticBounds();
	Vec2 halfDimsPhysics = m_localPhysicsBounds.GetDimensions() * 0.5f;
	m_localBottomLeft = Vec2(-halfDimsPhysics.x, 0.0f);
	m_localBottomRight = Vec2(halfDimsPhysics.x, 0.0f);
	m_options = m_definition->m_properties.GetProperty("entityOptions", (uint32_t) 0);
	m_collisionLayers = m_definition->m_properties.GetProperty("collisionLayers", (uint32_t) 0);
	SetAnimationState(ANIMATION_STATE_IDLE);

	switch (m_type)
	{
	case ENTITY_TYPE_MARIO:
	case ENTITY_TYPE_SMALL_MARIO:
	{
		m_acceleration = Vec2::ZERO;
		m_health = 2;
		m_coyoteJumpFrameCount = g_gameConfigBlackboard.GetValue("playerJumpFrameAllowance", 1);

		m_invulnerabiiltyDuration = g_gameConfigBlackboard.GetValue("playerInvulnerabilityDuration", 1.0f);
		float invulnerabilityIndicatorDuration = g_gameConfigBlackboard.GetValue("playerInvulnerableIndicatorTimeInterval", 0.3f);
		m_invulnerabiltyIndicatorTimer.SetDuration(invulnerabilityIndicatorDuration);
		break;
	}
	case ENTITY_TYPE_GOOMBA:
	{
		m_speed = GOOMBA_SPEED;
		m_velocity = Vec2(1.0f, 0.0f) * m_speed;
		m_health = 1;
		SetAnimationState(ANIMATION_STATE_MOVING);
		break;
	}
	case ENTITY_TYPE_KOOPA_TROOPA:
	{
		m_speed = TURTLE_SPEED;
		m_velocity = Vec2(1.0f, 0.0f) * m_speed;
		m_health = 1;
		SetAnimationState(ANIMATION_STATE_MOVING);
		break;
	}
	case ENTITY_TYPE_BOWSER:
	{
		m_speed = BOWSER_SPEED;
		m_velocity = Vec2(1.0f, 0.0f) * m_speed;
		m_health = 3;
		SetAnimationState(ANIMATION_STATE_IDLE);

		m_invulnerabiiltyDuration = g_gameConfigBlackboard.GetValue("playerInvulnerabilityDuration", 1.0f);
		float invulnerabilityIndicatorDuration = g_gameConfigBlackboard.GetValue("playerInvulnerableIndicatorTimeInterval", 0.3f);
		m_invulnerabiltyIndicatorTimer.SetDuration(invulnerabilityIndicatorDuration);
		break;
	}
	case ENTITY_TYPE_COIN:
	{
		break;
	}
	case ENTITY_TYPE_LEVEL_END:
	{
		break;
	}
	case ENTITY_TYPE_SWITCH_BLOCK:
	case ENTITY_TYPE_RED_BLOCK:
	{
		g_eventSystem->SubscribeEventCallbackObjectMethod("switchToggle", *this, &Entity::OnSwitchToggle);
		break;
	}
	case ENTITY_TYPE_BLUE_BLOCK:
	{
		g_eventSystem->SubscribeEventCallbackObjectMethod("switchToggle", *this, &Entity::OnSwitchToggle);
		m_isDead = true;
		SetAnimationState(ANIMATION_STATE_DEATH);
		break;
	}
	case ENTITY_TYPE_FLIPPABLE_BLOCK:
	case  ENTITY_TYPE_BREAKABLE_BLOCK:
	{
		break;
	}
	case ENTITY_TYPE_TURTLE_SHELL:
	{
		break;
	}
	case ENTITY_TYPE_RED_MUSHROOM:
	{
		SetAnimationState(ANIMATION_STATE_MOVING);
		m_speed = RED_MUSHROOM_SPEED;
		break;
	}
	case ENTITY_TYPE_FIREBALL:
	{
		SetAnimationState(ANIMATION_STATE_MOVING);
		m_speed = FIREBALL_SPEED;
		break;
	}
	default: //fallthrough
		break;
	}
}


Entity::Entity(Level* level, EntitySpawnInfo const& spawnInfo)
	: Entity(level, spawnInfo.m_type, spawnInfo.m_startingPosition)
{
	m_entityId = spawnInfo.m_entityId;
	m_nextWaypointId = spawnInfo.m_targetWaypointId;
	m_direction = spawnInfo.m_direction;
	if (m_type == ENTITY_TYPE_FIREBALL)
	{
		Vec2 fwdVec = m_direction == ENTITY_FACING_LEFT ? Vec2(-1.0f, 0.0f) : Vec2(1.0f, 0.0f);
		m_velocity = fwdVec * m_speed;
		m_newDirection = m_direction;
	}
}


Entity::~Entity()
{
	if (m_type == ENTITY_TYPE_SWITCH_BLOCK || m_type == ENTITY_TYPE_RED_BLOCK || m_type == ENTITY_TYPE_BLUE_BLOCK)
	{
		g_eventSystem->UnsubscribeEventCallbackObjectMethod("switchToggle", *this, &Entity::OnSwitchToggle);
	}
}

//common update function for all entities
void Entity::Update(float deltaSeconds)
{
	if (m_type == ENTITY_TYPE_WAYPOINT)
		return;

	if (!m_isDead && m_lifetime > 0.0f)
	{
		m_lifetime -= deltaSeconds;
		if (m_lifetime < 0.0f)
		{
			Die();
			return;
		}
	}

	m_animTimer += deltaSeconds;
	if (m_position.x < 0.0f || m_position.x > LEVEL_SIZE_X || m_position.y < 0.0f || m_position.y > LEVEL_SIZE_Y)
	{
		if (m_type != ENTITY_TYPE_MARIO && m_type != ENTITY_TYPE_SMALL_MARIO)
			m_isGarbage = true;
	}

	if (m_isGarbage)
		return;

	if (m_direction != m_newDirection)
	{
		m_direction = m_newDirection;
	}
	
	Entity* waypointEntity = m_level->GetEntityById(m_nextWaypointId.m_id);
	if (waypointEntity && !m_isDead)
	{
		Vec2 targetPos = waypointEntity->m_position;
		Vec2 targetVector = targetPos - m_position;
		if (targetVector.GetLengthSquared() != 0)
		{
			Vec2 targetDir = targetVector.GetNormalized();
			Vec2 displacement = targetDir * WAYPOINT_TRAVEL_SPEED * deltaSeconds;
			Vec2 newPos = m_position + displacement;
			Vec2 targetToPos = m_position - targetPos;
			Vec2 targetToNewPos = newPos - targetPos;
			if (DotProduct2D(targetToPos, targetToNewPos) <= 0.0f)
			{
				m_displacementDueWaypoint = targetPos - m_position;
				if (DotProduct2D(m_displacementDueWaypoint, Vec2(1.0f, 0.0f)) > 0.0f)
				{
					m_newDirection = ENTITY_FACING_RIGHT;
				}
				else if (DotProduct2D(m_displacementDueWaypoint, Vec2(1.0f, 0.0f)) < 0.0f)
				{

					m_newDirection = ENTITY_FACING_LEFT;
				}
				else if(DotProduct2D(m_displacementDueWaypoint, Vec2(1.0f, 0.0f)) == 0.0f)
				{
					Entity* mario = m_level->GetMario();
					if (mario)
					{
						if (mario->m_position.x > m_position.x)
						{
							m_newDirection = ENTITY_FACING_RIGHT;
						}
						else
						{
							m_newDirection = ENTITY_FACING_LEFT;
						}
					}
				}

				if (m_type == ENTITY_TYPE_GOOMBA || m_type == ENTITY_TYPE_KOOPA_TROOPA || m_type == ENTITY_TYPE_BOWSER)
				{

				}
				m_position = targetPos;
				if (m_isGoingForward)
				{
					EntityID currentWaypointId = m_nextWaypointId;
					EntityID nextWaypointId = waypointEntity->m_nextWaypointId;
					EntityID prevWaypointId = waypointEntity->m_prevWaypointId;
					if (nextWaypointId.IsValid())
					{
						Entity* nextWaypoint = m_level->GetEntityById(nextWaypointId.m_id);
						if (nextWaypoint)
						{
							nextWaypoint->m_prevWaypointId = currentWaypointId;
							m_nextWaypointId = nextWaypointId;
							return;
						}
					}
					else if (prevWaypointId.IsValid())
					{
						Entity* prevWaypoint = m_level->GetEntityById(prevWaypointId.m_id);
						if (prevWaypoint)
						{
							m_nextWaypointId = prevWaypointId;
							m_isGoingForward = false;
						}
					}
				}
				else
				{
					if (waypointEntity->m_prevWaypointId.IsValid())
					{
						Entity* prevWaypoint = m_level->GetEntityById(waypointEntity->m_prevWaypointId.m_id);
						if (prevWaypoint)
						{
							m_nextWaypointId = waypointEntity->m_prevWaypointId;
						}
					}
					else if (waypointEntity->m_nextWaypointId.IsValid())
					{
						Entity* nextTargetWaypoint = m_level->GetEntityById(waypointEntity->m_nextWaypointId.m_id);
						if (nextTargetWaypoint)
						{
							m_nextWaypointId = waypointEntity->m_nextWaypointId;
							m_isGoingForward = true;
						}
					}
				}
			}
			else
			{
				m_displacementDueWaypoint = displacement;
				if (DotProduct2D(m_displacementDueWaypoint, Vec2(1.0f, 0.0f)) > 0.0f)
				{
					m_newDirection = ENTITY_FACING_RIGHT;
				}
				else if (DotProduct2D(m_displacementDueWaypoint, Vec2(1.0f, 0.0f)) < 0.0f)
				{

					m_newDirection = ENTITY_FACING_LEFT;
				}
				else if (DotProduct2D(m_displacementDueWaypoint, Vec2(1.0f, 0.0f)) == 0.0f)
				{
					Entity* mario = m_level->GetMario();
					if (mario)
					{
						if (mario->m_position.x > m_position.x)
						{
							m_newDirection = ENTITY_FACING_RIGHT;
						}
						else
						{
							m_newDirection = ENTITY_FACING_LEFT;
						}
					}
				}
				m_position = newPos;
			}
		}

		if (m_type != ENTITY_TYPE_RED_BLOCK && m_type != ENTITY_TYPE_BLUE_BLOCK)
			return;
	}

	switch (m_type)
	{
	case ENTITY_TYPE_MARIO:
	case ENTITY_TYPE_SMALL_MARIO:
	{
		if (m_isDead)
		{
			SetAnimationState(ANIMATION_STATE_DEATH);
			return;
		}

		//check input
		{
			Vec2 movementDir = Vec2::ZERO;

			if (g_inputSystem->WasKeyJustPressed(PLAYER_JUMP_KEY) && (m_isGrounded))
			{
				m_isGrounded = false;
				SoundID playerJumpSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/PlayerJump.wav");
				//float volume = g_muteSfx ? 0.0f : 1.0f;
				g_audioSystem->StartSound(playerJumpSoundId, false, g_gameMusicVolume);
				SetVelocity(Vec2(m_velocity.x, PLAYER_JUMP_IMPULSE_VELOCITY));
			}

			if (g_inputSystem->IsKeyPressed(PLAYER_JUMP_KEY) && m_velocity.y > 0)
			{
				static float sustainThrust = g_gameConfigBlackboard.GetValue("playerJumpSustainThrust", PLAYER_JUMP_SUSTAIN_THRUST);
				m_acceleration += sustainThrust * Vec2(0.0f, 1.0f);
			}

			if (g_inputSystem->IsKeyPressed('D'))
			{
				movementDir += Vec2(1.0f, 0.0f);
			}

			if (g_inputSystem->IsKeyPressed('A'))
			{
				movementDir += Vec2(-1.0f, 0.0f);
			}

			if (movementDir.GetLength() > 0)
			{
				float accelerationMagnitude = PLAYER_WALKING_ACCELERATION;
				if (g_inputSystem->IsKeyPressed(KEYCODE_SHIFT))
				{
					accelerationMagnitude = PLAYER_SPRINTING_ACCELERATION;
				}
				m_acceleration += accelerationMagnitude * movementDir;
			}

			if (g_inputSystem->WasKeyJustPressed(KEYCODE_F9))
			{
				m_position = Vec2(32.0f, 30.0f);
				m_velocity = Vec2::ZERO;
			}
		}

		//calculate world forces
		{
			if (!m_isGrounded)
			{
				m_acceleration += GRAVITY * Vec2(0.0f, -1.0f);
			}
			float friction = m_isGrounded ? GROUND_FRICTION : AIR_FRICTION;
			Vec2 horizontalVelocity = GetProjectedOnto2D(m_velocity, Vec2(1.0f, 0.0f));
			m_acceleration -= friction * horizontalVelocity;
		}


		//move player
		{
			m_velocity += m_acceleration * deltaSeconds;
			m_position += m_velocity * deltaSeconds;
		}

		m_acceleration = Vec2::ZERO;

		if (m_isGrounded)
		{
			if (abs(m_velocity.x) > 0.0f)
			{
				SetAnimationState(ANIMATION_STATE_MOVING);
			}

			if (abs(m_velocity.x) < 1.0f)
			{
				SetAnimationState(ANIMATION_STATE_IDLE);
			}
		}
		else
		{
			if (m_velocity.y > 0.0f)
			{
				SetAnimationState(ANIMATION_STATE_JUMPING_UP);
			}
			else if (m_velocity.y <= 0.0f)
			{
				SetAnimationState(ANIMATION_STATE_FALLING_DOWN);
			}
		}

		if (DotProduct2D(m_velocity, Vec2(-1.0f, 0.0f)) > 0.0f)
		{
			m_direction = ENTITY_FACING_LEFT;
		}
		else
		{
			m_direction = ENTITY_FACING_RIGHT;
		}

		if (m_isInvulnerable)
		{
			float currentTime = (float) GetCurrentTimeSeconds();
			if (currentTime - m_invulnerabilityStartTime > m_invulnerabiiltyDuration)
			{
				m_isInvulnerable = false;
				m_isInvisible = false;
			}

			if (m_invulnerabiltyIndicatorTimer.CheckAndDecrement())
			{
				m_isInvisible = !m_isInvisible;
			}
		}
		break;
	}
	case ENTITY_TYPE_GOOMBA:
	case  ENTITY_TYPE_KOOPA_TROOPA:
	{
		if (!m_isDead)
		{
			if (m_isGrounded)
			{
				Vec2 fwdVec = m_direction == ENTITY_FACING_LEFT ? Vec2(-1.0f, 0.0f) : Vec2(1.0f, 0.0f);
				m_velocity = fwdVec * m_speed;
				Vec2 newPosition = m_position + m_velocity * deltaSeconds;
				Vec2 posToCheck;
				if (m_direction == EntityFacingDirection::ENTITY_FACING_RIGHT)
				{
					posToCheck = newPosition + m_localBottomRight;
				}
				else
				{
					posToCheck = newPosition + m_localBottomLeft;
				}
				posToCheck.y = (float) RoundToNearestInt(posToCheck.y);
				IntVec2 nextPosTileCoords = m_level->GetTileCoords(posToCheck);
				IntVec2 nextPosFloorTileCoords = nextPosTileCoords + IntVec2(0, -1);
				bool canMoveForward = true;
				canMoveForward &= m_level->CanStepOnTile(nextPosFloorTileCoords) && !m_level->DoesTileDoDamage(nextPosFloorTileCoords);
				canMoveForward &= m_level->IsTileEmpty(nextPosTileCoords);
				if (!canMoveForward)
				{
					int newDirectionInt = (int) m_direction * -1;
					m_newDirection = (EntityFacingDirection) newDirectionInt;
				}
				else
				{
					m_position = newPosition;
				}
			}
			else
			{
				static Vec2 gravity = GRAVITY * Vec2(0.0, -1.0f);
				m_velocity += gravity * deltaSeconds;
				m_position += m_velocity * deltaSeconds;
			}
			SetAnimationState(ANIMATION_STATE_MOVING);
		}
		else
		{
			SetAnimationState(ANIMATION_STATE_DEATH);
			if (m_animTimer >= m_animation.m_playbackDuration)
			{
				m_isGarbage = true;
			}
		}

		if (DotProduct2D(m_velocity, Vec2(-1.0f, 0.0f)) > 0.0f)
		{
			m_direction = ENTITY_FACING_LEFT;
		}
		else
		{
			m_direction = ENTITY_FACING_RIGHT;
		}
		break;
	}
	case ENTITY_TYPE_COIN:
	{
		if (m_isDead)
		{
			SetAnimationState(ANIMATION_STATE_DEATH);
			if (m_animTimer >= m_animation.m_playbackDuration)
			{
				m_isGarbage = true;
			}
		}
		break;
	}
	case ENTITY_TYPE_LEVEL_END:
	{
		if (m_isDead)
		{
			SetAnimationState(ANIMATION_STATE_DEATH);
		}
		break;
	}
	case ENTITY_TYPE_SWITCH_BLOCK:
	case ENTITY_TYPE_RED_BLOCK:
	case ENTITY_TYPE_BLUE_BLOCK:
	{
		if (m_isDead)
		{
			SetAnimationState(ANIMATION_STATE_DEATH);
		}
		else
		{
			SetAnimationState(ANIMATION_STATE_IDLE);
		}
		break;
	}
	case ENTITY_TYPE_BREAKABLE_BLOCK:
	{
		if (m_isDead)
		{
			SetAnimationState(ANIMATION_STATE_DEATH);
			if (m_animTimer >= m_animation.m_playbackDuration)
			{
				m_isGarbage = true;
			}
		}
		break;
	}
	case ENTITY_TYPE_FLIPPABLE_BLOCK:
	{
		if (m_isDead)
		{
			SetAnimationState(ANIMATION_STATE_DEATH);
			if (m_animTimer >= m_animation.m_playbackDuration)
			{

				AABB2 worldPhysicsBounds = GetWorldPhysicsBounds();
				if (!m_level->DoesEntityOverlapAnyOtherEntity(this))
				{
					m_isDead = false;
					SetAnimationState(ANIMATION_STATE_IDLE);
				}
			}
		}
		break;
	}
	case ENTITY_TYPE_TURTLE_SHELL:
	case ENTITY_TYPE_RED_MUSHROOM:
	{
		if (m_type == ENTITY_TYPE_RED_MUSHROOM && m_isDead)
		{
			m_isGarbage = true;
			return;
		}

		if (m_isDead)
			return;

		if (m_velocity.GetLengthSquared() != 0.0f)
		{
			SetAnimationState(ANIMATION_STATE_MOVING);
		}
		else
		{
			SetAnimationState(ANIMATION_STATE_IDLE);
		}

		if (m_wasHit)
		{
			Rgba8 color = DevConsole::MINOR_INFO;
			g_console->AddLine(color, Stringf("GreenShell Velocity before: %.02f, %.02f", m_velocity.x, m_velocity.y));
		}

		if (m_isGrounded)
		{
			Vec2 fwdVec = m_direction == ENTITY_FACING_LEFT ? Vec2(-1.0f, 0.0f) : Vec2(1.0f, 0.0f);
			m_velocity = fwdVec * m_speed;
			m_velocity.y = 0.0f;
		}
		else
		{
			static Vec2 gravity = GRAVITY * Vec2(0.0, -1.0f);
			m_velocity += gravity * deltaSeconds;
		}

		Vec2 newPosition = m_position + m_velocity * deltaSeconds;
		Vec2 posToCheck;
		if (m_direction == EntityFacingDirection::ENTITY_FACING_RIGHT)
		{
			posToCheck = newPosition + m_localBottomRight;
		}
		else
		{
			posToCheck = newPosition + m_localBottomLeft;
		}
		posToCheck.y = (float) RoundToNearestInt(posToCheck.y);
		IntVec2 nextPosTileCoords = m_level->GetTileCoords(posToCheck);
		IntVec2 nextPosFloorTileCoords = nextPosTileCoords + IntVec2(0, -1);
		bool canMoveForward = m_level->IsTileEmpty(nextPosTileCoords);
		if (!canMoveForward)
		{
			int newDirectionInt = (int) m_direction * -1;
			m_newDirection = (EntityFacingDirection) newDirectionInt;
		}
		else
		{
			m_position = newPosition;
		}

		if (DotProduct2D(m_velocity, Vec2(-1.0f, 0.0f)) > 0.0f)
		{
			m_direction = ENTITY_FACING_LEFT;
		}
		else
		{
			m_direction = ENTITY_FACING_RIGHT;
		}

		break;
	}
	case ENTITY_TYPE_BOWSER:
	{
		bool s_isAttacking = false;
		if (m_isDead)
		{
			SetAnimationState(ANIMATION_STATE_DEATH);
			if (m_animTimer >= m_animation.m_playbackDuration)
			{
				m_isGarbage = true;
			}
		}

		static bool s_move = true;
		static bool s_canAttack = true;
// 		if (g_inputSystem->WasKeyJustPressed('V'))
// 		{
// 			SetAnimationState(ANIMATION_STATE_IDLE);
// 			s_move = false;
// 		}
// 
// 		if (g_inputSystem->WasKeyJustPressed('C'))
// 		{
// 			SetAnimationState(ANIMATION_STATE_MOVING);
// 			s_move = true;
// 		}

		Entity* mario =  m_level->GetMario();
		if (mario)
		{
			if (mario->m_position.x > m_position.x)
			{
				m_newDirection = ENTITY_FACING_RIGHT;
			}
			else
			{
				m_newDirection = ENTITY_FACING_LEFT;
			}
		}

// 		if (g_inputSystem->WasKeyJustPressed('X'))
// 		{
// 			int newDirectionInt = (int) m_direction * -1;
// 			m_newDirection = (EntityFacingDirection) newDirectionInt;
// 		}
// 
// 		if (g_inputSystem->WasKeyJustPressed('Z'))
// 		{
// 			SetAnimationState(ANIMATION_STATE_ATTACK);
// 			s_isAttacking = true;
// 			m_isInvulnerable = true;
// 		}

		if (m_animState == ANIMATION_STATE_ATTACK && m_animTimer >= m_animation.m_playbackDuration)
		{
			SetAnimationState(ANIMATION_STATE_MOVING);
			m_isInvulnerable = false;
			s_isAttacking = false;
			EntitySpawnInfo fireballSpawnInfo = EntitySpawnInfo(ENTITY_TYPE_FIREBALL);
			if (m_direction == ENTITY_FACING_LEFT)
			{
				Vec2 fireballSpawnPos = m_position + m_localBottomLeft;
				fireballSpawnPos.y += g_rng->GetRandomFloatInRange(0.25f, m_localPhysicsBounds.GetDimensions().y);
				fireballSpawnPos.x -= 0.5f;
				fireballSpawnInfo.m_startingPosition = fireballSpawnPos;
				fireballSpawnInfo.m_direction = ENTITY_FACING_LEFT;
			}
			else
			{
				Vec2 fireballSpawnPos = m_position + m_localBottomRight;
				fireballSpawnPos.y += g_rng->GetRandomFloatInRange(0.25f, m_localPhysicsBounds.GetDimensions().y);
				fireballSpawnPos.x += 0.5f;
				fireballSpawnInfo.m_startingPosition = fireballSpawnPos;
				fireballSpawnInfo.m_direction = ENTITY_FACING_RIGHT;
			}
			m_level->SpawnEntity(fireballSpawnInfo);
		}

		if (!m_isDead && s_move)
		{
			if (m_isGrounded)
			{
				static bool s_movebackwards = false;
				if (m_randomMovementTimer < 0.0f)
				{
					s_movebackwards = !s_movebackwards;
					m_randomMovementTimer = g_rng->GetRandomFloatInRange(1.0f, 5.0f);
				}
				Vec2 fwdVec = m_direction == ENTITY_FACING_LEFT ? Vec2(-1.0f, 0.0f) : Vec2(1.0f, 0.0f);
				m_velocity = fwdVec * m_speed;
				//float randomNoise = Get1dPerlinNoise((float) GetCurrentTimeSeconds(), 10.0f, 1, 0.5f, 2.0f, true, g_rng->GetRandomIntInRange(0, 100)) * 0.5f + 0.5f;
				if (s_movebackwards)
				{
					m_velocity *= -1.0f;
				}
				Vec2 newPosition = m_position + m_velocity * deltaSeconds;
				Vec2 posToCheck;
				if (m_direction == EntityFacingDirection::ENTITY_FACING_RIGHT)
				{
					posToCheck = newPosition + m_localBottomRight;
				}
				else
				{
					posToCheck = newPosition + m_localBottomLeft;
				}
				posToCheck.y = (float) RoundToNearestInt(posToCheck.y);
				IntVec2 nextPosTileCoords = m_level->GetTileCoords(posToCheck);
				IntVec2 nextPosFloorTileCoords = nextPosTileCoords + IntVec2(0, -1);
				bool canMoveForward = true;
				canMoveForward &= m_level->CanStepOnTile(nextPosFloorTileCoords) && !m_level->DoesTileDoDamage(nextPosFloorTileCoords);
				canMoveForward &= m_level->IsTileEmpty(nextPosTileCoords);
				canMoveForward &= !s_isAttacking;
				if (canMoveForward)
				{
					m_position = newPosition;
				}

				if (m_attackTimerDuration < 0.0f)
				{
					SetAnimationState(ANIMATION_STATE_ATTACK);
					s_isAttacking = true;
					m_isInvulnerable = true;
					m_attackTimerDuration = g_rng->GetRandomFloatInRange(1.5f, 2.5f);
				}
			}
			else
			{
				static Vec2 gravity = GRAVITY * Vec2(0.0, -1.0f);
				m_velocity += gravity * deltaSeconds;
				m_position += m_velocity * deltaSeconds;
			}
			SetAnimationState(ANIMATION_STATE_MOVING);
		}

		if (m_isInvulnerable)
		{
			float currentTime = (float) GetCurrentTimeSeconds();
			if (currentTime - m_invulnerabilityStartTime > m_invulnerabiiltyDuration)
			{
				m_isInvulnerable = false;
			}

			if (m_invulnerabiltyIndicatorTimer.CheckAndDecrement())
			{
				m_isInvisible = !m_isInvisible;
			}
		}

		m_attackTimerDuration -= deltaSeconds;
		m_randomMovementTimer -= deltaSeconds;
		break;
	}
	case ENTITY_TYPE_FIREBALL:
	{
		if (m_isDead)
		{
			SetAnimationState(ANIMATION_STATE_DEATH);
			if (m_animTimer >= m_animation.m_playbackDuration)
			{
				m_isGarbage = true;
			}
		}

		m_position += m_velocity * deltaSeconds;
	}
	default: //fallthrough
		break;
	}
}


void Entity::Render() const
{
	if (m_type == ENTITY_TYPE_WAYPOINT)
		return;

	Rgba8 tint = Rgba8::WHITE;
	if(m_type == ENTITY_TYPE_MARIO || m_type == ENTITY_TYPE_SMALL_MARIO)
	{
		if (m_isDead)
		{
			return;
		}

		if (m_isInvulnerable)
		{
			if (m_isInvisible)
			{
				tint.a = 0;
			}
		}
	}

	if (m_type == ENTITY_TYPE_BOWSER)
	{
		if (m_isInvulnerable)
		{
			if (m_isInvisible)
			{
				tint.a = 0;
			}
		}
	}

	if (g_game->IsDebugModeActive())
	{
		DebugRender();
	}

	if (m_isGarbage)
		return;

	AABB2 cosmeticBox = GetWorldCosmeticBounds();
	cosmeticBox.UniformScaleFromCenter(m_animation.m_scale);
	Vec2 cosmeticDims = cosmeticBox.GetDimensions();
	Vec2 pivotOffset = cosmeticDims * m_animation.m_pivot;
	cosmeticBox.Translate(-pivotOffset);
	static std::vector<Vertex_PCU> verts;
	verts.clear();
	Vec2 maxs;
	Vec2 mins;
	SpriteAnimDefinition animDefinition = m_animation.GetAsSpriteAnimDefinition();
	SpriteDefinition sprite = animDefinition.GetSpriteDefAtTime(m_animTimer);
	sprite.GetUVs(mins, maxs);

	if (m_direction == ENTITY_FACING_LEFT)
	{
		Vec2 maxsCopy = maxs;
		Vec2 minsCopy = mins;
		mins = Vec2(maxsCopy.x, minsCopy.y);
		maxs = Vec2(minsCopy.x, maxsCopy.y);
	}
	AddVertsForAABB2ToVector(verts, cosmeticBox, tint, mins, maxs);
	g_theRenderer->BindTexture(0, g_spriteSheetTexture);
	g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
}


void Entity::DebugRender() const
{
	float width = 0.05f;
	Rgba8 physicsColor = Rgba8::YELLOW;
	Rgba8 cosmeticColor = Rgba8::MAGENTA;

	AABB2 physicsBox = m_localPhysicsBounds;
	physicsBox.Translate(m_position);

	AABB2 cosmeticBox = m_localCosmeticBounds;
	cosmeticBox.Translate(m_position);

	DebugDrawAABB2Outline(cosmeticBox, width, cosmeticColor);
	DebugDrawAABB2Outline(physicsBox, width, physicsColor);
}


void Entity::Die()
{
	m_isDead = true;
	if (m_type == ENTITY_TYPE_KOOPA_TROOPA)
	{
		EntitySpawnInfo shellSpawnInfo = EntitySpawnInfo(ENTITY_TYPE_TURTLE_SHELL);
		shellSpawnInfo.m_startingPosition = m_position + Vec2(0.0f, 0.5f);
		m_level->SpawnEntity(shellSpawnInfo);
	}
}


bool Entity::TakeDamage(int damage)
{
	if (m_isInvulnerable || m_isDead)
		return false;

	m_health -= damage;
	if (m_health <= 0)
	{
		Die();
		return true;
	}

	if (m_type == ENTITY_TYPE_MARIO || m_type == ENTITY_TYPE_BOWSER)
	{
		m_isInvulnerable = true;
		m_invulnerabilityStartTime = (float) GetCurrentTimeSeconds();
		m_invulnerabiltyIndicatorTimer.Restart();

		if (m_type == ENTITY_TYPE_MARIO)
		{
			SetAnimationState(ANIMATION_STATE_TAKE_DAMAGE);
			m_type = ENTITY_TYPE_SMALL_MARIO;
			m_definition = EntityDefinition::GetEntityDefinitionByType(ENTITY_TYPE_SMALL_MARIO);
			m_localPhysicsBounds = m_definition->GetPhysicsBounds();
		}
	}
	return true;
}


void Entity::Heal(int healAmt)
{
	m_health += healAmt;
	if (m_type == ENTITY_TYPE_SMALL_MARIO)
	{
		m_type = ENTITY_TYPE_MARIO;
		m_health = Clamp(m_health, 0, 2);
		SetAnimationState(ANIMATION_STATE_HEAL);
		m_definition = EntityDefinition::GetEntityDefinitionByType(ENTITY_TYPE_MARIO);
		m_localPhysicsBounds = m_definition->GetPhysicsBounds();
	}
}


void Entity::OnOverlap_Enemy(AABB2CollisionData const& collisionData, Entity* otherEntity)
{
	if (otherEntity->m_type != ENTITY_TYPE_MARIO && otherEntity->m_type != ENTITY_TYPE_SMALL_MARIO && otherEntity->m_type != ENTITY_TYPE_COIN && otherEntity->m_type != ENTITY_TYPE_LEVEL_END)
	{
		if ((collisionData.m_shortestOverlappingSide == AABB2_LEFT_SIDE || collisionData.m_shortestOverlappingSide == AABB2_RIGHT_SIDE))
		{
			if(m_direction != otherEntity->m_direction || m_speed > otherEntity->m_speed)
			{
				int newDirectionInt = (int) m_direction * -1;
				m_newDirection = (EntityFacingDirection) newDirectionInt;
			}
		}
	}

	if (otherEntity->m_type == ENTITY_TYPE_MARIO || otherEntity->m_type == ENTITY_TYPE_SMALL_MARIO)
	{
		//enemy got hit by mario
		if (collisionData.m_shortestOverlappingSide == AABB2_TOP_SIDE)
		{
			if (TakeDamage(1))
			{
				otherEntity->SetVelocity(Vec2(m_velocity.x, PLAYER_BOUNCE_IMPULSE_VELOCITY));
				static SoundID goomaDeathSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/GoombaDeath.wav");
				//float volume = g_muteSfx ? 0.0f : 1.0f;
				g_audioSystem->StartSound(goomaDeathSoundId, false, g_gameMusicVolume);
			}
		}
		else
		{
			if (otherEntity->TakeDamage(1))
			{
				if (collisionData.m_shortestOverlappingSide == AABB2_RIGHT_SIDE)
				{
					otherEntity->SetVelocity(Vec2(PLAYER_JUMP_IMPULSE_VELOCITY, PLAYER_JUMP_IMPULSE_VELOCITY));
				}
				else if (collisionData.m_shortestOverlappingSide == AABB2_LEFT_SIDE)
				{
					otherEntity->SetVelocity(Vec2(-PLAYER_JUMP_IMPULSE_VELOCITY, PLAYER_JUMP_IMPULSE_VELOCITY));
				}
			}
		}
	}
}


void Entity::OnOverlap_GreenShell(AABB2CollisionData const& collisionData, Entity* otherEntity)
{
	if (otherEntity->m_type != ENTITY_TYPE_MARIO && otherEntity->m_type != ENTITY_TYPE_SMALL_MARIO && otherEntity->m_type != ENTITY_TYPE_KOOPA_TROOPA && otherEntity->m_type != ENTITY_TYPE_GOOMBA)
	{
		if ((collisionData.m_shortestOverlappingSide == AABB2_LEFT_SIDE || collisionData.m_shortestOverlappingSide == AABB2_RIGHT_SIDE))
		{
			int newDirectionInt = (int) m_direction * -1;
			m_newDirection = (EntityFacingDirection) newDirectionInt;
		}
	}

	if (otherEntity->m_type == ENTITY_TYPE_GOOMBA || otherEntity->m_type == ENTITY_TYPE_KOOPA_TROOPA)
	{
		if (m_speed != 0.0f)
			otherEntity->Die();
	}

	if (otherEntity->m_type == ENTITY_TYPE_MARIO || otherEntity->m_type == ENTITY_TYPE_SMALL_MARIO)
	{
		if (collisionData.m_shortestOverlappingSide == AABB2_TOP_SIDE)
		{
			if (m_speed == 0.0f)
			{
				m_speed = GREEN_SHELL_SPEED;
			}
			else
			{
				m_speed = 0.0f;
			}
			otherEntity->SetVelocity(Vec2(m_velocity.x, PLAYER_BOUNCE_IMPULSE_VELOCITY));
		}
		else if (collisionData.m_shortestOverlappingSide == AABB2_LEFT_SIDE)
		{
			if (m_speed == 0.0f)
			{
				m_speed = GREEN_SHELL_SPEED;
				m_direction = ENTITY_FACING_RIGHT;
			}
			else
			{
				if (otherEntity->TakeDamage(1))
				{
					otherEntity->SetVelocity(Vec2(-PLAYER_JUMP_IMPULSE_VELOCITY, PLAYER_JUMP_IMPULSE_VELOCITY));
				}
				else
				{
					int newDirectionInt = (int) m_direction * -1;
					m_newDirection = (EntityFacingDirection) newDirectionInt;
				}
			}
		}
		else if (collisionData.m_shortestOverlappingSide == AABB2_RIGHT_SIDE)
		{
			if (m_speed == 0.0f)
			{
				m_speed = GREEN_SHELL_SPEED;
				m_direction = ENTITY_FACING_LEFT;
			}
			else
			{
				if (otherEntity->TakeDamage(1))
				{
					otherEntity->SetVelocity(Vec2(PLAYER_JUMP_IMPULSE_VELOCITY, PLAYER_JUMP_IMPULSE_VELOCITY));
				}
				else
				{
					int newDirectionInt = (int) m_direction * -1;
					m_newDirection = (EntityFacingDirection) newDirectionInt;
				}
			}
		}
	}

	if (otherEntity->m_type == ENTITY_TYPE_TURTLE_SHELL && otherEntity->m_speed > 0.0f)
	{
		if (m_speed == 0.0f)
		{
			m_speed = GREEN_SHELL_SPEED;
			if (collisionData.m_shortestOverlappingSide == AABB2_RIGHT_SIDE)
			{
				m_newDirection = ENTITY_FACING_LEFT;
			}
			else
			{
				m_newDirection = ENTITY_FACING_RIGHT;
			}
		}
		else
		{
			int newDirectionInt = (int) m_direction * -1;
			m_newDirection = (EntityFacingDirection) newDirectionInt;
		}
	}
}


void Entity::OnOverlap_SwitchBlock(AABB2CollisionData const& collisionData, Entity* otherEntity)
{
	if ((otherEntity->m_type == ENTITY_TYPE_MARIO || otherEntity->m_type == ENTITY_TYPE_SMALL_MARIO) && otherEntity->m_velocity.y > 0.0f)
	{
		if (collisionData.m_shortestOverlappingSide == AABB2_BOTTOM_SIDE)
		{
			m_isDead = !m_isDead;
			EventArgs args;
			args.AddProperty("isSwitchOn", !m_isDead);
			g_eventSystem->FireEvent("switchToggle", args);
		}
	}

	if (otherEntity->m_type == ENTITY_TYPE_TURTLE_SHELL && collisionData.m_shortestOverlappingSide != AABB2_TOP_SIDE)
	{
		m_isDead = !m_isDead;
		EventArgs args;
		args.AddProperty("isSwitchOn", !m_isDead);
		g_eventSystem->FireEvent("switchToggle", args);
	}
}


void Entity::OnOverlap(AABB2CollisionData const& collisionData, Entity* otherEntity)
{
	if (m_type == ENTITY_TYPE_SWITCH_BLOCK)
	{
		OnOverlap_SwitchBlock(collisionData, otherEntity);
		return;
	}

	if (m_type == ENTITY_TYPE_BREAKABLE_BLOCK || m_type == ENTITY_TYPE_FLIPPABLE_BLOCK)
	{
		if (m_isDead)
			return;

		if ((otherEntity->m_type == ENTITY_TYPE_MARIO || otherEntity->m_type == ENTITY_TYPE_SMALL_MARIO) && otherEntity->m_velocity.y > 0.0f)
		{
			if (collisionData.m_shortestOverlappingSide == AABB2_BOTTOM_SIDE)
			{
				Die();
			}
		}

		if (otherEntity->m_type == ENTITY_TYPE_TURTLE_SHELL && collisionData.m_shortestOverlappingSide != AABB2_TOP_SIDE)
		{
			Die();
		}

		return;
	}

	if (m_type == ENTITY_TYPE_GOOMBA || m_type == ENTITY_TYPE_KOOPA_TROOPA)
	{
		OnOverlap_Enemy(collisionData, otherEntity);
		return;
	}

	if (m_type == ENTITY_TYPE_FIREBALL)
	{
		if (otherEntity->m_type != ENTITY_TYPE_MARIO && otherEntity->m_type != ENTITY_TYPE_SMALL_MARIO 
			&& otherEntity->m_type != ENTITY_TYPE_COIN && otherEntity->m_type != ENTITY_TYPE_LEVEL_END && otherEntity->m_type != ENTITY_TYPE_BOWSER)
		{
			Die();
		}
		return;
	}

	if (m_type == ENTITY_TYPE_TURTLE_SHELL)
	{
		OnOverlap_GreenShell(collisionData, otherEntity);
		return;
	}

	if (m_type == ENTITY_TYPE_RED_MUSHROOM)
	{
		if (otherEntity->m_type != ENTITY_TYPE_MARIO && otherEntity->m_type != ENTITY_TYPE_SMALL_MARIO && otherEntity->m_type != ENTITY_TYPE_KOOPA_TROOPA && otherEntity->m_type != ENTITY_TYPE_GOOMBA)
		{
			if ((collisionData.m_shortestOverlappingSide == AABB2_LEFT_SIDE || collisionData.m_shortestOverlappingSide == AABB2_RIGHT_SIDE))
			{
				int newDirectionInt = (int) m_direction * -1;
				m_direction = (EntityFacingDirection) newDirectionInt;
			}
		}

		if (otherEntity->m_type == ENTITY_TYPE_MARIO || otherEntity->m_type == ENTITY_TYPE_SMALL_MARIO)
		{
			Die();
		}
		return;
	}

	if (m_type == ENTITY_TYPE_COIN)
	{
		if (otherEntity->m_type == ENTITY_TYPE_MARIO || otherEntity->m_type == ENTITY_TYPE_SMALL_MARIO)
			Die();

		return;
	}

	if (m_type == ENTITY_TYPE_MARIO || m_type == ENTITY_TYPE_SMALL_MARIO)
	{
		if (otherEntity->m_type == ENTITY_TYPE_COIN)
		{
			m_collectedCoinCount++;
			static SoundID coinCollectSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/CoinCollect.wav");
			//float volume = g_muteSfx ? 0.0f : 1.0f;
			g_audioSystem->StartSound(coinCollectSoundId, false, g_gameMusicVolume);
		}
		
		if (otherEntity->m_type == ENTITY_TYPE_RED_MUSHROOM)
		{
			Heal(1);
			static SoundID coinCollectSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/CoinCollect.wav");
			//float volume = g_muteSfx ? 0.0f : 1.0f;
			g_audioSystem->StartSound(coinCollectSoundId, false, g_gameMusicVolume);
		}

		if (otherEntity->m_type == ENTITY_TYPE_FIREBALL)
		{
			if (TakeDamage(1))
			{
				if (m_direction == ENTITY_FACING_LEFT)
				{
					SetVelocity(Vec2(PLAYER_JUMP_IMPULSE_VELOCITY, PLAYER_JUMP_IMPULSE_VELOCITY));
				}
				else if (m_direction == ENTITY_FACING_RIGHT)
				{
					SetVelocity(Vec2(-PLAYER_JUMP_IMPULSE_VELOCITY, PLAYER_JUMP_IMPULSE_VELOCITY));
				}
			}
		}

		return;
	}

	if (m_type == ENTITY_TYPE_LEVEL_END)
	{
		if ((otherEntity->m_type == ENTITY_TYPE_MARIO || otherEntity->m_type == ENTITY_TYPE_SMALL_MARIO) && m_level->CheckLevelEndCondition())
		{
			Die();
		}

		return;
	}

	if (m_type == ENTITY_TYPE_BOWSER)
	{
		if (otherEntity->m_type == ENTITY_TYPE_MARIO || otherEntity->m_type == ENTITY_TYPE_SMALL_MARIO)
		{
			//enemy got hit by mario
			if (collisionData.m_shortestOverlappingSide == AABB2_TOP_SIDE)
			{
				if (TakeDamage(1))
				{
// 					static SoundID goomaDeathSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/GoombaDeath.wav");
// 					float volume = g_muteSfx ? 0.0f : 1.0f;
// 					g_audioSystem->StartSound(goomaDeathSoundId, false, volume);
				}
				otherEntity->SetVelocity(Vec2(m_velocity.x, PLAYER_BOUNCE_IMPULSE_VELOCITY));
			}
			else if (collisionData.m_shortestOverlappingSide == AABB2_BOTTOM_SIDE)
			{
				otherEntity->Die();
			}
			else
			{
				if (otherEntity->TakeDamage(1))
				{
					if (collisionData.m_shortestOverlappingSide == AABB2_RIGHT_SIDE)
					{
						otherEntity->SetVelocity(Vec2(PLAYER_JUMP_IMPULSE_VELOCITY, PLAYER_JUMP_IMPULSE_VELOCITY));
					}
					else if (collisionData.m_shortestOverlappingSide == AABB2_LEFT_SIDE)
					{
						otherEntity->SetVelocity(Vec2(-PLAYER_JUMP_IMPULSE_VELOCITY, PLAYER_JUMP_IMPULSE_VELOCITY));
					}
				}
			}
		}

		return;
	}
}


void Entity::SetGrounded(bool newGroundedState)
{
	if (m_isGrounded && !newGroundedState)
	{
		if (g_currentFrameNumber - m_lostGroundOnFrame <= m_coyoteJumpFrameCount)
			return;

		m_lostGroundOnFrame = g_currentFrameNumber;
	}

	m_isGrounded = newGroundedState;
}


void Entity::MakeGarbage()
{
	m_isGarbage = true;
}


void Entity::SetPosition(Vec2 const& newPos)
{
	m_position = newPos;
}


void Entity::SetVelocity(Vec2 const& newVel)
{
	m_velocity = newVel;
}


void Entity::AddImpulseVelocity(Vec2 const& impulseVel)
{
	m_velocity += impulseVel;
}


void Entity::CollectCoins(int coinsToAdd)
{
	m_collectedCoinCount += coinsToAdd;
}


void Entity::SetIsDead(bool isDead)
{
	m_isDead = isDead;
	if (m_type == ENTITY_TYPE_SWITCH_BLOCK)
	{
		EventArgs args;
		args.AddProperty("isSwitchOn", !m_isDead);
		g_eventSystem->FireEvent("switchToggle", args);
	}
}


Vec2 Entity::GetVelocity() const
{
	return m_velocity;
}


Vec2 Entity::GetPosition() const
{
	return m_position;
}


AABB2 Entity::GetLocalPhysicsBounds() const
{
	return m_localPhysicsBounds;
}


AABB2 Entity::GetWorldPhysicsBounds() const
{
	AABB2 worldPhysicsBounds = m_localPhysicsBounds;
	worldPhysicsBounds.Translate(m_position);
	return worldPhysicsBounds;
}


AABB2 Entity::GetLocalCosmeticBounds() const
{
	return m_localCosmeticBounds;
}


AABB2 Entity::GetWorldCosmeticBounds() const
{
	AABB2 worldCosmeticBounds = m_localCosmeticBounds;
	worldCosmeticBounds.Translate(m_position);
	return worldCosmeticBounds;
}


bool Entity::HasOptions(uint32_t optionsBitmask)
{
	return (m_options & optionsBitmask) == optionsBitmask;
}


bool Entity::OnSwitchToggle(EventArgs& args)
{
	bool isSwitchOn = args.GetProperty("isSwitchOn", false);
	if (m_type == ENTITY_TYPE_RED_BLOCK)
	{
		m_isDead = !isSwitchOn;
	}
	else if (m_type == ENTITY_TYPE_BLUE_BLOCK)
	{
		m_isDead = isSwitchOn;
	}
	else if (m_type == ENTITY_TYPE_SWITCH_BLOCK)
	{
		m_isDead = !isSwitchOn;
	}
	return false;
}


bool Entity::IsInvulnerable() const
{
	return m_isInvulnerable;
}


bool Entity::IsAlive() const
{
	return !m_isDead;
}


bool Entity::IsGarbage() const
{
	return m_isGarbage;
}


void Entity::SetAnimationState(AnimationState newState)
{
	if (m_animation.m_blocksOtherAnimations && m_animTimer < m_animation.m_playbackDuration)
		return;

	if (newState != m_animState)
	{
		m_animTimer = 0.0f;
		m_animState = newState;
		m_animation = m_definition->GetAnimation(newState);
	}
}

