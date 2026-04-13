#include "Game/LevelEndFlag.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Game.hpp"

// LevelEndFlag::LevelEndFlag(Level* level, Vec2 const& position)
// 	:Entity(level, position)
// {
// 	m_collidesWithTiles = false;
// 	m_type = ENTITY_TYPE_LEVEL_END;
// 	m_localPhysicsBounds = GetPhysicsBoundsForEntity(m_type);
// 	m_localCosmeticBounds = GetCosmeticBoundsForEntity(m_type);
// 	m_levelEndAnimDuration = g_gameConfigBlackboard.GetValue("levelEndAnimDuration", 1.0f);
// }

// 
// void LevelEndFlag::Update(float deltaSeconds)
// {
// 	m_levelEndAnimTimer += deltaSeconds;
// }


// void LevelEndFlag::Render() const
// {
// 	if (g_game->IsDebugModeActive())
// 	{
// 		DebugRender();
// 	}
// 
// 	AABB2 cosmeticBox = GetLocalCosmeticBounds();
// 	static std::vector<Vertex_PCU> verts;
// 	verts.clear();
// 	Vec2 maxs;
// 	Vec2 mins;
// 	static IntVec2 spriteCoords = GetEditorSpriteCoordsForEntity(m_type);
// 	static SpriteSheet* spriteSheet = GetSpriteSheetForEntity(m_type);
// 	spriteSheet->GetSpriteUVs(mins, maxs, spriteCoords);
// 	AddVertsForAABB2ToVector(verts, cosmeticBox, Rgba8::WHITE, mins, maxs);
// 	float orientation = 0.0f;
// 	if(m_levelEndAnimStartTime > 0.0f)
// 	{
// 		float animationStartTime = m_levelEndAnimStartTime;
// 		float animationEndTime = m_levelEndAnimStartTime + m_levelEndAnimDuration;
// 		orientation = RangeMapClamped_SmoothStart4(m_levelEndAnimTimer, animationStartTime, animationEndTime, 0.0f, 90.0f);
// 	}
// 	TransformVertexArrayXY3D((int) verts.size(), verts.data(), 1.0f, orientation, m_bottomCenterPosition);
// 	g_theRenderer->BindTexture(0, g_spriteSheetTexture);
// 	g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
// }


void LevelEndFlag::StartLevelEndAnimation()
{
	m_levelEndAnimStartTime = (float) GetCurrentTimeSeconds();
	m_levelEndAnimTimer = m_levelEndAnimStartTime;
}

