#include "Game/Weapon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/SpawnInfo.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

bool WeaponAnimation::LoadFromXML(XmlElement const& node)
{
	m_name = ParseXmlAttribute(node, "name", m_name);
	m_secondsPerFrame = ParseXmlAttribute(node, "secondsPerFrame", m_secondsPerFrame);
	m_framesPerSecond = 1.0f / m_secondsPerFrame;

	std::string animCSV = ParseXmlAttribute(node, "frames", "");
	if (animCSV != "")
	{
		Strings animationSpriteIndices = SplitStringOnDelimiter(animCSV, ',');
		if ((int) animationSpriteIndices.size() == 0)
			return false;

		for (int spriteFrame = 0; spriteFrame < (int) animationSpriteIndices.size(); spriteFrame++)
		{
			std::string spriteIndexText = animationSpriteIndices[spriteFrame];
			int spriteIndex = atoi(spriteIndexText.c_str());
			m_frameIndices.push_back(spriteIndex);
		}
		return true;
	}

	return true;
}


bool WeaponHud::LoadFromXML(XmlElement const& node)
{
	std::string reticleTextureName = ParseXmlAttribute(node, "reticle", "Default");
	m_reticleTexture = g_theRenderer->CreateOrGetTexture(reticleTextureName.c_str());
	m_reticleSize = ParseXmlAttribute(node, "reticleSize", m_reticleSize);

	std::string spriteSheetName = ParseXmlAttribute(node, "spriteSheet", "");
	if ((int) spriteSheetName.size() > 0)
	{
		m_cellCount = ParseXmlAttribute(node, "cellCount", m_cellCount);
		Texture* spriteSheetTexture = g_theRenderer->CreateOrGetTexture(spriteSheetName.c_str());
		m_spriteSheet = new SpriteSheet(*spriteSheetTexture, m_cellCount);
		m_spriteScale = ParseXmlAttribute(node, "spriteScale", m_spriteScale);
	}

	XmlElement const* animationElement = node.FirstChildElement("Animation");
	while (animationElement != nullptr)
	{
		WeaponAnimation weaponAnim;
		if (weaponAnim.LoadFromXML(*animationElement))
		{
			m_animations.push_back(weaponAnim);
		}
		else
			return false;

		animationElement = animationElement->NextSiblingElement("Animation");
	}
	return true;
}


WeaponAnimation WeaponHud::GetAnimationByName(std::string animationName)
{
	WeaponAnimation resultAnimation = {};
	for (int animationIndex = 0; animationIndex < (int) m_animations.size(); animationIndex++)
	{
		WeaponAnimation& animation = m_animations[animationIndex];
		if (animation.m_name == animationName)
		{
			resultAnimation = animation;
		}
	}

	return resultAnimation;
}


void ProjectileActorDefinitions::LoadProjectileActorDefinitionsFromXML(std::string filePath)
{
	XmlDocument xmlDoc;
	XmlError result = xmlDoc.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(result == 0, "Unable to load file for projectile actor definitions: " + filePath);

	XmlElement* rootElement = xmlDoc.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "Definitions") == 0, "Projectile Actor Definition file does not have definitions element: " + filePath);

	XmlElement* definitionElement = rootElement->FirstChildElement("ActorDefinition");
	while (definitionElement)
	{
		ActorDefinition* definition = new ActorDefinition();
		definition->LoadFromXmlElement(*definitionElement);
		GUARANTEE_OR_DIE(!DoesDefinitionExist(definition->m_name), "Duplicate data in projectile actor definitions: " + filePath);
		s_projectileActorDefinitions.push_back(definition);

		definitionElement = definitionElement->NextSiblingElement("ActorDefinition");
	}
}


ActorDefinition const* ProjectileActorDefinitions::GetProjectileActorDefinitionByName(std::string name)
{
	for (int defIndex = 0; defIndex < (int) s_projectileActorDefinitions.size(); defIndex++)
	{
		ActorDefinition const*& def = s_projectileActorDefinitions[defIndex];
		if (_stricmp(def->m_name.c_str(), name.c_str()) == 0)
		{
			return def;
		}
	}

	return nullptr;
}


bool ProjectileActorDefinitions::DoesDefinitionExist(std::string definitionName)
{
	for (int defIndex = 0; defIndex < (int) s_projectileActorDefinitions.size(); defIndex++)
	{
		ActorDefinition const*& def = s_projectileActorDefinitions[defIndex];
		if (_stricmp(def->m_name.c_str(), definitionName.c_str()) == 0)
		{
			return true;
		}
	}

	return false;
}


void ProjectileActorDefinitions::DeleteAllProjectileActorDefinitions()
{
	for (int defIndex = 0; defIndex < (int) s_projectileActorDefinitions.size(); defIndex++)
	{
		ActorDefinition const*& def = s_projectileActorDefinitions[defIndex];
		if (def)
		{
			delete def;
			def = nullptr;
		}
	}

	s_projectileActorDefinitions.clear();
}


std::vector<ActorDefinition const*> ProjectileActorDefinitions::s_projectileActorDefinitions;


void WeaponDefinition::LoadWeaponDefinitionsFromXML(std::string filePath)
{
	XmlDocument xmlDoc;
	XmlError result = xmlDoc.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(result == 0, "Unable to load file for weapon definitions: " + filePath);

	XmlElement* rootElement = xmlDoc.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "Definitions") == 0, "Weapon definitions file does not have definitions element: " + filePath);

	XmlElement* definitionElement = rootElement->FirstChildElement("WeaponDefinition");
	while (definitionElement)
	{
		WeaponDefinition* definition = new WeaponDefinition();
		definition->LoadFromXMLElement(*definitionElement);
		GUARANTEE_OR_DIE(!DoesDefinitionExist(definition->m_name), "Duplicate data in projectile weapon definitions: " + filePath);
		s_weaponDefinitions.push_back(definition);

		definitionElement = definitionElement->NextSiblingElement("WeaponDefinition");
	}
}


WeaponDefinition const* WeaponDefinition::GetWeaponDefinitionByName(std::string name)
{
	for (int defIndex = 0; defIndex < (int) s_weaponDefinitions.size(); defIndex++)
	{
		WeaponDefinition const*& def = s_weaponDefinitions[defIndex];
		if (_stricmp(def->m_name.c_str(), name.c_str()) == 0)
		{
			return def;
		}
	}

	return nullptr;
}


bool WeaponDefinition::DoesDefinitionExist(std::string definitionName)
{
	for (int defIndex = 0; defIndex < (int) s_weaponDefinitions.size(); defIndex++)
	{
		WeaponDefinition const*& def = s_weaponDefinitions[defIndex];
		if (_stricmp(def->m_name.c_str(), definitionName.c_str()) == 0)
		{
			return true;
		}
	}

	return false;
}


void WeaponDefinition::DeleteAllWeaponDefinitions()
{
	for (int defIndex = 0; defIndex < (int) s_weaponDefinitions.size(); defIndex++)
	{
		WeaponDefinition const*& def = s_weaponDefinitions[defIndex];
		if (def)
		{
			delete def;
			def = nullptr;
		}
	}

	s_weaponDefinitions.clear();
}


bool WeaponDefinition::LoadFromXMLElement(XmlElement const& node)
{
	m_name						= ParseXmlAttribute(node, "name",					m_name);
	m_refireTime				= ParseXmlAttribute(node, "refireTime",				m_refireTime);
	m_isAutomatic				= ParseXmlAttribute(node, "isAutomatic",			m_isAutomatic);
	
	m_numRays					= ParseXmlAttribute(node, "numRays",				m_numRays);
	m_rayLength					= ParseXmlAttribute(node, "rayLength",				m_rayLength);
	m_rayDamage					= ParseXmlAttribute(node, "rayDamage",				m_rayDamage);
	m_rayImpulse				= ParseXmlAttribute(node, "rayImpulse",				m_rayImpulse);
	m_rayConeDegrees			= ParseXmlAttribute(node, "rayCone",				m_rayConeDegrees);
	
	m_numProjectiles			= ParseXmlAttribute(node, "numProjectiles",			m_numProjectiles);
	m_projectileSpeed			= ParseXmlAttribute(node, "projectileSpeed",		m_projectileSpeed);
	m_projectileConeDegrees		= ParseXmlAttribute(node, "projectileCone",			m_projectileConeDegrees);
	if (m_numProjectiles > 0)
	{
		std::string projectileActorName = ParseXmlAttribute(node, "projectileActor", "");
		m_projectileActorDef = ProjectileActorDefinitions::GetProjectileActorDefinitionByName(projectileActorName);
		if (m_projectileActorDef == nullptr)
		{
			ERROR_AND_DIE("Invalid projectile actor definition name in weapon definition: " + m_name);
		}
	}

	m_sfx = ParseXmlAttribute(node, "fireSounds", m_sfx);
	m_ownerAttackAnimation = ParseXmlAttribute(node, "ownerAttackAnimation", m_ownerAttackAnimation);

	XmlElement const* hudElement = node.FirstChildElement("HUD");
	if (hudElement)
	{
		if (!m_hud.LoadFromXML(*hudElement))
		{
			ERROR_AND_DIE("Unable to load hud data for weapon definition: " + m_name);
		}
	}

	return true;
}


std::vector<WeaponDefinition const*> WeaponDefinition::s_weaponDefinitions;

Weapon::Weapon(WeaponDefinition const* weaponDefinition, Actor* owningActor)
	: m_definition(weaponDefinition)
	, m_owningActor(owningActor)
{
	m_attackSfxId = g_audioSystem->CreateOrGetSound(m_definition->m_sfx);
}


void Weapon::Attack(bool isFreshAttack, Vec3 const& rayOrigin, Vec3 const& spawnOrigin, Vec3 const& attackForward)
{
	if (!CanAttack())
		return;

	if (!isFreshAttack && !IsAutomatic())
		return;

	Map* map = m_owningActor->GetMap();
	if (m_definition->m_numRays > 0 || m_definition->m_numProjectiles > 0)
	{
		int numRays			= m_definition->m_numRays;
		int numProjectiles  = m_definition->m_numProjectiles;
		if (numRays > 0)
		{
			float rayImpulse = m_definition->m_rayImpulse;
			float raycastDistance = m_definition->m_rayLength;
			FloatRange damageRange = m_definition->m_rayDamage;

			for (int rayIndex = 0; rayIndex < numRays; rayIndex++)
			{
				Vec3 raycastForward = GetRandomForwardInCone(attackForward, m_definition->m_rayConeDegrees);
				RaycastResult hitResult = map->RayCastAll(rayOrigin, raycastForward, raycastDistance, m_owningActor->GetActorUID());
				//DebugAddWorldLine(rayOrigin, rayOrigin + raycastForward * 5.0f, Rgba8::GREY, 0.025f, 10.0f);
				if (hitResult.m_didImpact)
				{
					SpawnInfo attackEffect;
					attackEffect.m_position = hitResult.m_impactPoint + hitResult.m_impactNormal * 0.01f;
					attackEffect.m_faction = ACTOR_FACTION_NEUTRAL;
					if (hitResult.m_impactActorUID.IsValid())
					{
						int damage = g_rng->GetRandomIntInRange((int) damageRange.m_min, (int) damageRange.m_max);
						Actor* hitActor = map->GetActorWithUID(hitResult.m_impactActorUID);
						if (!hitActor->IsDead())
						{
							hitActor->TakeDamage(damage);
							Vec3 impactNormal = hitResult.m_impactNormal;
							Vec3 impulse = -impactNormal * rayImpulse;
							hitActor->ApplyImpulse(impulse);
							attackEffect.m_definition = ActorDefinition::GetActorDefinition("BloodSplatter");
							map->SpawnActor(attackEffect);
							continue;
						}
						continue;
					}
					attackEffect.m_definition = ActorDefinition::GetActorDefinition("BulletHit");
					map->SpawnActor(attackEffect);
				}
			}
		}

		if (numProjectiles > 0)
		{
			for (int projectileIndex = 0; projectileIndex < numProjectiles; projectileIndex++)
			{
				Vec3 projectileDirection = GetRandomForwardInCone(attackForward, m_definition->m_projectileConeDegrees);
				//DebugAddWorldLine(spawnOrigin, spawnOrigin + projectileDirection * 5.0f, Rgba8::GREY, 0.025f, 10.0f);
				SpawnInfo projectileInfo;
				projectileInfo.m_owningActor = m_owningActor->GetActorUID();
				projectileInfo.m_definition = m_definition->m_projectileActorDef;
				projectileInfo.m_position = spawnOrigin;
				projectileInfo.m_orientation = m_owningActor->GetOrientation();
				projectileInfo.m_initialVelocity = projectileDirection * m_definition->m_projectileSpeed;
				projectileInfo.m_faction = ACTOR_FACTION_PROJECTILE;
				map->SpawnActor(projectileInfo);
			}
		}
		g_audioSystem->StartSound(m_attackSfxId, false, 0.2f);
		m_attackTimer.Start((float) m_definition->m_refireTime);
		m_owningActor->SetAnimationToPlay(m_definition->m_ownerAttackAnimation);
	}
}


bool Weapon::CanAttack() const
{
	return m_attackTimer.HasElapsed() || m_attackTimer.GetDuration() == 0.0;
}


bool Weapon::IsAutomatic() const
{
	return m_definition->m_isAutomatic;
}


Texture* Weapon::GetReticle() const
{
	return m_definition->m_hud.m_reticleTexture;
}


Vec2 Weapon::GetReticleDims() const
{
	return m_definition->m_hud.m_reticleSize;
}


SpriteDefinition Weapon::GetWeaponSprite() const
{
	WeaponHud hud = m_definition->m_hud;

	std::string animationToPlay;
	animationToPlay = "Idle";
	if (!CanAttack())
		animationToPlay = "Attack";

	WeaponAnimation weaponAnimation = hud.GetAnimationByName(animationToPlay);
	float animationDuration = (int) weaponAnimation.m_frameIndices.size() * weaponAnimation.m_secondsPerFrame;
	SpriteAnimDefinition animation = SpriteAnimDefinition(*hud.m_spriteSheet, weaponAnimation.m_frameIndices, animationDuration);
	SpriteDefinition animationSprite = animation.GetSpriteDefAtTime((float) m_attackTimer.GetElapsedSeconds());

	return animationSprite;
}


Vec2 Weapon::GetSpriteScale() const
{
	return m_definition->m_hud.m_spriteScale;
}


Vec3 Weapon::GetRandomForwardInCone(Vec3 const& refForwardNormal, float coneDegrees)
{
	Mat44 lookAtMat = Mat44::CreateLookAtMatrix_XFwd_YLeft_ZUp(refForwardNormal);

	float halfConeDegrees = coneDegrees * 0.5f;
	float randomConeDegrees = g_rng->GetRandomFloatInRange(0.0f, halfConeDegrees);
	float randomRotationDegrees = g_rng->GetRandomFloatInRange(0.0f, 360.0f);

	Vec3 rotationOffset = lookAtMat.GetKBasis3D() * SinDegrees(randomRotationDegrees) + lookAtMat.GetJBasis3D() * CosDegrees(randomRotationDegrees);
	Vec3 offsetVector = rotationOffset * TanDegrees(randomConeDegrees);

	return (refForwardNormal + offsetVector).GetNormalized();
}
