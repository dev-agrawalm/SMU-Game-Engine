#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Game/GameCommon.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/EntityID.hpp"

struct EntitySpawnInfo
{
public:
	static const EntitySpawnInfo s_INVALID_SPAWN_INFO;

public:
	EntitySpawnInfo(EntityType type);
	//EntitySpawnInfo(EntityType type, EntityDefinition* definition);
	void Render(Rgba8 const& tint) const;
	void DebugRender() const;
	bool IsInvalid() const;
	bool IsValid() const;

public:
	EntityType m_type = ENTITY_TYPE_NONE;
	Vec2 m_startingPosition;
	bool m_isSelected = false;
	EntityID m_entityId = EntityID::s_INVALID_ID;
	EntityID m_targetWaypointId = EntityID::s_INVALID_ID;
	EntityFacingDirection m_direction = ENTITY_FACING_RIGHT;

private:
	EntityDefinition const* m_definition = nullptr;
};
