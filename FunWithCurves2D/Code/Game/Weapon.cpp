#include "Game/Weapon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

Weapon::Weapon(WeaponDefinition const* definition, Entity* owner)
	: m_definition(definition)
	, m_owner(owner)
{
	m_fireTimer.Start((double) definition->m_fireRate);
	m_fireTimer.Pause();
}


bool Weapon::Attack(float attackOrientationDegrees)
{
	if (!m_fireTimer.IsPaused() && !m_fireTimer.HasElapsed())
		return false;

	Vec2 ownerPos = m_owner->GetPosition();
	EntityType bulletType = GetBulletType();
	float bulletSpawnOffsetLength = m_owner->GetCosmeticRadius();
	float attackOrientation = attackOrientationDegrees;

	switch (m_definition->m_type)
	{
		case WEAPON_TYPE_SINGLE_SHOT:
		{
			Vec2 ownerForward = m_owner->GetForwardNormal();
			Vec2 bulletPos = ownerPos + ownerForward * bulletSpawnOffsetLength;
			SpawnInfo bulletSpawnInfo = {};
			bulletSpawnInfo.m_position = bulletPos;
			bulletSpawnInfo.m_entityType = bulletType;
			bulletSpawnInfo.m_bulletDefinition = m_definition->m_bulletDefinition;
			bulletSpawnInfo.m_orientation = attackOrientation;
			g_game->SpawnEntity(bulletSpawnInfo);
			break;
		}
		case WEAPON_TYPE_MULTI_SHOT:
		{
			int numBullets = m_definition->m_numBullets;
			float attackConeAngleHalf = m_definition->m_attackConeAngleDegrees * 0.5f;
			for (int i = 0; i < numBullets; i++)
			{
				float bulletOrientation =  g_rng->GetRandomFloatInRange(attackOrientation - attackConeAngleHalf, attackOrientation + attackConeAngleHalf);
				Vec2 bulletPos = ownerPos + Vec2::MakeFromPolarDegrees(bulletOrientation) * bulletSpawnOffsetLength;
				SpawnInfo bulletSpawnInfo = {};
				bulletSpawnInfo.m_position = bulletPos;
				bulletSpawnInfo.m_entityType = bulletType;
				bulletSpawnInfo.m_bulletDefinition = m_definition->m_bulletDefinition;
				bulletSpawnInfo.m_orientation = bulletOrientation;
				g_game->SpawnEntity(bulletSpawnInfo);
			}
			break;
		}
		case WEAPON_TYPE_INVALID:
		default:
			break;
	}

	m_fireTimer.Restart();
	return true;
}


void Weapon::SetOwner(Entity* owner)
{
	m_owner = owner;
}


EntityType Weapon::GetBulletType() const
{
	EntityFaction ownerFaction = m_owner->GetFaction();
	switch (ownerFaction)
	{
		case ENTITY_FACTION_PLAYER:	return ENTITY_TYPE_PLAYER_BULLET;
		case ENTITY_FACTION_ENEMY:	return ENTITY_TYPE_ENEMY_BULLET;
	}

	return ENTITY_TYPE_INVALID;
}


WeaponDefinition const* Weapon::GetWeaonDefinition() const
{
	return m_definition;
}


std::string Weapon::GetWeaponName() const
{
	return m_definition->m_name;
}


void WeaponDefinition::LoadWeaponDefinitionsFromXml()
{
	std::string weaponDefinitionsFileName = g_gameConfigBlackboard.GetValue("weaponDefinitionsFileName", "Data/XMLData/WeaponDefinitions.xml");
	XmlDocument weaponDefinitionDoc;
	XmlError result = weaponDefinitionDoc.LoadFile(weaponDefinitionsFileName.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Unable to open weapon definitions xml file. TinyXml Error code: %i", result));

	XmlElement const* rootElement = weaponDefinitionDoc.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "WeaponDefinitions") == 0, "Bullet Definition xml does not contain the BulletDefinitions root element");

	XmlElement const* definition = rootElement->FirstChildElement("Weapon");
	while (definition)
	{
		std::string name = ParseXmlAttribute(*definition, "name", "");
		GUARANTEE_OR_DIE(!DoesDefinitionAlreadyExist(name), Stringf("Duplicate data in weapon xml. Duplicate Name: %s", name.c_str()));
		WeaponDefinition* weaponDefinition = new WeaponDefinition(*definition);
		s_weaponDefinitions.push_back(weaponDefinition);

		definition = definition->NextSiblingElement("Weapon");
	}
}


WeaponDefinition const* WeaponDefinition::GetWeaponDefinitionByName(std::string const& name)
{
	for (int bulletIndex = 0; bulletIndex < (int) s_weaponDefinitions.size(); bulletIndex++)
	{
		WeaponDefinition const*& definition = s_weaponDefinitions[bulletIndex];
		if (_stricmp(definition->m_name.c_str(), name.c_str()) == 0)
		{
			return definition;
		}
	}

	return nullptr;
}


bool WeaponDefinition::DoesDefinitionAlreadyExist(std::string const& definitionName)
{
	for (int splineIndex = 0; splineIndex < (int) s_weaponDefinitions.size(); splineIndex++)
	{
		WeaponDefinition const*& definition = s_weaponDefinitions[splineIndex];
		if (_stricmp(definition->m_name.c_str(), definitionName.c_str()) == 0)
		{
			return true;
		}
	}

	return false;
}


void WeaponDefinition::DeleteAllWeaponDefinitions()
{
	for (int splineIndex = 0; splineIndex < (int) s_weaponDefinitions.size(); splineIndex++)
	{
		WeaponDefinition const*& definition = s_weaponDefinitions[splineIndex];
		if (definition)
		{
			delete definition;
			definition = nullptr;
		}
	}
	s_weaponDefinitions.clear();
}


WeaponDefinition::WeaponDefinition(XmlElement const& xmlElement)
{
	m_name = ParseXmlAttribute(xmlElement, "name", m_name);
	m_type = (WeaponType) ParseXmlAttribute(xmlElement, "type", -1);
	m_fireRate = ParseXmlAttribute(xmlElement, "fireRate", m_fireRate);
	m_numBullets = ParseXmlAttribute(xmlElement, "numBullets", m_numBullets);
	std::string bullet = ParseXmlAttribute(xmlElement, "bullet", "");
	m_bulletDefinition = BulletDefinition::GetBulletDefinitionByName(bullet);
	m_attackConeAngleDegrees = ParseXmlAttribute(xmlElement, "attackConeAngleDegrees", m_attackConeAngleDegrees);
	GUARANTEE_OR_DIE(m_bulletDefinition != nullptr, Stringf("Invalid bullet name in the weapons xml data. Weapon: %s | Bullet: %s", m_name.c_str(), bullet.c_str()));
}


std::vector<WeaponDefinition const*> WeaponDefinition::s_weaponDefinitions;
