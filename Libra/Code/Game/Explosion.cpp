#include "Game/Explosion.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <vector>

Explosion::Explosion(Vec2 const& startingPosition, float orientationDegrees, EntityFaction faction, EntityType type, Map* map)
	:Entity(startingPosition, orientationDegrees, faction, type, map)
{
	m_lifetime = 0.0f;
	m_isPushedByWalls = false;
	m_isPushedByEntities = false;
	m_doesPushEntities = false;

	switch (m_type)
	{
		case ENTITY_TYPE_TANK_EXPLOSION:
			m_duration = g_gameConfigBlackboard.GetValue("tankExplosionDuration", 0.0f);
			m_aabb2SideLength = g_gameConfigBlackboard.GetValue("tankExplosionAABB2SideLength", 0.0f);
			break;
		case ENTITY_TYPE_BULLET_EXPLOSION:
			m_duration = g_gameConfigBlackboard.GetValue("bulletExplosionDuration", 0.0f);
			m_aabb2SideLength = g_gameConfigBlackboard.GetValue("bulletExplosionAABB2SideLength", 0.0f);
			break;
		case ENTITY_TYPE_PLAYER_EXPLOSION:
			m_duration = g_gameConfigBlackboard.GetValue("playerExplosionDuration", 0.0f);
			m_aabb2SideLength = g_gameConfigBlackboard.GetValue("playerExplosionAABB2SideLength", 0.0f);
			break;
	}

	std::string animTexturePath = g_gameConfigBlackboard.GetValue("explosionAnimationSpriteSheetPath", "");
	IntVec2 animSpriteSheetDims = g_gameConfigBlackboard.GetValue("explosionAnimationSpriteSheetDims", IntVec2(0, 0));
	int startSpriteIndex		= g_gameConfigBlackboard.GetValue("explosionAnimationStartSpriteIndex", 0);
	int endSpriteIndex			= g_gameConfigBlackboard.GetValue("explosionAnimationEndSpriteIndex", 0);
	Texture* animTexture = g_theRenderer->CreateOrGetTexture(animTexturePath.c_str());
	SpriteSheet* explosionAnimSpriteSheet = new SpriteSheet(*animTexture, animSpriteSheetDims);
	m_explosionAnimDef = new SpriteAnimDefinition(*explosionAnimSpriteSheet, startSpriteIndex, endSpriteIndex, m_duration);
}


Explosion::~Explosion()
{
}


void Explosion::Update(float deltaSeconds)
{
	m_lifetime += deltaSeconds;
	if (m_lifetime >= m_duration)
	{
		Die();
		m_isGarbage = true;

		delete m_explosionAnimDef;
		m_explosionAnimDef = nullptr;
	}
}


void Explosion::Render() const
{
	if (m_isDead)
		return;

	SpriteDefinition animSpriteDef = m_explosionAnimDef->GetSpriteDefAtTime(m_lifetime);
	AABB2 spriteBounds = AABB2(Vec2::ZERO, m_aabb2SideLength, m_aabb2SideLength);
	std::vector<Vertex_PCU> renderVertexes;
	Vec2 uvMins;
	Vec2 uvMaxs;
	animSpriteDef.GetUVs(uvMins, uvMaxs);
	AddVertsForAABB2ToVector(renderVertexes, spriteBounds, Rgba8::WHITE, uvMins, uvMaxs);
	TransformVertexArrayXY3D((int) renderVertexes.size(), renderVertexes.data(), 1.0f, m_orientationDegrees, m_position);
	Texture const* spriteAnimTexture = &(animSpriteDef.GetTexture());
	g_theRenderer->BindTexture(0, (Texture*) spriteAnimTexture);
	g_theRenderer->DrawVertexArray((int) renderVertexes.size(), renderVertexes.data());
}
