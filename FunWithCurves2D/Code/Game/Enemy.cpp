#include "Game/Enemy.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Spline.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

static int s_enemyID = -10000;

Enemy::Enemy(EnemyDefinition const* definition)
	: m_definition(definition)
	, Entity(Vec2::ZERO, ENTITY_FACTION_ENEMY)
{
	m_id = s_enemyID;
	s_enemyID++;
	//m_position			= Vec2(100.0, 80.0f);
	m_speed				= m_definition->m_speed;
	m_cosmeticRadius	= m_definition->m_cosmeticRadius;
	m_physicsRadius		= m_definition->m_physicsRadius;
	m_health			= m_definition->m_health;
	m_damageOnCollision = m_definition->m_contactDamage;

	m_splineDefinition = m_definition->m_splineDefinition;
	WeaponDefinition const* weaponDef = m_definition->m_weaponDefinition;
	m_weapon = new Weapon(weaponDef, this);
	m_angularVelocity = 180.0f;
}


Enemy::~Enemy()
{
	if (m_weapon)
	{
		delete m_weapon;
		m_weapon = nullptr;
	}

	Enemy* latestSpawnedEntity = g_game->GetLatestSpawnedEntity();
	if (latestSpawnedEntity && latestSpawnedEntity->GetId() == m_id)
	{
		g_game->SetLatestSpawnedEntity(nullptr);
	}

	if (m_enemyBehind)
	{
		m_enemyBehind->SetEnemyInFront(nullptr);
		m_enemyBehind = nullptr;
	}

	if (m_enemyInFront)
	{
		m_enemyInFront->SetEnemyBehind(nullptr);
		m_enemyInFront = nullptr;
	}
}


void Enemy::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (IsDead())
	{
		static float deathAnimationDuration = g_gameConfigBlackboard.GetValue("enemyDeathAnimationDuration", 2.0f);
		if (m_deathAnimationTime > deathAnimationDuration)
		{
			m_isGarbage = true;
			return;
		}
		m_deathAnimationTime += deltaSeconds;
		return;
	}

	m_animationTimer += deltaSeconds;

	if (!m_reachedEndOfPath)
	{
		float nextFractionOfSpline = m_currentFractionOfSpline + m_speed * deltaSeconds;
		nextFractionOfSpline = ClampZeroToOne(nextFractionOfSpline);
		Vec2 newPosition = m_splineDefinition->m_spline.GetPointAtFraction(nextFractionOfSpline);

		bool collidingWithEnemyInFront = false;
		if (m_enemyInFront && m_enemyInFront->IsAlive())
		{
			Vec2 enemyInFrontPos = m_enemyInFront->GetPosition();
			float enemyInFrontRadius = m_enemyInFront->GetPhysicsRadius();
			collidingWithEnemyInFront = DoDiscsOverlap2D(newPosition, m_physicsRadius, enemyInFrontPos, enemyInFrontRadius);
		}

		if ( !collidingWithEnemyInFront && m_currentFractionOfSpline <= 1.0f)
		{
			Vec2 tangent = newPosition - m_position;
			m_orientationDegrees = tangent.GetOrientationDegrees();
			m_position = newPosition;
			m_currentFractionOfSpline = nextFractionOfSpline;
		}
		else
		{
			m_reachedEndOfPath = true;
			static float worldX = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeX", 0.0f);
			static float worldY = g_gameConfigBlackboard.GetValue("worldCamOrthoSizeY", 0.0f);
			if (m_position.x <= -m_cosmeticRadius ||
				m_position.x >= worldX + m_cosmeticRadius ||
				m_position.y <= -m_cosmeticRadius ||
				m_position.y >= worldY + m_cosmeticRadius)
			{
				MakeGarbage();
			}
		}
	}
	else if(GetShortestAngularDisplacementDegrees(m_orientationDegrees, -90.0f) != 0.0f)
	{
		float orientationDelta = GetTurnedTowardDegrees(m_orientationDegrees, -90.0f, m_angularVelocity * deltaSeconds);
		m_orientationDegrees = orientationDelta;
	}

	Player* player = g_game->GetPlayer();
	Vec2 playerPos = player->GetPosition();
	if (player->IsAlive() && IsPointInsideDirectedSector2D(playerPos, m_position, Vec2(0.0f, -1.0f), m_definition->m_attackConeAngleDegrees, 100.0f))
	{
		bool canAiFire = g_gameConfigBlackboard.GetValue("debugCanAIFire", true);
		if (m_weapon && canAiFire)
		{
			Attack();
		}
	}
}


void Enemy::Render() const
{
	if (g_game->IsDebugModeActive())
	{
		DebugRender();
		m_splineDefinition->m_spline.Render();
		Vec2 lineVector1 = Vec2::MakeFromPolarDegrees(-90 - (m_definition->m_attackConeAngleDegrees * 0.5f), 100.0f);
		Vec2 lineVector2 = Vec2::MakeFromPolarDegrees(-90 + (m_definition->m_attackConeAngleDegrees * 0.5f), 100.0f);
		DrawLine(m_position, lineVector1, 0.02f, Rgba8::MAGENTA);
		DrawLine(m_position, lineVector2, 0.02f, Rgba8::MAGENTA);
	}
	
	Vec2 uvMins;
	Vec2 uvMaxs;

	if (IsGarbage())
		return;

	if (IsDead())
	{
		static float deathAnimationDuration = g_gameConfigBlackboard.GetValue("enemyDeathAnimationDuration", 1.0f);
		static SpriteAnimDefinition deathAnim = SpriteAnimDefinition(*g_gameSpriteSheet, 7, 11, deathAnimationDuration, SpriteAnimPlaybackMode::ONCE);
		SpriteDefinition deathAnimSpriteDef = deathAnim.GetSpriteDefAtTime(m_deathAnimationTime);
		deathAnimSpriteDef.GetUVs(uvMins, uvMaxs);
	}
	else
	{
		SpriteDefinition animSpriteDef = m_definition->m_animationDefinition->GetSpriteDefAtTime(m_animationTimer);
		animSpriteDef.GetUVs(uvMins, uvMaxs);
	}

	AABB2 enemySpriteBox = AABB2(Vec2::ZERO, m_cosmeticRadius, m_cosmeticRadius);
	std::vector<Vertex_PCU> enemyVerts;
	AddVertsForAABB2ToVector(enemyVerts, enemySpriteBox, Rgba8::WHITE, uvMins, uvMaxs);
	TransformVertexArrayXY3D((int) enemyVerts.size(), enemyVerts.data(), 1.0f, m_orientationDegrees, m_position);

	Texture const& spriteSheetTexture = g_gameSpriteSheet->GetTexture();
	g_theRenderer->BindTexture(0, &spriteSheetTexture);
	g_theRenderer->DrawVertexArray((int) enemyVerts.size(), enemyVerts.data());
}


void Enemy::Die()
{
	m_isDead = true;
	g_game->AddPointsToPlayerScore(m_definition->m_pointsOnDeath);
	m_deathAnimationTime = 0.0f;
	static float deathSfxVolume = g_gameConfigBlackboard.GetValue("enemyDeathSfxVolume", 0.7f);
	ShakeScreenAndPlaySfx(0.15f, 0.25f, "Data/Audio/ShipExplosion3_394127__morganpurkis__space-explosion-with-reverb.wav", deathSfxVolume);
}


void Enemy::TakeDamage(int damage)
{
	m_health -= damage;

	if (m_health <= 0)
	{
		Die();
		return;
	}

	static float hitSfxVolume = g_gameConfigBlackboard.GetValue("enemyHitSfxVolume", 0.7f);
	ShakeScreenAndPlaySfx(0.15f, 0.25f, "Data/Audio/LaserImpact2_459782__metzik__deflector-shield.wav", hitSfxVolume);
}


void Enemy::Attack()
{
	if (m_weapon->Attack(-90.0f))
	{
		static float fireSfxVolume = g_gameConfigBlackboard.GetValue("enemyFireSfxVolume", 0.7f);
		ShakeScreenAndPlaySfx(0.15f, 0.25f, "Data/Audio/EnemyFireSfx_234083__211redman112__lasgun-fire.ogg", fireSfxVolume);
	}
}


void Enemy::SetEnemyBehind(Enemy* enemy)
{
	m_enemyBehind = enemy;
}


void Enemy::SetEnemyInFront(Enemy* enemy)
{
	m_enemyInFront = enemy;
}


int Enemy::GetId() const
{
	return m_id;
}


void EnemyDefinition::LoadEnemyDefinitionsFromXml()
{
	std::string enemyDefinitionsFileName = g_gameConfigBlackboard.GetValue("enemyDefinitionsFileName", "Data/XMLData/EnemyDefinitions.xml");
	XmlDocument enemyDefinitionDoc;
	XmlError result = enemyDefinitionDoc.LoadFile(enemyDefinitionsFileName.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Unable to open enemy definitions xml file. TinyXml Error code: %i", result));

	XmlElement const* rootElement = enemyDefinitionDoc.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "EnemyDefinitions") == 0, "Enemy Definition xml does not contain the EnemyDefinitions root element");

	XmlElement const* definition = rootElement->FirstChildElement("Definition");
	while (definition)
	{
		std::string name = ParseXmlAttribute(*definition, "name", "");
		GUARANTEE_OR_DIE(!DoesDefinitionAlreadyExist(name), Stringf("Duplicate data in enemy xml. Duplicate Name: %s", name.c_str()));
		EnemyDefinition* enemyDefinition = new EnemyDefinition(*definition);
		s_enemyDefinitions.push_back(enemyDefinition);

		definition = definition->NextSiblingElement("Definition");
	}
}


EnemyDefinition const* EnemyDefinition::GeEnemyDefinitionByName(std::string const& name)
{
	for (int enemyIndex = 0; enemyIndex < (int) s_enemyDefinitions.size(); enemyIndex++)
	{
		EnemyDefinition const*& definition = s_enemyDefinitions[enemyIndex];
		if (_stricmp(definition->m_name.c_str(), name.c_str()) == 0)
		{
			return definition;
		}
	}

	return nullptr;
}


bool EnemyDefinition::DoesDefinitionAlreadyExist(std::string const& definitionName)
{
	for (int enemyIndex = 0; enemyIndex < (int) s_enemyDefinitions.size(); enemyIndex++)
	{
		EnemyDefinition const*& definition = s_enemyDefinitions[enemyIndex];
		if (_stricmp(definition->m_name.c_str(), definitionName.c_str()) == 0)
		{
			return true;
		}
	}

	return false;
}


void EnemyDefinition::DeleteAllEnemyDefinitions()
{
	for (int enemyIndex = 0; enemyIndex < (int) s_enemyDefinitions.size(); enemyIndex++)
	{
		EnemyDefinition const*& definition = s_enemyDefinitions[enemyIndex];
		if (definition)
		{
			delete definition;
			definition = nullptr;
		}
	}
	s_enemyDefinitions.clear();
}


EnemyDefinition const* EnemyDefinition::GetDefinitionByIndex(int index)
{
	if (index < 0 || index >= (int) s_enemyDefinitions.size())
		return nullptr;

	return s_enemyDefinitions[index];
}


int EnemyDefinition::GetEnemyDefinitionCount()
{
	return (int) s_enemyDefinitions.size();
}


EnemyDefinition::EnemyDefinition(XmlElement const& xmlElement)
{
	m_name = ParseXmlAttribute(xmlElement, "name", "");
	std::string splineName = ParseXmlAttribute(xmlElement, "splineName", "");
	m_splineDefinition = SplineDefinition::GetSplineDefinitionByName(splineName);
	GUARANTEE_OR_DIE(m_splineDefinition != nullptr, Stringf("Invalid spline name in enemy definition. Enemy Definition name: %s | spine definition name: %s", m_name.c_str(), splineName.c_str()));
	std::string weaponName = ParseXmlAttribute(xmlElement, "weaponName", "");
	m_weaponDefinition = WeaponDefinition::GetWeaponDefinitionByName(weaponName);
	GUARANTEE_OR_DIE(m_weaponDefinition != nullptr, Stringf("Invalid weapon name in enemy definition. Enemy Definition name: %s | weapon definition name: %s", m_name.c_str(), weaponName.c_str()));
	m_pointsOnDeath = ParseXmlAttribute(xmlElement, "points", m_pointsOnDeath);
	m_physicsRadius = ParseXmlAttribute(xmlElement, "physicsRadius", m_physicsRadius);
	m_cosmeticRadius = ParseXmlAttribute(xmlElement, "cosmeticRadius", m_cosmeticRadius);
	m_speed = ParseXmlAttribute(xmlElement, "speed", m_speed);
	m_health = ParseXmlAttribute(xmlElement, "health", m_health);
	m_contactDamage = ParseXmlAttribute(xmlElement, "contactDamage", m_contactDamage);
	m_attackConeAngleDegrees = ParseXmlAttribute(xmlElement, "attackConeAngleDegree", m_attackConeAngleDegrees);

	XmlElement const* animationElement = xmlElement.FirstChildElement("Animation");
	LoadAnimationFromXml(*animationElement);
}


EnemyDefinition::~EnemyDefinition()
{
	if (m_animationDefinition)
	{
		delete m_animationDefinition;
		m_animationDefinition = nullptr;
	}
}


void EnemyDefinition::LoadAnimationFromXml(XmlElement const& xmlElement)
{
	float duration = ParseXmlAttribute(xmlElement, "duration", 0.0f);
	XmlElement const* frameElement = xmlElement.FirstChildElement("Frame");
	std::vector<int> frameSpriteIndices;
	while (frameElement)
	{
		IntVec2 spriteCellCoords = ParseXmlAttribute(*frameElement, "spriteCellCoords", IntVec2::ZERO);
		int spriteIndex = spriteCellCoords.x + spriteCellCoords.y * 12;
		frameSpriteIndices.push_back(spriteIndex);

		frameElement = frameElement->NextSiblingElement("Frame");
	}

	m_animationDefinition = new SpriteAnimDefinition(*g_gameSpriteSheet, frameSpriteIndices, duration, SpriteAnimPlaybackMode::LOOP);
}


std::vector<EnemyDefinition const*> EnemyDefinition::s_enemyDefinitions;
