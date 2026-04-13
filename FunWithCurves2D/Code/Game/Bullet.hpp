#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/IntVec2.hpp"


struct BulletDefinition
{
public:
	static void LoadBulletDefinitionsFromXml();
	static BulletDefinition const* GetBulletDefinitionByName(std::string const& name);
	static bool DoesDefinitionAlreadyExist(std::string const& definitionName);
	static void DeleteAllBulletDefinitions();
public:
	BulletDefinition(XmlElement const& xmlElement);

public:
	std::string		m_name;
	float			m_speed = 0.0f;
	int				m_damage = 0;
	float			m_lifetime = -1.0f;
	int				m_childBulletCount = 0;
	std::string		m_childBulletName;
	float			m_physicsRadius = 0.0f;
	float			m_cosmeticRadius = 0.0f;
	bool			m_dieOnImpact = true;
	IntVec2 m_spriteCoords;

protected:
	static std::vector<BulletDefinition const*> s_bulletDefinitions;
};


class Bullet : public Entity
{
public:
	Bullet(Vec2 const& position, float orientation, EntityFaction faction, BulletDefinition const* bulletDef);
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;
private:
	BulletDefinition const* m_definition = nullptr;
	float m_cosmeticOrientation = 0.0f;
};
