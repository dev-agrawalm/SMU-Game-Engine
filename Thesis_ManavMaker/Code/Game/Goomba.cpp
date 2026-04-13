// #include "Game/Goomba.hpp"
// #include "Engine/Core/EngineCommon.hpp"
// #include "Game/GameCommon.hpp"
// #include "Game/Game.hpp"
// #include "Engine/Core/Vertex_PCU.hpp"
// #include "Engine/Renderer/Renderer.hpp"
// #include "Engine/Renderer/SpriteSheet.hpp"
// #include "Engine/Core/VertexUtils.hpp"
// #include "Game/Level.hpp"
// #include "Engine/Renderer/SpriteAnimDefinition.hpp"
// #include "Engine/Core/Time.hpp"
// 
// Goomba::Goomba(Level* level, Vec2 const& position)
// 	: Entity(level, position)
// {
// 	m_collidesWithTiles = true;
// 	m_type = ENTITY_TYPE_GOOMBA;
// 	m_localPhysicsBounds = GetPhysicsBoundsForEntity(m_type);
// 	m_localCosmeticBounds = GetCosmeticBoundsForEntity(m_type);
// 	m_speed = GOOMBA_SPEED;
// 	m_targetVec = Vec2(1.0f, 0.0f);
// 
// 	Vec2 halfDimsPhysics = m_localPhysicsBounds.GetDimensions() * 0.5f;
// 	m_localBottomLeft = Vec2(-halfDimsPhysics.x, 0.0f);
// 	m_localBottomRight = Vec2(halfDimsPhysics.x, 0.0f);
// 
// 	m_deathAnimationDuration = g_gameConfigBlackboard.GetValue("goombaDeathAnimationDuration", 0.25f);
// 	m_deathAnimDefinition = new SpriteAnimDefinition(*g_spriteSheet_16x16, 16, 19, m_deathAnimationDuration);
// }
// 
// 
// void Goomba::Update(float deltaSeconds)
// {
// 	if (m_isGrounded)
// 	{
// 		float groundedThreshold = 0.1f;
// 		LevelRaycastResult bottomLeftRaycast = m_level->RaycastLevel(m_bottomCenterPosition + m_localBottomLeft, Vec2(0.0f, -1.0f), groundedThreshold, true);
// 		LevelRaycastResult bottomRightRaycast = m_level->RaycastLevel(m_bottomCenterPosition + m_localBottomRight, Vec2(0.0f, -1.0f), groundedThreshold, true);
// 		LevelRaycastResult bottomCenterRaycast = m_level->RaycastLevel(m_bottomCenterPosition, Vec2(0.0f, -1.0f), groundedThreshold, true);
// 		if (!bottomLeftRaycast.m_didHit && !bottomCenterRaycast.m_didHit && !bottomRightRaycast.m_didHit)
// 		{
// 			m_isGrounded = false;
// 		}
// 
// 		m_velocity = m_targetVec * m_speed;
// 		Vec2 newPosition = m_bottomCenterPosition + m_velocity * deltaSeconds;
// 		Vec2 posToCheck;
// 		if (m_targetVec == Vec2(1.0f, 0.0f))
// 		{
// 			posToCheck = newPosition + m_localBottomRight;
// 		}
// 		else
// 		{
// 			posToCheck = newPosition + m_localBottomLeft;
// 		}
// 
// 		IntVec2 refPosTileCoords = m_level->GetTileCoords(posToCheck);
// 		IntVec2 floorTileCoords = refPosTileCoords - IntVec2(0, 1);
// 		if (m_level->IsTileCoordValid(refPosTileCoords))
// 		{
// 			Tile tile = m_level->GetTile(refPosTileCoords);
// 			if (!tile.IsSolid() && !tile.DoesDamage())
// 			{
// 				if (m_level->IsTileCoordValid(floorTileCoords))
// 				{
// 					Tile floorTile = m_level->GetTile(floorTileCoords);
// 					if (floorTile.IsSolid(true) && !floorTile.DoesDamage())
// 					{
// 						m_bottomCenterPosition = newPosition;
// 						return;
// 					}
// 				}
// 			}
// 		}
// 
// 		m_targetVec *= -1.0f;
// 	}
// 	else
// 	{
// 		static Vec2 gravity = GRAVITY * Vec2(0.0, -1.0f);
// 		m_velocity += gravity * deltaSeconds;
// 		m_bottomCenterPosition += m_velocity * deltaSeconds;
// 	}
// 
// 	if (m_deathAnimationTimer >= 0.0f)
// 	{
// 		m_deathAnimationTimer += deltaSeconds;
// 		if (m_deathAnimationTimer >= m_deathAnimationDuration)
// 		{
// 			m_isGarbage = true;
// 		}
// 	}
// }
// 
// 
// void Goomba::Render() const
// {
// 	if (g_game->IsDebugModeActive())
// 	{
// 		DebugRender();
// 	}
// 
// 	if (m_isGarbage)
// 		return;
// 
// 	static float duration = 0.25f;
// 	static int goombaAnimStartIndex = g_spriteSheet_128x128->GetSpriteIndex(IntVec2(4, 2));
// 	static int goombaAnimEndIndex = g_spriteSheet_128x128->GetSpriteIndex(IntVec2(5, 2));
// 	static SpriteAnimDefinition animation = SpriteAnimDefinition(*g_spriteSheet_128x128, goombaAnimStartIndex, goombaAnimEndIndex, duration, SpriteAnimPlaybackMode::PING_PONG);
// 
// 	AABB2 cosmeticBox = GetWorldCosmeticBounds();
// 	static std::vector<Vertex_PCU> verts;
// 	verts.clear();
// 	Vec2 maxs;
// 	Vec2 mins;
// 	IntVec2 spriteCoords = GetEditorSpriteCoordsForEntity(m_type);
// 	float time = (float) GetCurrentTimeSeconds();
// 	SpriteDefinition sprite = animation.GetSpriteDefAtTime(time);
// 	sprite.GetUVs(mins, maxs);
// 	if (m_deathAnimationTimer >= 0.0f)
// 	{
// 		SpriteDefinition deathSprite = m_deathAnimDefinition->GetSpriteDefAtTime(m_deathAnimationTimer);
// 		deathSprite.GetUVs(mins, maxs);
// 	}
// 	
// 	Vec2 uvMins = mins;
// 	Vec2 uvMaxs = maxs;
// 	if (m_targetVec == Vec2(-1.0f, 0.0f))
// 	{
// 		uvMins = Vec2(maxs.x, mins.y);
// 		uvMaxs = Vec2(mins.x, maxs.y);
// 	}
// 	AddVertsForAABB2ToVector(verts, cosmeticBox, Rgba8::WHITE, uvMins, uvMaxs);
// 	g_theRenderer->BindTexture(0, g_spriteSheetTexture);
// 	g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
// }
// 
// 
// void Goomba::Die()
// {
// 	if (!m_isDead)
// 	{
// 		m_deathAnimationTimer = 0.0f;
// 		m_triggerResponses[TRIGGER_TYPE_ON_DEATH].Execute();
// 	}
// 	m_isDead = true;
// }
