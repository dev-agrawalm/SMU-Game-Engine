#include "Game/Actor.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/SpawnInfo.hpp"
#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Controller.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/AIController.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Actor::Actor(Map* spawningMap, SpawnInfo const& spawnInfo)
{
	m_map			= spawningMap;
	m_position		= spawnInfo.m_position;
	m_flags			= spawnInfo.m_flags;
	m_faction		= spawnInfo.m_faction;
	m_orientation	= spawnInfo.m_orientation;
	m_goalOrientation = m_orientation;
	m_definition	= spawnInfo.m_definition;
	m_options		= m_definition->m_options;
	m_eyeHeight		= m_definition->m_eyeHeight;
	m_deadEyeHeight	= m_definition->m_deadEyeHeight;
	m_camFov		= m_definition->m_cameraFOVDegrees;
	m_physicsHeight = m_definition->m_physicsHeight;
	m_physicsRadius = m_definition->m_physicsRadius;
	m_physicsBase	= m_definition->m_physicsBase;
	m_animationClock.SetParent(g_game->GetGameStateClock());
	m_lifetime.SetClock(g_game->GetGameStateClock());
	m_health		= m_definition->m_health;
	m_maxHealth		= m_definition->m_health;
	m_velocity		= spawnInfo.m_initialVelocity;
	m_owningActorUID = spawnInfo.m_owningActor;

	if ((int) m_definition->m_weaponInventory.size() > 0)
	{
		int weaponCount = (int) m_definition->m_weaponInventory.size();
		for (int weaponIndex = 0; weaponIndex < weaponCount; weaponIndex++)
		{
			WeaponDefinition const* weaponDef = m_definition->m_weaponInventory[weaponIndex];
			if (weaponDef != nullptr)
			{
				Weapon* weapon = new Weapon(weaponDef, this);
				m_weapons.push_back(weapon);
			}
		}
		m_equippedWeaponIndex = 0;
	}

	if (HasAI())
	{
		m_aiController = new AIController(spawningMap->GetWorld(), m_definition->m_aiType);
	}

	if (DieOnSpawn())
		Die();
}


Actor::~Actor()
{
}


void Actor::Update(float deltaSeconds)
{
	if (IsGarbage())
		return;

	if (IsDead() && m_lifetime.HasElapsed() && m_definition->m_lingerTime >= 0.0f)
	{
		MakeGarbage();
		return;
	}

	if (!IsPossessed() && m_aiController != nullptr)
	{
		m_aiController->PossessActor(this);
	}

	if (IsPossessed())
		m_currentController->Think(deltaSeconds);

	bool hasDefaultAnim = HasDefaultAnim();
	std::string animationToPlay;
	std::string defaultAnim = m_definition->m_appearance.m_defaultAnimName;
	if (m_animationToPlay.size() > 0)
	{
		animationToPlay = m_animationToPlay;
	}

	if (hasDefaultAnim)
	{
		if (_stricmp(animationToPlay.c_str(), "Pain") == 0 && IsAnimationFinished()
			|| _stricmp(animationToPlay.c_str(), "Attack") == 0 && IsAnimationFinished())
		{
			animationToPlay = defaultAnim;
		}

		if (animationToPlay.size() == 0)
		{
			animationToPlay = defaultAnim;
		}
	}

	SetAnimationToPlay(animationToPlay);
}


void Actor::LateUpdate(float deltaSeconds)
{
// 	if (IsDead())
// 		return;

	if (HasPhysicsUpdate())
	{
		ApplyPhysics(deltaSeconds);
		ResetPhysicsFrameData();
	}
}


void Actor::Render() const
{
	if (g_game->ShouldDebugActors())
	{
		Vec3 eyePosition = GetEyePosition();
		Vec3 forwardVec = GetForwardVector();
		float forwardArrowLength = 2.0f;
		DebugAddWorldArrow(eyePosition, eyePosition + forwardVec * forwardArrowLength, 0.0f, Rgba8::CYAN);

		if (HasCollision())
		{
			Vec3 cylinderTop = Vec3(m_position.x, m_position.y, m_physicsHeight);
			DebugAddWorldWireCylinder(m_position, cylinderTop, m_physicsRadius, 0.0f, Rgba8::MAGENTA);
		}
	}

	if (_stricmp(m_animationToPlay.c_str(), "Walk") == 0)
		PlayAnimation(m_animationToPlay, SpriteAnimPlaybackMode::LOOP);
	else if (_stricmp(m_animationToPlay.c_str(), "Death") == 0 || _stricmp(m_animationToPlay.c_str(), "Pain") == 0)
		PlayAnimation(m_animationToPlay, SpriteAnimPlaybackMode::ONCE);
	else
		PlayAnimation(m_animationToPlay);
}


World* Actor::GetWorld() const
{
	return m_map->GetWorld();
}


void Actor::SetActorUID(ActorUID uid)
{
	m_uid = uid;
}


Vec3 Actor::GetDeadEyePosition() const
{
	return Vec3(m_position.x, m_position.y, m_definition->m_deadEyeHeight);
}


Vec3 Actor::GetPosition() const
{
	return m_position;
}


void Actor::SetPosition(Vec3 const& pos)
{
	m_position = pos;
}


void Actor::MoveBy(Vec3 const& movement)
{
	UNUSED(movement);
}


EulerAngles Actor::GetOrientation() const
{
	return m_orientation;
}


Vec3 Actor::GetForwardVector() const
{
	Mat44 orientationMat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	return orientationMat.GetIBasis3D();
}


Vec3 Actor::GetLeftVector() const
{
	Mat44 orientationMat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	return orientationMat.GetJBasis3D();
}


Vec3 Actor::GetUpVector() const
{
	Mat44 orientationMat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	return orientationMat.GetKBasis3D();
}


void Actor::SetOrientation(EulerAngles const& orient)
{
	m_orientation = orient;
	m_orientation.m_pitch = Clamp(m_orientation.m_pitch, -85.0f, 85.0f);
	m_orientation.m_roll = Clamp(m_orientation.m_roll, -45.0f, 45.0f);
}


void Actor::RotateBy(EulerAngles const degrees)
{
	UNUSED(degrees);
}


bool Actor::DieOnSpawn() const
{
	ActorOptions dieOnSpawnMask = ACTOR_OPTION_DIE_ON_SPAWN;
	return (m_options & dieOnSpawnMask) == dieOnSpawnMask;
}


bool Actor::DieOnCollision() const
{
	ActorOptions dieOnCollisionMask = ACTOR_OPTION_DIE_ON_COLLISION;
	return (m_options & dieOnCollisionMask) == dieOnCollisionMask;
}


void Actor::Attack(bool isFreshAttack)
{
	if (m_equippedWeaponIndex < 0 || (int) m_weapons.size() < 0)
		return;

	Weapon* equippedWeapon = m_weapons[m_equippedWeaponIndex];
	Vec3 forwardVector = GetForwardVector();
	Vec3 rayOrigin = GetEyePosition();
	float spawnOffset = 3.0f;
	Vec3 spawnOrigin = rayOrigin + forwardVector * spawnOffset;
	equippedWeapon->Attack(isFreshAttack, rayOrigin, spawnOrigin, forwardVector);
}


Weapon* Actor::GetEquippedWeapon() const
{
	if (m_equippedWeaponIndex >= 0 && (int) m_weapons.size() > 0)
	{
		return m_weapons[m_equippedWeaponIndex];
	}

	return nullptr;
}


bool Actor::HasEquippedWeapon() const
{
	return m_equippedWeaponIndex >= 0 && (int) m_weapons.size() > 0;
}


FloatRange Actor::GetCollisionDamage() const
{
	return m_definition->m_damageOnTouch;
}


bool Actor::DoesDamageOnTouch() const
{
	ActorOptions doesDamageOnTouch = ACTOR_OPTION_DAMAGE_ON_TOUCH;
	return (m_options & doesDamageOnTouch) == doesDamageOnTouch;
}


bool Actor::CreatesImpulseOnTouch() const
{
	ActorOptions createsImpulseOnTouch = ACTOR_OPTION_IMPULSE_ON_TOUCH;
	return (m_options & createsImpulseOnTouch) == createsImpulseOnTouch;
}


float Actor::GetTouchImpulse() const
{
	return m_definition->m_impulseOnTouch;
}


bool Actor::CanFly() const
{
	ActorOptions canFly = ACTOR_OPTION_CAN_FLY;
	return (m_options & canFly) == canFly;
}


void Actor::EquipWeapon(int weaponIndex)
{
	if (weaponIndex >= (int) m_weapons.size())
		return;

	m_equippedWeaponIndex = weaponIndex;
}


void Actor::Die()
{
	if (m_definition->m_lingerTime >= 0.0f)
	{
		m_lifetime.Start(m_definition->m_lingerTime);
	}
	m_flags |= ACTOR_FLAG_IS_DEAD;
	SetAnimationToPlay("Death");

	std::string deathSfx = m_definition->m_deathSfx;
	if (deathSfx != "")
	{
		SoundID deathSfxId = g_audioSystem->CreateOrGetSound(deathSfx);
		g_audioSystem->StartSound(deathSfxId, false, 0.7f);
	}

	if (m_currentController)
		m_currentController->OnPossessedActorKilled();
}


void Actor::TakeDamage(int damage)
{
	static int s_framesSincegotHit = 120;
	s_framesSincegotHit++;
	if (s_framesSincegotHit < 30 && m_aiController == nullptr)
		return;

	s_framesSincegotHit = 0;
	m_health -= damage;
	if (m_health <= 0)
	{
		Die();
	}
	else
	{
		SetAnimationToPlay("Pain");
		std::string painSfx = m_definition->m_painSfx;
		if (painSfx != "")
		{
			SoundID painSfxId = g_audioSystem->CreateOrGetSound(painSfx);
			g_audioSystem->StartSound(painSfxId, false, 0.7f);
		}

		if (m_currentController)
			m_currentController->OnPossessedActorDamaged();
	}
}


int Actor::GetHealth() const
{
	return m_health;
}


bool Actor::IsDead() const
{
	ActorFlags isDeadMask = ACTOR_FLAG_IS_DEAD;
	return (m_flags & isDeadMask) == isDeadMask;
}


void Actor::MakeGarbage()
{
	m_flags |= ACTOR_FLAG_IS_GARBAGE;
}


bool Actor::IsGarbage() const
{
	ActorFlags isGarbageMask = ACTOR_FLAG_IS_GARBAGE;
	return (m_flags & isGarbageMask) == isGarbageMask;
}


void Actor::SetHidden(bool isHidden)
{
	ActorFlags hiddenMask = ACTOR_FLAG_IS_HIDDEN;
	if (isHidden)
	{
		m_flags |= hiddenMask;
	}
	else
	{
		m_flags &= ~hiddenMask;
	}
}


bool Actor::IsHidden() const
{
	ActorFlags isHiddenMask = ACTOR_FLAG_IS_HIDDEN;
	return (m_flags & isHiddenMask) == isHiddenMask;
}


void Actor::SetAnimationToPlay(std::string animName)
{
	if (_stricmp(animName.c_str(), m_animationToPlay.c_str()) == 0)
		return;
	if (_stricmp(animName.c_str(), "Death") != 0 && _stricmp(m_animationToPlay.c_str(), "Pain") == 0 && !IsAnimationFinished())
		return;

	m_animationToPlay = animName;
	m_animationClock.Reset(false);
	SetAnimationSpeed(animName);
}


void Actor::PlayAnimation(std::string const& anim, SpriteAnimPlaybackMode playbackMode /*= SpriteAnimPlaybackMode::LOOP*//*, float animSpeed / *= 1.0f * /*/) const
{
	Camera const& renderCam = g_theRenderer->GetCurrentCamera();
	Vec3 absoluteDir = (renderCam.GetModelMatrix().GetTranslation3D() - m_position).GetNormalized();

	Mat44 modelMat = GetModelMatrix();
	float localDirIComponent = DotProduct3D(absoluteDir, modelMat.GetIBasis3D());
	float localDirJComponent = DotProduct3D(absoluteDir, modelMat.GetJBasis3D());
	float localDirKComponent = DotProduct3D(absoluteDir, modelMat.GetKBasis3D());
	Vec3 localDir = Vec3(localDirIComponent, localDirJComponent, localDirKComponent).GetNormalized();

	ViewDependentSpriteAnimationGroup animationSet = m_definition->m_appearance.GetAnimationGroupByName(anim);
	if (animationSet.m_name != "null")
	{
		SpriteAnimation const* animation = animationSet.GetBestAnimationForDirection(localDir);
		float animationDuration = animationSet.m_secondsPerFrame * (int) animation->m_frameIndices.size();
		SpriteSheet animSpriteSheet = *m_definition->m_appearance.m_spritesheet;
		SpriteAnimDefinition animDefinition = SpriteAnimDefinition(animSpriteSheet, animation->m_frameIndices, animationDuration, playbackMode);
		SpriteDefinition sprite = animDefinition.GetSpriteDefAtTime((float) m_animationClock.GetTotalSeconds());

		Mat44 billboardMat = m_definition->m_appearance.GetBillboardMatrix(m_position, renderCam);
		Vec3 upDir = billboardMat.GetKBasis3D();
		Vec3 leftDir = billboardMat.GetJBasis3D();

		float width = m_definition->m_appearance.m_size.x * m_scale.x;
		float height = m_definition->m_appearance.m_size.y * m_scale.y;
		float leftOfPivot = 0.0f - m_definition->m_appearance.m_pivot.x * width;
		float rightOfPivot = width + leftOfPivot;
		float bottomOfPivot = 0.0f - m_definition->m_appearance.m_pivot.y * height;
		float topOfPivot = height + bottomOfPivot;

		Vec3 bottomLeft = m_position + upDir * bottomOfPivot + leftDir * leftOfPivot;
		Vec3 bottomRight = m_position + upDir * bottomOfPivot + leftDir * rightOfPivot;
		Vec3 topLeft = m_position + upDir * topOfPivot + leftDir * leftOfPivot;
		Vec3 topRight = m_position + upDir * topOfPivot + leftDir * rightOfPivot;
		Vec2 uvMins;
		Vec2 uvMaxs;
		sprite.GetUVs(uvMins, uvMaxs);

		std::vector<Vertex_PCU> verts;
		AddVertsForQuad3DToVector(verts, bottomLeft, bottomRight, topLeft, topRight, m_tint, AABB2(uvMins, uvMaxs));

		Texture const& texture = sprite.GetSpriteSheet().GetTexture();
		g_theRenderer->BindShader(m_definition->m_appearance.m_shader);
		g_theRenderer->BindTexture(0, &texture);
		g_theRenderer->SetCullMode(CullMode::NONE);
		g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
	}
}


void Actor::SetAnimationSpeed(std::string animName, float speedScale /*= 1.0f */)
{
	ViewDependentSpriteAnimationGroup animationSet = m_definition->m_appearance.GetAnimationGroupByName(animName);
	if (animationSet.m_scaleBySpeed)
	{
		m_animationClock.SetTimeScale(speedScale);
	}
	else
	{
		m_animationClock.SetTimeScale(1.0);
	}
}


bool Actor::IsAnimationFinished() const
{
	ViewDependentSpriteAnimationGroup animation = m_definition->m_appearance.GetAnimationGroupByName(m_animationToPlay);
	float animDuration = animation.m_secondsPerFrame * (int) animation.GetBestAnimationForDirection(Vec3(1.0f, 0.0f, 0.0f))->m_frameIndices.size();
	return m_animationClock.GetTotalSeconds() >= (double) animDuration;
}


bool Actor::HasDefaultAnim() const
{
	ActorOptions hasDefaultAnim = ACTOR_OPTION_HAS_DEFAULT_ANIM;
	return (m_options & hasDefaultAnim) == hasDefaultAnim;
}


bool Actor::HasCollision() const
{
	return HasCollisionWithMap() || HasCollisionWithActors();
}


bool Actor::IsImmovable() const
{
	ActorOptions isImmovableMask = ACTOR_OPTION_IMMOVABLE;
	return (m_options & isImmovableMask) == isImmovableMask;
}


bool Actor::HasCollisionWithMap() const
{
	ActorOptions hasCollisionMask = ACTOR_OPTION_COLLIDES_WITH_MAP;
	return (m_options & hasCollisionMask) == ACTOR_OPTION_COLLIDES_WITH_MAP;
}


bool Actor::HasCollisionWithActors() const
{
	ActorOptions hasCollisionMask = ACTOR_OPTION_COLLIDES_WITH_ACTORS;
	return (m_options & hasCollisionMask) == ACTOR_OPTION_COLLIDES_WITH_ACTORS;
}


void Actor::SetCollisionCylinder(float radius, float height)
{
	m_physicsRadius = radius;
	m_physicsHeight = height;
}


float Actor::GetCollisionRadius() const
{
	return m_physicsRadius;
}


float Actor::GetCollisionHeight() const
{
	return m_position.z + m_physicsHeight;
}


float Actor::GetCollisionBase() const
{
	return m_position.z - m_physicsBase;
}


float Actor::GetLocalCollisionHeight() const
{
	return m_physicsHeight;
}


float Actor::GetLocalCollisionBase() const
{
	return m_physicsBase;
}


bool Actor::HasPhysicsUpdate() const
{
	ActorOptions hasPhysicsUpdate = ACTOR_OPTION_PHYSICS_UPDATE;
	return (m_options & hasPhysicsUpdate) == ACTOR_OPTION_PHYSICS_UPDATE;
}


void Actor::SetVelocity(Vec3 const& vel)
{
	m_velocity = vel;
}


void Actor::AddVelocity(Vec3 const& vel)
{
	m_velocity += vel;
}


void Actor::ApplyImpulse(Vec3 const& impulse)
{
	m_velocity += impulse;
}


void Actor::SetAngularVelocity(EulerAngles const& angularVel)
{
	m_angularVelocity = angularVel;
}


void Actor::SetTurnVelocity(float const yawVel)
{
	m_angularVelocity.m_yaw = yawVel;
}


void Actor::AddFrameAcceleration(Vec3 const& acc)
{
	m_frameAcceleration += acc;
}


float Actor::GetWalkSpeed() const
{
	return m_definition->m_walkSpeed;
}


float Actor::GetRunSpeed() const
{
	return m_definition->m_runSpeed;
}


float Actor::GetDrag() const
{
	return m_definition->m_drag;
}


bool Actor::IsPossessable() const
{
	if (m_currentController != nullptr)
	{
		PlayerController* pc = dynamic_cast<PlayerController*>(m_currentController);
		if (pc != nullptr)
			return false;
	}

	ActorOptions canBePossessedMask = ACTOR_OPTION_CAN_BE_POSSESSED;
	return (m_options & canBePossessedMask) == ACTOR_OPTION_CAN_BE_POSSESSED;
}


void Actor::OnPossessed(Controller* controller)
{
	m_currentController = controller;
}


void Actor::OnUnpossessed(Controller* oldController)
{
	PlayerController* pc = dynamic_cast<PlayerController*>(oldController);
	if (pc && m_aiController != nullptr)
	{
		m_currentController = m_aiController;
	}
	else
	{
		m_currentController = nullptr;
	}
}


float Actor::GetCameraFOV() const
{
	return m_camFov;
}


float Actor::GetLocalEyeHeight() const
{
	return m_eyeHeight;
}


Vec3 Actor::GetEyePosition() const
{
	return Vec3( m_position.x, m_position.y, m_eyeHeight);
}


void Actor::MoveInDirection(Vec3 direction, float moveFraction, float walkSprintRatio)
{
	direction.Normalize();

	float walkSpeed = m_definition->m_walkSpeed;
	float runSpeed = m_definition->m_runSpeed;
	float drag = m_definition->m_drag;
	
	float desiredSpeed = Lerp(walkSpeed, runSpeed, walkSprintRatio);
	desiredSpeed *= moveFraction;

	float desiredAcceleration = drag * desiredSpeed;
	AddFrameAcceleration(desiredAcceleration * direction);
}


void Actor::TurnByOrientation(EulerAngles orientationDelta)
{
	m_goalOrientation = m_orientation + orientationDelta;
}


void Actor::TurnTowardsOrientation(EulerAngles goalOrientation)
{
	m_goalOrientation = goalOrientation;
}


Mat44 Actor::GetModelMatrix() const
{
	Mat44 modelMatrix = Mat44::CreateTranslation3D(m_position);
	modelMatrix.Append(m_orientation.GetAsMatrix_XFwd_YLeft_ZUp());
	return modelMatrix;
}


bool Actor::HasAI() const
{
	ActorOptions aiMask = ACTOR_OPTION_HAS_AI;
	return (m_options & aiMask) == aiMask;
}


void Actor::ResetPhysicsFrameData()
{
	m_frameAcceleration = Vec3(0.0f, 0.0f, 0.0f);
}


void Actor::ApplyPhysics(float deltaSeconds)
{
	if (!IsDead())
	{
		float dragForce = m_definition->m_drag;
		AddFrameAcceleration(-dragForce * m_velocity);

		SetVelocity(m_velocity + m_frameAcceleration * deltaSeconds);
		SetPosition(m_position + m_velocity * deltaSeconds);
	}
	m_angularVelocity = m_definition->m_turnSpeed * deltaSeconds;
	float newYaw	=	GetTurnedTowardDegrees(m_orientation.m_yaw, m_goalOrientation.m_yaw, m_angularVelocity.m_yaw);
	float newPitch	=	GetTurnedTowardDegrees(m_orientation.m_pitch, m_goalOrientation.m_pitch, m_angularVelocity.m_pitch);
	float newRoll	=	GetTurnedTowardDegrees(m_orientation.m_roll, m_goalOrientation.m_roll, m_angularVelocity.m_roll);
	SetOrientation(EulerAngles(newYaw, newPitch, newRoll));
}
