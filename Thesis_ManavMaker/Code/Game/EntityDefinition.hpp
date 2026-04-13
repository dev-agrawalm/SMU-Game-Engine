#pragma once
#include "Engine/Core/NamedProperties.hpp"
#include "Game/GameCommon.hpp"
#include<vector>
#include "Engine\Core\Vertex_PCU.hpp"
#include "Engine\Core\Rgba8.hpp"
#include "Engine\Math\AABB2.hpp"

typedef NamedProperties EntityProperties;

//structure containing all properties for an entity's definition
struct EntityDefinition
{
public:
	static void InitialiseDefinitions();
	static void AddDefinition(EntityType type, EntityProperties const& properties);
	static EntityDefinition const* GetEntityDefinitionByType(EntityType type);
	static SpriteDefinition const* GetEditorSpriteDefinitionForEntity(EntityType type);
	static SpriteDefinition const* GetPaletteSpriteDefinitionForEntity(EntityType type);
	static AABB2 GetPhysicsBoundsForEntity(EntityType type, Vec2 const& translation);
	static AABB2 GetCosmeticBoundsForEntity(EntityType type, Vec2 const& translation);

public:
	EntityAnimStateData GetAnimation(AnimationState state) const;
	void AddVertsForEditorSpriteToAABB2(std::vector<Vertex_PCU>& out_verts, AABB2 const& refAABB2, Rgba8 const& tint) const;
	AABB2 GetPhysicsBounds(Vec2 const& translation = Vec2::ZERO) const;
	AABB2 GetCosmeticBounds(Vec2 const& translation = Vec2::ZERO) const;

public:
	EntityProperties m_properties;
	
private:
	static EntityDefinition s_defaultDefinitions[ENTITY_TYPE_COUNT];
};
