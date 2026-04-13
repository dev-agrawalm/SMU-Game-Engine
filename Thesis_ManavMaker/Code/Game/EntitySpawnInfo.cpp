#include "Game/EntitySpawnInfo.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"

const EntitySpawnInfo EntitySpawnInfo::s_INVALID_SPAWN_INFO = EntitySpawnInfo(ENTITY_TYPE_NONE);

EntitySpawnInfo::EntitySpawnInfo(EntityType type)
	:m_type(type)
{
	if (m_type != ENTITY_TYPE_COUNT && m_type != ENTITY_TYPE_NONE)
	{
		m_definition = EntityDefinition::GetEntityDefinitionByType(m_type);
	}
}


void EntitySpawnInfo::Render(Rgba8 const& tint) const
{
	if (g_game->IsDebugModeActive())
	{
		DebugRender();
	}

	AABB2 cosmeticBox = m_definition->GetCosmeticBounds(m_startingPosition);// GetCosmeticBoundsForEntity(m_type, m_startingPosition);
	static std::vector<Vertex_PCU> verts;
	verts.clear();
	m_definition->AddVertsForEditorSpriteToAABB2(verts, cosmeticBox, tint);
	g_theRenderer->BindTexture(0, g_spriteSheetTexture);
	g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());

	if (m_isSelected)
	{
		g_theRenderer->BindTexture(0, nullptr);
		AABB2 entityBounds = m_definition->GetPhysicsBounds(m_startingPosition);// GetPhysicsBoundsForEntity(m_type, m_startingPosition);
		DebugDrawAABB2Outline(entityBounds, 0.05f, Rgba8::MAGENTA);
	}
}


void EntitySpawnInfo::DebugRender() const
{
	float width = 0.05f;
	Rgba8 physicsColor = Rgba8::CYAN;
	Rgba8 cosmeticColor = Rgba8::MAGENTA;

	AABB2 physicsBox =  m_definition->GetPhysicsBounds(m_startingPosition);
	AABB2 cosmeticBox = m_definition->GetCosmeticBounds(m_startingPosition);
	DebugDrawAABB2Outline(cosmeticBox, width, cosmeticColor);
	DebugDrawAABB2Outline(physicsBox, width, physicsColor);
}


bool EntitySpawnInfo::IsInvalid() const
{
	return m_type == ENTITY_TYPE_NONE || m_entityId.IsInvalid();
}


bool EntitySpawnInfo::IsValid() const
{
	return m_type != ENTITY_TYPE_NONE && m_entityId.IsValid() && m_type != ENTITY_TYPE_COUNT;
}
