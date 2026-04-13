#pragma once
#include <string>
#include "Engine/Math/FloatRange.hpp"
#include <vector>
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class ActorDefinition;

struct WeaponAnimation
{
public:
	bool LoadFromXML(XmlElement const& node);

public:
	std::string m_name;
	float m_secondsPerFrame = 0.25f; // "frameTime"
	float m_framesPerSecond = 4.0f;  // derived from above
	std::vector<int> m_frameIndices;
};


struct WeaponHud
{
public:
	bool LoadFromXML(XmlElement const& node);
	WeaponAnimation GetAnimationByName(std::string animationName);

public:
	Texture* m_reticleTexture = nullptr;
	Vec2 m_reticleSize;
	SpriteSheet* m_spriteSheet = nullptr;
	Vec2 m_spriteScale;
	IntVec2 m_cellCount;
	std::vector<WeaponAnimation> m_animations;
};


struct ProjectileActorDefinitions
{
public:
	static void LoadProjectileActorDefinitionsFromXML(std::string filePath);
	static ActorDefinition const* GetProjectileActorDefinitionByName(std::string name);
	static bool DoesDefinitionExist(std::string definitionName);
	static void DeleteAllProjectileActorDefinitions();
protected:
	static std::vector<ActorDefinition const*> s_projectileActorDefinitions;
};


struct WeaponDefinition
{
public:
	static void LoadWeaponDefinitionsFromXML(std::string filePath);
	static WeaponDefinition const* GetWeaponDefinitionByName(std::string name);
	static bool DoesDefinitionExist(std::string definitionName);
	static void DeleteAllWeaponDefinitions();

public:
	bool LoadFromXMLElement(XmlElement const& node);

public:
	std::string m_name;
	float m_refireTime = 0.0f;
	bool m_isAutomatic = false;

	int m_numRays = 0;
	float m_rayLength = 0.0f;
	float m_rayImpulse = 0.0f;
	float m_rayConeDegrees = 0.0f;
	FloatRange m_rayDamage = FloatRange::ZERO;
	
	int m_numProjectiles = 0;
	float m_projectileConeDegrees = 0.0f;
	float m_projectileSpeed = 0.0f;
	ActorDefinition const* m_projectileActorDef = nullptr;

	WeaponHud m_hud;
	std::string m_sfx;
	std::string m_ownerAttackAnimation;

protected:
	static std::vector<WeaponDefinition const*> s_weaponDefinitions;
};

struct Vec3;
class Actor;
class SpriteDefinition;

class Weapon
{
public:
	Weapon(WeaponDefinition const* weaponDefinition, Actor* owningActor);

	void Attack(bool isFreshAttack, Vec3 const& rayOrigin, Vec3 const& spawnOrigin, Vec3 const& forward);
	
	bool CanAttack() const;
	bool IsAutomatic() const;
	Texture* GetReticle() const;
	Vec2 GetReticleDims() const;
	SpriteDefinition GetWeaponSprite() const;
	Vec2 GetSpriteScale() const;

protected:
	Vec3 GetRandomForwardInCone(Vec3 const& refForwardNormal, float coneDegrees);

private:
	WeaponDefinition const* m_definition = nullptr;
	Actor* m_owningActor;
	Stopwatch m_attackTimer;
	SoundID m_attackSfxId;
};
