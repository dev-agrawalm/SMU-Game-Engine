#include "Game/Bullet.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

int g_numChildBullets = 0;

Bullet::Bullet(Vec2 const& position, float orientation, EntityFaction faction, BulletDefinition const* bulletDef)
	: Entity(position, faction)
	, m_definition(bulletDef)
{
	m_orientationDegrees = orientation;
	m_physicsRadius		= m_definition->m_physicsRadius;
	m_cosmeticRadius	= m_definition->m_cosmeticRadius;
	m_damageOnCollision = m_definition->m_damage;
	m_lifetime			= m_definition->m_lifetime;
	m_speed				= m_definition->m_speed;
	m_dieOnImpact		= m_definition->m_dieOnImpact;
	m_isBullet			= true;
}


void Bullet::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (IsDead())
		return;

	if (m_lifetime > 0.0f && m_age >= m_lifetime)
	{
		Die();
		return;
	}

	m_position += m_speed * GetForwardNormal() * deltaSeconds;
	m_cosmeticOrientation += 90.0f * deltaSeconds;

	static float worldX = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
	static float worldY = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);
	if (m_position.x <= -m_cosmeticRadius ||
		m_position.x >= worldX + m_cosmeticRadius ||
		m_position.y <= -m_cosmeticRadius ||
		m_position.y >= worldY + m_cosmeticRadius)
	{
		m_isGarbage = true;
	}

	m_age += deltaSeconds;
}


void Bullet::Render() const
{
	if (IsDead())
		return;

	AABB2 bulletSpriteBox = AABB2(Vec2::ZERO, m_cosmeticRadius, m_cosmeticRadius);
	std::vector<Vertex_PCU> bulletSpriteVerts;
	SpriteDefinition bulletSpriteDef = g_gameSpriteSheet->GetSpriteDefinition(m_definition->m_spriteCoords.x + m_definition->m_spriteCoords.y * 12);
	Vec2 uvMins;
	Vec2 uvMaxs;
	bulletSpriteDef.GetUVs(uvMins, uvMaxs);
	AddVertsForAABB2ToVector(bulletSpriteVerts, bulletSpriteBox, Rgba8::WHITE, uvMins, uvMaxs);
	TransformVertexArrayXY3D((int) bulletSpriteVerts.size(), bulletSpriteVerts.data(), 1.0f, m_cosmeticOrientation, m_position);
	g_theRenderer->BindTexture(0, &bulletSpriteDef.GetTexture());
	g_theRenderer->DrawVertexArray((int) bulletSpriteVerts.size(), bulletSpriteVerts.data());
	
	if (g_game->IsDebugModeActive())
	{
		DebugRender();
	}
}


void Bullet::Die()
{
	m_isDead = true;
	m_isGarbage = true;

	if (m_definition->m_childBulletCount > 0 && m_health > 0)
	{
		static float explodeSfxVolume = g_gameConfigBlackboard.GetValue("bulletExplosionSfxVolume", 0.7f);
		ShakeScreenAndPlaySfx(0.15f, 0.25f, "Data/Audio/BulletExplosion3.wav", explodeSfxVolume);

		int childBulletCount = m_definition->m_childBulletCount;
		float angularInterval = DEGREES_360 / childBulletCount;
		float orientation = g_rng->GetRandomFloatInRange(0.0f, DEGREES_360);
		for (int childBulletIndex = 0; childBulletIndex < childBulletCount; childBulletIndex++)
		{
			SpawnInfo spawnInfo = {};
			spawnInfo.m_position = m_position;
			spawnInfo.m_entityType = m_faction == ENTITY_FACTION_PLAYER ? ENTITY_TYPE_PLAYER_BULLET : ENTITY_TYPE_ENEMY_BULLET;
			std::string childBulletName = m_definition->m_childBulletName;
			BulletDefinition const* childBulletDefinition = BulletDefinition::GetBulletDefinitionByName(childBulletName);
			GUARANTEE_OR_DIE(childBulletDefinition != nullptr, Stringf("Invalid definition for child bullet. Bullet Def: %s | Child Bullet Def: %s", m_definition->m_name.c_str(), childBulletName.c_str()));
			spawnInfo.m_bulletDefinition = childBulletDefinition;
			float randomnessMagnitude = 10.0f;
			float orientationOffset = g_rng->GetRandomFloatInRange(-randomnessMagnitude, randomnessMagnitude);
			spawnInfo.m_orientation = orientation + orientationOffset;
			g_game->SpawnEntity(spawnInfo);
			g_numChildBullets++;
			orientation += angularInterval;
		}
	}
}


void BulletDefinition::LoadBulletDefinitionsFromXml()
{
	std::string bulletDefinitionsFileName = g_gameConfigBlackboard.GetValue("bulletDefinitionsFileName", "Data/XMLData/BulletDefinitions.xml");
	XmlDocument bulletDefinitionDoc;
	XmlError result = bulletDefinitionDoc.LoadFile(bulletDefinitionsFileName.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Unable to open bullet definitions xml file. TinyXml Error code: %i", result));

	XmlElement const* rootElement = bulletDefinitionDoc.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "BulletDefinitions") == 0, "Bullet Definition xml does not contain the BulletDefinitions root element");

	XmlElement const* definition = rootElement->FirstChildElement("Bullet");
	while (definition)
	{
		std::string name = ParseXmlAttribute(*definition, "name", "");
		GUARANTEE_OR_DIE(!DoesDefinitionAlreadyExist(name), Stringf("Duplicate data in bullet xml. Duplicate Name: %s", name.c_str()));
		BulletDefinition* bulletDefinition = new BulletDefinition(*definition);
		s_bulletDefinitions.push_back(bulletDefinition);

		definition = definition->NextSiblingElement("Bullet");
	}
}


BulletDefinition const* BulletDefinition::GetBulletDefinitionByName(std::string const& name)
{
	for (int bulletIndex = 0; bulletIndex < (int) s_bulletDefinitions.size(); bulletIndex++)
	{
		BulletDefinition const*& definition = s_bulletDefinitions[bulletIndex];
		if (_stricmp(definition->m_name.c_str(), name.c_str()) == 0)
		{
			return definition;
		}
	}

	return nullptr;
}


bool BulletDefinition::DoesDefinitionAlreadyExist(std::string const& definitionName)
{
	for (int splineIndex = 0; splineIndex < (int) s_bulletDefinitions.size(); splineIndex++)
	{
		BulletDefinition const*& definition = s_bulletDefinitions[splineIndex];
		if (_stricmp(definition->m_name.c_str(), definitionName.c_str()) == 0)
		{
			return true;
		}
	}

	return false;
}


void BulletDefinition::DeleteAllBulletDefinitions()
{
	for (int bulletDefIndex = 0; bulletDefIndex < (int) s_bulletDefinitions.size(); bulletDefIndex++)
	{
		BulletDefinition const*& definition = s_bulletDefinitions[bulletDefIndex];
		if (definition)
		{
			delete definition;
			definition = nullptr;
		}
	}
	s_bulletDefinitions.clear();
}


BulletDefinition::BulletDefinition(XmlElement const& xmlElement)
{
	m_name = ParseXmlAttribute(xmlElement, "name", m_name);
	m_speed = ParseXmlAttribute(xmlElement, "speed", m_speed);
	m_damage = ParseXmlAttribute(xmlElement, "damage", m_damage);
	m_lifetime = ParseXmlAttribute(xmlElement, "lifetime", m_lifetime);
	m_childBulletCount = ParseXmlAttribute(xmlElement, "numChildBullets", m_childBulletCount);
	m_childBulletName = ParseXmlAttribute(xmlElement, "childBullet", "");
	m_spriteCoords = ParseXmlAttribute(xmlElement, "spriteCoords", m_spriteCoords);
	//new stuff
	m_physicsRadius = ParseXmlAttribute(xmlElement, "physicsRadius", m_physicsRadius);
	m_cosmeticRadius = ParseXmlAttribute(xmlElement, "cosmeticRadius", m_cosmeticRadius);
	m_dieOnImpact = ParseXmlAttribute(xmlElement, "dieOnImpact", m_dieOnImpact);
}


std::vector<BulletDefinition const*> BulletDefinition::s_bulletDefinitions;
