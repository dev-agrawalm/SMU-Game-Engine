#pragma once
#include <string>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Stopwatch.hpp"

enum WeaponType
{
	WEAPON_TYPE_INVALID = -1,
	WEAPON_TYPE_SINGLE_SHOT,
	WEAPON_TYPE_MULTI_SHOT,
	WEAPON_TYPE_COUNT
};

enum EntityType : int;
struct BulletDefinition;
class Entity;

struct WeaponDefinition
{
public:
	static void LoadWeaponDefinitionsFromXml();
	static WeaponDefinition const* GetWeaponDefinitionByName(std::string const& name);
	static bool DoesDefinitionAlreadyExist(std::string const& definitionName);
	static void DeleteAllWeaponDefinitions();
public:
	WeaponDefinition(XmlElement const& xmlElement);

public:
	std::string m_name;
	float m_fireRate = 0.0f;
	WeaponType m_type = WEAPON_TYPE_INVALID;
	int m_numBullets = 0;
	BulletDefinition const* m_bulletDefinition = nullptr;
	float m_attackConeAngleDegrees = 0.0f;

protected:
	static std::vector<WeaponDefinition const*> s_weaponDefinitions;
};


class Weapon
{
public:
	Weapon(WeaponDefinition const* definition, Entity* owner);

	bool Attack(float attackOrientationDegrees);
	void SetOwner(Entity* owner);

	EntityType GetBulletType() const;
	WeaponDefinition const* GetWeaonDefinition() const;
	std::string GetWeaponName() const;
private:
	WeaponDefinition const* m_definition = nullptr;
	Entity* m_owner = nullptr;
	Stopwatch m_fireTimer;
};
