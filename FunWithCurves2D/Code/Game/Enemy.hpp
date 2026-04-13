#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

struct WeaponDefinition;
struct SplineDefinition;
class Weapon;
class SpriteAnimDefinition;

struct EnemyDefinition
{
public:
	static void LoadEnemyDefinitionsFromXml();
	static EnemyDefinition const* GeEnemyDefinitionByName(std::string const& name);
	static bool DoesDefinitionAlreadyExist(std::string const& definitionName);
	static void DeleteAllEnemyDefinitions();
	static EnemyDefinition const* GetDefinitionByIndex(int index);
	static int GetEnemyDefinitionCount();
public:
	EnemyDefinition(XmlElement const& xmlElement);
	~EnemyDefinition();

private:
	void LoadAnimationFromXml(XmlElement const& xmlElement);

public:
	std::string m_name;
	int m_health = 0;
	int m_pointsOnDeath = 0;
	int m_contactDamage = 0;
	float m_speed = 0.0f;
	float m_physicsRadius = 0.0f;
	float m_cosmeticRadius = 0.0f;
	float m_attackConeAngleDegrees = -1.0f;
	SpriteAnimDefinition* m_animationDefinition = nullptr;
	SplineDefinition const* m_splineDefinition = nullptr;
	WeaponDefinition const* m_weaponDefinition = nullptr;

protected:
	static std::vector<EnemyDefinition const*> s_enemyDefinitions;
};



class Enemy : public Entity
{
public:
	Enemy(EnemyDefinition const* definition);
	~Enemy();
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void TakeDamage(int damage) override;

	void Attack();

	void SetEnemyBehind(Enemy* entity);
	void SetEnemyInFront(Enemy* enemy);

	int GetId() const;

private:
	int m_id = 0;
	EnemyDefinition const* m_definition = nullptr;
	SplineDefinition const* m_splineDefinition = nullptr;

	float m_animationTimer = 0.0f;
	float m_deathAnimationTime = 0.0f;

	float m_currentFractionOfSpline = 0.0f;
	bool m_reachedEndOfPath = false;

	Weapon* m_weapon = nullptr;

	Enemy* m_enemyBehind = nullptr;
	Enemy* m_enemyInFront = nullptr;
};
