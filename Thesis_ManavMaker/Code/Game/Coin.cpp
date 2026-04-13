// #include "Game/Coin.hpp"
// #include "Engine/Core/EngineCommon.hpp"
// #include "Engine/Renderer/Renderer.hpp"
// #include "Game/GameCommon.hpp"
// #include "Engine/Math/IntVec2.hpp"
// #include "Engine/Renderer/SpriteSheet.hpp"
// #include "Engine/Core/VertexUtils.hpp"
// #include "Game/Level.hpp"
// #include "Engine/Math/Vec2.hpp"
// #include "Game/Game.hpp"
// #include "Engine/Renderer/SpriteAnimDefinition.hpp"
// 
// Coin::Coin(Level* level, Vec2 const& position)
// 	: Entity(level, position)
// {
// 	m_collidesWithTiles = false;
// 	m_type = ENTITY_TYPE_COIN;
// 	m_localPhysicsBounds = GetPhysicsBoundsForEntity(m_type);
// 	m_localCosmeticBounds = GetCosmeticBoundsForEntity(m_type);
// 
// 	m_deathDuration = g_gameConfigBlackboard.GetValue("coinCollectDuration", 2.0f);
// 	m_deathAnimationDuration = g_gameConfigBlackboard.GetValue("coinCollectAnimationDuration", 0.25f);
// 	m_deathAnimDefinition = new SpriteAnimDefinition(*g_spriteSheet_32x32, 34, 38, m_deathAnimationDuration, SpriteAnimPlaybackMode::PING_PONG);
// }
// 
// 
// void Coin::Update(float deltaSeconds)
// {
// 	if (m_deathAnimationTimer >= 0.0f)
// 	{
// 		m_deathAnimationTimer += deltaSeconds;
// 		if (m_deathAnimationTimer >= m_deathDuration)
// 		{
// 			m_isGarbage = true;
// 		}
// 	}
// }
// 
// 
// void Coin::Render() const
// {
// 	if (g_game->IsDebugModeActive())
// 	{
// 		DebugRender();
// 	}
// 
// 	if (m_isGarbage)
// 		return;
// 
// 	AABB2 cosmeticBox = GetWorldCosmeticBounds();
// 	static std::vector<Vertex_PCU> verts;
// 	verts.clear();
// 	Vec2 maxs;
// 	Vec2 mins;
// 	IntVec2 spriteCoords = GetEditorSpriteCoordsForEntity(m_type);
// 	g_spriteSheet_32x32->GetSpriteUVs(mins, maxs, spriteCoords);
// 	Rgba8 tint = Rgba8::WHITE;
// 	if (m_deathAnimationTimer >= 0.0f)
// 	{
// 		SpriteDefinition deathSprite = m_deathAnimDefinition->GetSpriteDefAtTime(m_deathAnimationTimer);
// 		deathSprite.GetUVs(mins, maxs);
// 		static float scale = g_gameConfigBlackboard.GetValue("coinCollectAnimScale", 5.0f);
// 		cosmeticBox.UniformScaleFromCenter(scale);
// 		if (g_gameConfigBlackboard.GetValue("coinCollectBlend", false))
// 		{
// 			g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
// 		}
// 	}
// 	AddVertsForAABB2ToVector(verts, cosmeticBox, tint, mins, maxs);
// 	g_theRenderer->BindTexture(0, g_spriteSheetTexture);
// 	g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
// 	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
// }
// 
// 
// void Coin::Die()
// {
// 	if (!m_isDead)
// 		m_deathAnimationTimer = 0.0f;
// 
// 	m_isDead = true;
// }
// 
