#include "Game/Level.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Player.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Goomba.hpp"
#include "Game/Game.hpp"
#include "Game/Coin.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/LevelEndFlag.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/Vec4.hpp"
#include <map>
#include "Engine/Input/InputSystem.hpp"
#include "Game/LevelEditor.hpp"

Level::Level()
{
	InitGrid();
}


bool Level::LoadLevelFromPath(std::string const& levelPath)
{
	XmlDocument levelXml;
	XmlError result = levelXml.LoadFile(levelPath.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Unable to open level xml: %s", levelPath.c_str()));

	XmlElement* rootElement = levelXml.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "Level") == 0, Stringf("Level xml does not have the correct root element: %s", levelPath.c_str()));

	//root element for the tile grid
	XmlElement* gridElement = rootElement->FirstChildElement("Grid");
	//root element for the entity data
	XmlElement* spawnDataElement = rootElement->FirstChildElement("SpawnData");
	if (gridElement)
	{
		LoadGridFromXmlElement(gridElement);
	}

	if (spawnDataElement)
	{
		LoadEntitySpawnDataFromXmlElement(spawnDataElement);
	}

	return true;
}


void Level::Init_EditorMode()
{
	
}


void Level::Init_PlayMode()
{
	for (int i = 0; i < m_spawnData.size(); i++)
	{
		SpawnEntity(m_spawnData[i]);
	}

	static float gameCamOrthoX = g_gameConfigBlackboard.GetValue("gameplayCamOrthoX", 0.0f);
	static float gameCamOrthoY = g_gameConfigBlackboard.GetValue("gameplayCamOrthoY", 0.0f);
	m_gameplayCam.SetOrthoView(Vec2::ZERO, Vec2(gameCamOrthoX, gameCamOrthoY));

	static float uiOrthoX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	static float uiOrthoY = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeY", 0.0f);
	m_uiCam.SetOrthoView(Vec2::ZERO, Vec2(uiOrthoX, uiOrthoY));

	UpdateCamera();

	m_gameOverStartTime = -1.0f;
	m_isGameCompletelyOver = false;
	m_gameOverState = GAME_OVER_STATE_NONE;

	//bool muteLevel = g_gameConfigBlackboard.GetValue("muteLevel", false);
	//float bgMusicVolume = muteLevel ? 0.0f : g_gameConfigBlackboard.GetValue("levelBgMusicVolume", 1.0f);
	SoundID bgMusicSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/LevelBackgroudMusic.mp3");
	m_bgMusicPlaybackId = g_audioSystem->StartSound(bgMusicSoundId, true, g_gameMusicVolume);

	m_curFrameOverlappingEntityIds.reserve(500);
	m_prevFrameOverlappingEntityIds.reserve(500);

	m_levelClock.Unpause();
}


void Level::InitGrid()
{
	TileDefinition const* emptyTileDef = TileDefinition::GetDefinitionByName("Empty");
	for (int i = 0; i < LEVEL_TILE_COUNT; i++)
	{
		m_grid[i].m_definition = emptyTileDef;
		m_grid[i].m_index = i;
	}
}


void Level::Deinit_EditorMode()
{

}


void Level::Deinit_PlayMode()
{
	for (int i = 0; i < (int) m_allEntities.size(); i++)
	{
		Entity* e = m_allEntities[i];
		if (e)
		{
			Entity* removedEntity = RemoveEntityFromLevel(e);
			delete removedEntity;
			removedEntity = nullptr;
			i--;
		}
	}

	g_audioSystem->StopSound(m_bgMusicPlaybackId);

	m_curFrameOverlappingEntityIds.clear();
	m_prevFrameOverlappingEntityIds.clear();
}


void Level::Update()
{
	CheckInput();
	float deltaSeconds = (float) m_levelClock.GetFrameDeltaSeconds();
	deltaSeconds = Clamp(deltaSeconds, 0.0f, FRAME_TIME_10_FPS);

	HandleEntityEntityCollisions();

	for (int listIndex = 0; listIndex < ENTITY_TYPE_COUNT; listIndex++)
	{
		EntityList& list = m_entitiesByType[listIndex];
		for (int entityIndex = 0; entityIndex < list.size(); entityIndex++)
		{
			Entity*& e = list[entityIndex];
			if (e)
			{
				e->Update(deltaSeconds);
			}
		}
	}

	HandleCollisionBetweenEntityAndTiles();
	CollectGarbage();
	UpdateCamera();

	//DebugAddScreenText(Stringf("Num Entities: %i", m_allEntities.size()), Vec2(0.0f, 0.0f), 0.0f, Vec2(0.0f, 0.0f), 16.0f);

	//DebugAddScreenText(Stringf("Press Z to make bowser attack"), Vec2(1800.0f, 900.0f), 0.0f, Vec2(1.0f, 1.0f), 12.0f);

	if ((int) m_entitiesByType[ENTITY_TYPE_MARIO].size() > 0)
	{
		Entity* player = m_entitiesByType[ENTITY_TYPE_MARIO][0];
		if (player)
		{
			//DebugAddScreenText(Stringf("Health: %i", player->m_health), Vec2::ZERO, 0.0f, Vec2::ZERO, 12.0f);

			if (player->m_isDead && m_gameOverStartTime < 0.0f && !m_isGameCompletelyOver)
			{
				TriggerGameOver(GAME_OVER_STATE_LOSS);
			}
		}
	}

	if (m_gameOverStartTime > 0.0f)
	{
		float currentTime = (float) GetCurrentTimeSeconds();
		if (currentTime - m_gameOverStartTime > m_gameOverDuration)
		{
			m_gameOverStartTime = -1.0f;
			m_isGameCompletelyOver = true;
			g_eventSystem->FireEvent("GameOverEvent");
		}
	}
}


bool Level::IsLevelPaused() const
{
	return m_levelClock.IsPaused();
}


void Level::CheckInput()
{
	if (g_inputSystem->WasKeyJustPressed('O'))
	{
		m_levelClock.StepFrame();
	}

	if (g_inputSystem->WasKeyJustPressed('P'))
	{
		m_levelClock.TogglePause();
	}

	if (g_inputSystem->WasKeyJustPressed('M'))
	{
		m_levelClock.SetTimeScale(1.0);
	}
	else if (g_inputSystem->WasKeyJustPressed('B'))
	{
		m_levelClock.SetTimeScale(0.2);
	}
	else if (g_inputSystem->WasKeyJustPressed('N'))
	{
		m_levelClock.SetTimeScale(5.0);
	}
}


void Level::HandleEntityEntityCollisions()
{
	m_curFrameOverlappingEntityIds.clear();
	for (int entityAIndex = 0; entityAIndex < (int) m_allEntities.size(); entityAIndex++)
	{
		Entity* entityA = m_allEntities[entityAIndex];
		for (int entityBIndex = entityAIndex + 1; entityBIndex < (int) m_allEntities.size(); entityBIndex++)
		{
			Entity* entityB = m_allEntities[entityBIndex];
			if (entityA && entityB && entityA != entityB)
			{
				uint32_t collisionLayersA = entityA->m_collisionLayers;
				uint32_t collisionLayersB = entityB->m_collisionLayers;
				if((collisionLayersA & collisionLayersB) == 0)
					continue;

				bool doesACollide = entityA->IsAlive() || (entityA->m_isDead && entityA->HasOptions(ENTITY_OPTIONS_COLLIDES_WHILE_DEAD));
				bool doesBCollide = entityB->IsAlive() || (entityB->m_isDead && entityB->HasOptions(ENTITY_OPTIONS_COLLIDES_WHILE_DEAD));
				if (!doesBCollide || !doesACollide)
					continue;

				AABB2 refBounds = entityA->GetWorldPhysicsBounds();
				AABB2 boundsToCheck = entityB->GetWorldPhysicsBounds();
				AABB2CollisionData collisionData = GetCollisionDataForAABB2s(refBounds, boundsToCheck);
				if (!collisionData.m_isTouching)
					continue;

				if (entityB->HasOptions(ENTITY_OPTIONS_CAN_BE_STEPPED_ON))
				{
					if (collisionData.m_shortestOverlappingSide == AABB2_BOTTOM_SIDE)
					{
						entityA->SetGrounded(true);
						Vec2 entityBDisplacement = entityB->m_displacementDueWaypoint;
						if (entityBDisplacement != Vec2::ZERO)
						{
							entityA->m_position += entityBDisplacement;
						}
					}
				}

				if (entityA->HasOptions(ENTITY_OPTIONS_CAN_BE_STEPPED_ON))
				{
					if (collisionData.m_shortestOverlappingSide == AABB2_TOP_SIDE)
					{
						entityB->SetGrounded(true);
						Vec2 entityADisplacement = entityA->m_displacementDueWaypoint;
						if (entityADisplacement != Vec2::ZERO)
						{
							entityB->m_position += entityADisplacement;
						}
					}
				}

				if (collisionData.m_isOverlapping)
				{
					uint32_t entityIdA32bit = entityA->m_entityId.m_id;
					uint32_t entityIdB32bit = entityB->m_entityId.m_id;
					uint64_t entityPairLowBits = (uint64_t) GetMin(entityIdA32bit, entityIdB32bit);
					uint64_t entityPairHighBits = (uint64_t) GetMax(entityIdA32bit, entityIdB32bit);
					uint64_t touchingEntityPairId = entityPairHighBits << 32 | entityPairLowBits;
					m_curFrameOverlappingEntityIds.push_back(touchingEntityPairId);
				}
			}
		}
	}

	for (int touchingEntityIdIndex = 0; touchingEntityIdIndex < m_curFrameOverlappingEntityIds.size(); touchingEntityIdIndex++)
	{
		uint64_t curFrameOverlappingEntityId = m_curFrameOverlappingEntityIds[touchingEntityIdIndex];
		uint32_t entityAId = (uint32_t) (curFrameOverlappingEntityId & 0x00000000ffffffff);
		uint32_t entityBId = (uint32_t) ((curFrameOverlappingEntityId & 0xffffffff00000000) >> 32);
		Entity* entityA = GetEntityById(entityAId);
		Entity* entityB = GetEntityById(entityBId);
		if (entityA == nullptr || entityB == nullptr)
			continue;

		if (!WereEntitiesOverlappingPrevFrame(curFrameOverlappingEntityId))
		{
			HandleOverlapBetweenEntities(entityA, entityB);
		}

		bool isAStatic = entityA->HasOptions(ENTITY_OPTIONS_IS_STATIC) /*&& !entityA->HasOptions(ENTITY_OPTIONS_PUSHED_BY_ENTITIES)*/;
		bool isBStatic = entityB->HasOptions(ENTITY_OPTIONS_IS_STATIC) /*&& !entityB->HasOptions(ENTITY_OPTIONS_PUSHED_BY_ENTITIES)*/;
		if (isAStatic && isBStatic)
			continue;

		if (entityA->m_type != ENTITY_TYPE_FIREBALL && entityB->m_type != ENTITY_TYPE_FIREBALL &&
			entityA->m_type != ENTITY_TYPE_COIN && entityB->m_type != ENTITY_TYPE_COIN)
		{
			if (isAStatic)
			{
				PushEntityOutOfEntity(entityB, entityA);
			}
			else if (isBStatic)
			{
				PushEntityOutOfEntity(entityA, entityB);
			}

			PushEntitiesOutOfEachOther(entityA, entityB);
		}
	}

	m_prevFrameOverlappingEntityIds.clear();
	for (int i = 0; i < m_curFrameOverlappingEntityIds.size(); i++)
	{
		uint64_t touchingEntityId = m_curFrameOverlappingEntityIds[i];
		m_prevFrameOverlappingEntityIds.push_back(touchingEntityId);
	}
}


void Level::HandleCollisionBetweenEntityAndTiles()
{
	for (int listIndex = 0; listIndex < ENTITY_TYPE_COUNT; listIndex++)
	{
		EntityList& list = m_entitiesByType[listIndex];
		for (int entityIndex = 0; entityIndex < list.size(); entityIndex++)
		{
			Entity*& e = list[entityIndex];
			if (e && e->IsAlive())
			{
				e->SetGrounded(false);
				std::vector<IntVec2> tileCoords;
				AddNonEmptyTileCoordsForBoundsToVector(tileCoords, e->GetWorldPhysicsBounds());
				AddNonEmptyEastTileCoordsForBoundsToVector(tileCoords, e->GetWorldPhysicsBounds());
				AddNonEmptyWestTileCoordsForBoundsToVector(tileCoords, e->GetWorldPhysicsBounds());
				AddNonEmptyNorthTileCoordsForBoundsToVector(tileCoords, e->GetWorldPhysicsBounds());
				AddNonEmptySouthTileCoordsForBoundsToVector(tileCoords, e->GetWorldPhysicsBounds());

				for (int tileCoordIndex = 0; tileCoordIndex < (int) tileCoords.size(); tileCoordIndex++)
				{
					IntVec2 tileCoord = tileCoords[tileCoordIndex];
					HandleTileEntityCollision(tileCoord, e);
				}
			}
		}
	}
}


void Level::Render(bool isEditor) const
{
	if (!isEditor)
	{
		g_theRenderer->BeginCamera(m_gameplayCam);
	}

	RenderBackground();
	RenderTiles();

	if (isEditor)
	{
		for (int tileIndex = 0; tileIndex < LEVEL_TILE_COUNT; tileIndex++)
		{
			Tile tile = m_grid[tileIndex];
			if (tile.IsTagged())
			{
				Vec2 tileCenter = GetTileCenter(tileIndex);
				RenderMaterial material = {};
				material.m_color = tile.m_taggedColor;
				g_theRenderer->DrawDisk2D(tileCenter, 0.3f, material);
			}

			if (tile.IsSelected())
			{
				AABB2 tileBounds = GetTileBounds(tileIndex);
				DebugDrawAABB2Outline(tileBounds, 0.1f, Rgba8::MAGENTA);
			}
		}

		for (int i = 0; i < m_spawnData.size(); i++)
		{
			EntitySpawnInfo const& info = m_spawnData[i];
			Camera currentCamera = g_theRenderer->GetCurrentCamera();
			AABB2 camBounds = currentCamera.GetOrthoCamBoundingBox();
			AABB2 entityBounds = EntityDefinition::GetPhysicsBoundsForEntity(info.m_type, info.m_startingPosition);

			if (DoAABB2sOverlap(entityBounds, camBounds))
			{
				info.Render(Rgba8::WHITE);
			}
		}
	}
	else
	{
		for (int listIndex = 0; listIndex < ENTITY_TYPE_COUNT; listIndex++)
		{
			EntityList const& list = m_entitiesByType[listIndex];
			for (int entityIndex = 0; entityIndex < list.size(); entityIndex++)
			{
				Entity* const& e = list[entityIndex];
				if (e)
				{
					Camera currentCamera = g_theRenderer->GetCurrentCamera();
					AABB2 camBounds = currentCamera.GetOrthoCamBoundingBox();
					AABB2 entityBounds = e->GetWorldCosmeticBounds();

					if (DoAABB2sOverlap(entityBounds, camBounds))
					{
						e->Render();
					}
				}
			}
		}
	}

	if (!isEditor)
	{
		g_theRenderer->EndCamera(m_gameplayCam);
	}

	if (!isEditor)
	{
		g_theRenderer->BeginCamera(m_uiCam);
		{
			AABB2 uiCamOrtho = m_uiCam.GetOrthoCamBoundingBox();
			Vec2 uiCamDims = uiCamOrtho.GetDimensions();
			Vec2 uiCamMinsBottomLeft = uiCamOrtho.m_mins;
			Vec2 uiCamMinsTopRight = uiCamOrtho.m_maxs;

			std::vector<Vertex_PCU> uiTextVerts;
			if ((int) m_entitiesByType[ENTITY_TYPE_MARIO].size() > 0)
			{
				Entity* player = m_entitiesByType[ENTITY_TYPE_MARIO][0];
				if (player)
				{
					Vec4 coinBoundsValues = g_gameConfigBlackboard.GetValue("coinBounds", Vec4(10.0f, 10.0f, 40.0f, 40.0f));
					AABB2 coinBounds = AABB2(coinBoundsValues.x, coinBoundsValues.y, coinBoundsValues.z, coinBoundsValues.w);
					std::vector<Vertex_PCU> hudCoinverts;
					Vec2 uvMins = Vec2::ZERO;
					Vec2 uvMaxs = Vec2::ZERO;
					EntityDefinition const* coinDef = EntityDefinition::GetEntityDefinitionByType(ENTITY_TYPE_COIN);
					coinDef->AddVertsForEditorSpriteToAABB2(hudCoinverts, coinBounds, Rgba8::WHITE);
					g_theRenderer->BindTexture(0, g_spriteSheetTexture);
					g_theRenderer->DrawVertexArray((int) hudCoinverts.size(), hudCoinverts.data());

					Vec4 coinTextBoundsValues = g_gameConfigBlackboard.GetValue("coinTextBounds", Vec4());
					AABB2 coinTextBounds = AABB2(coinTextBoundsValues.x, coinTextBoundsValues.y, coinTextBoundsValues.z, coinTextBoundsValues.w);
					float coinTextSize = g_gameConfigBlackboard.GetValue("coinTextSize", 35.0f);
					g_bitmapFont->AddVertsForTextInAABB2(uiTextVerts, coinTextBounds, coinTextSize, Stringf("x%i", player->m_collectedCoinCount), Rgba8::BLACK, 1.0f, BitmapFont::ALIGNED_CENTER_LEFT);
				}
			}

			if (m_gameOverStartTime > 0.0f)
			{
				float currentTime = (float) GetCurrentTimeSeconds();
				if (currentTime - m_gameOverStartTime <= m_gameOverDuration)
				{
					AABB2 gameOverTextBounds = AABB2(uiCamMinsBottomLeft.x + uiCamDims.x * 0.3f, uiCamMinsBottomLeft.y + uiCamDims.y * 0.5f, uiCamMinsBottomLeft.x + uiCamDims.x * 0.7f, uiCamMinsBottomLeft.y + uiCamDims.y * 0.85f);
					std::string gameOverText;
					if (m_gameOverState == GAME_OVER_STATE_WON)
					{
						gameOverText = "YOU WON";
					}
					if (m_gameOverState == GAME_OVER_STATE_LOSS)
					{
						gameOverText = "YOU LOST";
					}
					g_bitmapFont->AddVertsForTextInAABB2(uiTextVerts, gameOverTextBounds, 50.0f, Stringf("%s", gameOverText.c_str()), Rgba8::BLACK);
				}
			}

			if (IsLevelPaused())
			{
// 				AABB2 screenQuad = AABB2(uiCamMinsBottomLeft, uiCamMinsTopRight);
// 				std::vector<Vertex_PCU> quadVerts;
// 				Rgba8 quadColor = Rgba8(0, 0, 0, 150);
// 				AddVertsForAABB2ToVector(quadVerts, screenQuad, quadColor);
// 				g_theRenderer->BindTexture(0, nullptr);
// 				g_theRenderer->DrawVertexArray((int) quadVerts.size(), quadVerts.data());
// 
// 				AABB2 textBox = AABB2((uiCamMinsTopRight - uiCamMinsBottomLeft) * 0.5f, uiCamDims.x, uiCamDims.y);
// 				g_bitmapFont->AddVertsForTextInAABB2(uiTextVerts, textBox, 200.0f, Stringf("Game Paused"), Rgba8::WHITE);
// 				g_bitmapFont->AddVertsForTextInAABB2(uiTextVerts, textBox, 50.0f, Stringf("Press P to unpause"), Rgba8::WHITE, 1.0f, Vec2(0.5f, 0.35f));
			}

			if (uiTextVerts.size() > 0)
			{
				g_theRenderer->BindTexture(0, &g_bitmapFont->GetTexture());
				g_theRenderer->DrawVertexArray((int) uiTextVerts.size(), uiTextVerts.data());
			}
		}
		g_theRenderer->EndCamera(m_uiCam);
	}
}


void Level::RenderTiles() const
{
	static std::vector<Vertex_PCU> tileVerts;
	tileVerts.clear();
	for (int tileIndex = 0; tileIndex < LEVEL_TILE_COUNT; tileIndex++)
	{
		AddVertsForTileToVector(tileVerts, m_grid[tileIndex], tileIndex);
	}
	g_theRenderer->BindTexture(0, g_spriteSheetTexture);
	g_theRenderer->DrawVertexArray((int) tileVerts.size(), tileVerts.data());
}


void Level::RenderBackground() const
{
	static Texture* bgTexture = g_theRenderer->CreateOrGetTexture("Data/Images/Background1.png");
	static AABB2 levelBounds = AABB2(0.0f, 0.0f, (float) LEVEL_SIZE_X, (float) LEVEL_SIZE_Y);
	static RenderMaterial levelMaterial = {};
	levelMaterial.m_texture = bgTexture;
	g_theRenderer->DrawAABB2D(levelBounds, levelMaterial);
	g_theRenderer->BindTexture(0, nullptr);
}


void Level::RenderWaypointArrows(Rgba8 const& arrowColors) const
{
	std::vector<Vertex_PCU> arrowVerts;
	for (int i = 0; i < (int) m_spawnData.size(); i++)
	{
		EntitySpawnInfo const& entitySpawn = m_spawnData[i];
		EntitySpawnInfo targetWaypointSpawn = GetEntitySpawnInfoById(entitySpawn.m_targetWaypointId);
		if (targetWaypointSpawn.IsValid())
		{
			AddVertsForArrow2DToVector(arrowVerts, entitySpawn.m_startingPosition, targetWaypointSpawn.m_startingPosition, arrowColors, 0.1f, 0.5f, 0.0f);
		}
	}

	g_theRenderer->BindTexture(0, nullptr);
	g_theRenderer->DrawVertexArray((int) arrowVerts.size(), arrowVerts.data());
}


void Level::AddLevelWinCondition(LevelWinConditions condition)
{
	m_levelWinFlags |= condition;
}


LevelRaycastResult Level::RaycastLevel(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, bool treatSemiSolidAsSolid)
{
	LevelRaycastResult result = {};
	result.m_startPos = startPos;
	result.m_forwardNormal = forwardNormal;
	result.m_maxDistance = maxDistance;
	IntVec2 startingTileCoords = GetTileCoords(startPos); 
	
	if (IsTileCoordValid(startingTileCoords.x, startingTileCoords.y) && IsTileSolid(startPos, treatSemiSolidAsSolid))
	{
		Vec2 tileCenter = GetTileCenter(startingTileCoords);
		result.m_didHit = true;
		result.m_impactPoint = startPos;
		result.m_impactDistance = 0.0f;
		result.m_impactNormal = -forwardNormal;
		result.m_impactTileCoords = startingTileCoords;
		return result;
	}

	Vec2 raycast = forwardNormal * maxDistance;
	if (raycast.GetLengthSquared() != 0)
	{
		int startingX = startingTileCoords.x;
		int startingY = startingTileCoords.y;

		float raycastX = raycast.x;
		float tPerUnitX = 1.0f / abs(raycastX);
		int tileStepX = raycastX >= 0.0f ? 1 : -1;
		int nearestTileEdgeX = tileStepX > 0 ? startingX + 1 : startingX;
		float distanceToNearestTileEdgeX = abs(startPos.x - (float) nearestTileEdgeX);
		float tOfNextIntersectionX = distanceToNearestTileEdgeX * tPerUnitX;

		float raycastY = raycast.y;
		float tPerUnitY = 1.0f / abs(raycastY);
		int tileStepY = raycastY >= 0.0f ? 1 : -1;
		int nearestTileEdgeY = tileStepY > 0 ? startingY + 1 : startingY;
		float distanceToNearestTileEdgeY = abs(startPos.y - (float) nearestTileEdgeY);
		float tOfNextIntersectionY = distanceToNearestTileEdgeY * tPerUnitY;

		IntVec2 intersectingTileCoords = startingTileCoords;
		float tOfNextIntersection = GetMin(tOfNextIntersectionX, tOfNextIntersectionY);
		while (tOfNextIntersection < 1.0f)
		{
			if (tOfNextIntersection == tOfNextIntersectionX)
			{
				intersectingTileCoords.x += tileStepX;
				if (IsTileCoordValid(intersectingTileCoords) && IsTileSolid(intersectingTileCoords, treatSemiSolidAsSolid))
				{
					result.m_didHit = true;
					result.m_impactFraction = tOfNextIntersection;
					result.m_impactDistance = tOfNextIntersection * maxDistance;
					result.m_impactPoint = startPos + raycast * tOfNextIntersection;
					result.m_impactTileCoords = intersectingTileCoords;
					if (raycast.x > 0)
					{
						result.m_impactNormal = Vec2(-1.0f, 0.0f);
					}
					else
					{
						result.m_impactNormal = Vec2(1.0f, 0.0f);
					}
					return result;
				}

				tOfNextIntersectionX += tPerUnitX;
			}

			if (tOfNextIntersection == tOfNextIntersectionY)
			{
				intersectingTileCoords.y += tileStepY;
				if (IsTileCoordValid(intersectingTileCoords) && IsTileSolid(intersectingTileCoords, treatSemiSolidAsSolid))
				{
					result.m_didHit = true;
					result.m_impactFraction = tOfNextIntersection;
					result.m_impactDistance = tOfNextIntersection * maxDistance;
					result.m_impactPoint = startPos + raycast * tOfNextIntersection;
					result.m_impactTileCoords = intersectingTileCoords;
					if (raycast.y > 0)
					{
						result.m_impactNormal = Vec2(0.0f, -1.0f);
					}
					else
					{
						result.m_impactNormal = Vec2(0.0f, 1.0f);
					}
					return result;
				}

				tOfNextIntersectionY += tPerUnitY;
			}
			tOfNextIntersection = GetMin(tOfNextIntersectionX, tOfNextIntersectionY);
		}
	}

	return result;
}


bool Level::PushEntityOutOfTile(IntVec2 const& tileCoords, Entity*& entity)
{
	AABB2 tileBounds = GetTileBounds(tileCoords);
	AABB2 entityBounds = entity->GetWorldPhysicsBounds();
	if (!DoAABB2sOverlap(tileBounds, entityBounds))
		return false;

	float eastTranslation = tileBounds.m_maxs.x - entityBounds.m_mins.x;
	float westTranslation = tileBounds.m_mins.x - entityBounds.m_maxs.x;
	float northTranslation = tileBounds.m_maxs.y - entityBounds.m_mins.y;
	float southTranslation = tileBounds.m_mins.y - entityBounds.m_maxs.y;

	float smallestTranslation = GetAbsoluteMin(GetAbsoluteMin(eastTranslation, westTranslation), GetAbsoluteMin(northTranslation, southTranslation));

	if (smallestTranslation == eastTranslation || smallestTranslation == westTranslation)
	{
		entity->m_position.x += smallestTranslation;
		Vec2 impactNormal = smallestTranslation > 0 ? Vec2(1.0f, 0.0f) : Vec2(-1.0f, 0.0f);
		Vec2 entityVelocity = entity->m_velocity;
		entityVelocity -= GetProjectedOnto2D(entityVelocity, impactNormal);
		entity->m_velocity = entityVelocity;
	}

	if (smallestTranslation == southTranslation || smallestTranslation == northTranslation)
	{
		entity->m_position.y += smallestTranslation;
		Vec2 impactNormal = smallestTranslation > 0 ? Vec2(0.0f, 1.0f) : Vec2(0.0f, -1.0f);
		Vec2 entityVelocity = entity->m_velocity;
		entityVelocity -= GetProjectedOnto2D(entityVelocity, impactNormal);
		entity->m_velocity = entityVelocity;
	}
	
	return true;
}


bool Level::PushEntitiesOutOfEachOther(Entity*& a, Entity*& b)
{
	AABB2 aBounds = a->GetWorldPhysicsBounds();
	AABB2 bBounds = b->GetWorldPhysicsBounds();

	float westTranslation = aBounds.m_maxs.x - bBounds.m_mins.x;
	float eastTranslation = aBounds.m_mins.x - bBounds.m_maxs.x;
	float southTranslation = aBounds.m_maxs.y - bBounds.m_mins.y;
	float northTranslation = aBounds.m_mins.y - bBounds.m_maxs.y;

	float smallestTranslation = GetAbsoluteMin(GetAbsoluteMin(eastTranslation, westTranslation), GetAbsoluteMin(northTranslation, southTranslation));
	float halfTranslation = smallestTranslation * 0.5f;

	Vec2 impactNormal;
	if (smallestTranslation == eastTranslation || smallestTranslation == westTranslation)
	{
		a->m_position.x -= halfTranslation;
		b->m_position.x += halfTranslation;
		impactNormal = smallestTranslation > 0.0f ? Vec2(1.0f, 0.0f) : Vec2(-1.0f, 0.0f);
	}

	if (smallestTranslation == southTranslation || smallestTranslation == northTranslation)
	{
		a->m_position.y -= halfTranslation;
		b->m_position.y += halfTranslation;
		impactNormal = smallestTranslation > 0.0f ? Vec2(0.0f, 1.0f) : Vec2(0.0f, -1.0f);
	}

	UNUSED(impactNormal);
// 	Vec2 entityVelocityA = a->m_velocity;
// 	Vec2 entityVelocityB = b->m_velocity;
// 	entityVelocityA -= GetProjectedOnto2D(entityVelocityA, impactNormal);
// 	entityVelocityB += GetProjectedOnto2D(entityVelocityB, impactNormal);
// 	a->m_velocity = entityVelocityA;
// 	b->m_velocity = entityVelocityB;

	return true;
}


bool Level::PushEntityOutOfEntity(Entity*& mobileEntity, Entity const* staticEntity)
{
	AABB2 staticBounds = staticEntity->GetWorldPhysicsBounds();
	AABB2 entityBounds = mobileEntity->GetWorldPhysicsBounds();

	if (!DoAABB2sOverlap(staticBounds, entityBounds))
		return false;

	float eastTranslation = staticBounds.m_maxs.x - entityBounds.m_mins.x;
	float westTranslation = staticBounds.m_mins.x - entityBounds.m_maxs.x;
	float northTranslation = staticBounds.m_maxs.y - entityBounds.m_mins.y;
	float southTranslation = staticBounds.m_mins.y - entityBounds.m_maxs.y;

	float smallestTranslation = GetAbsoluteMin(GetAbsoluteMin(eastTranslation, westTranslation), GetAbsoluteMin(northTranslation, southTranslation));

	if (smallestTranslation == eastTranslation || smallestTranslation == westTranslation)
	{
		mobileEntity->m_position.x += smallestTranslation;
		Vec2 impactNormal = smallestTranslation > 0 ? Vec2(1.0f, 0.0f) : Vec2(-1.0f, 0.0f);
		Vec2 entityVelocity = mobileEntity->m_velocity;
		Vec2 velocityAlongNormal = GetProjectedOnto2D(entityVelocity, impactNormal);
		entityVelocity -= GetProjectedOnto2D(entityVelocity, impactNormal);
		mobileEntity->m_velocity = entityVelocity;
	}

	if (smallestTranslation == southTranslation || smallestTranslation == northTranslation)
	{
		mobileEntity->m_position.y += smallestTranslation;
		Vec2 impactNormal = smallestTranslation > 0 ? Vec2(0.0f, 1.0f) : Vec2(0.0f, -1.0f);
		Vec2 entityVelocity = mobileEntity->m_velocity;
		entityVelocity -= GetProjectedOnto2D(entityVelocity, impactNormal);
		mobileEntity->m_velocity = entityVelocity;
	}

	return true;
}


void Level::AddEntitySpawn(EntitySpawnInfo const& info)
{
	EntitySpawnInfo spawnInfoToAdd = info;
	AABB2 spawnBoundsToAdd = EntityDefinition::GetPhysicsBoundsForEntity(spawnInfoToAdd.m_type, spawnInfoToAdd.m_startingPosition);
	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_spawnData.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo const& spawnInfoToCheck = m_spawnData[spawnInfoIndex];
		AABB2 spawnBoundsToCheck = EntityDefinition::GetPhysicsBoundsForEntity(spawnInfoToCheck.m_type, spawnInfoToCheck.m_startingPosition);
		if (DoAABB2sOverlap(spawnBoundsToAdd, spawnBoundsToCheck))
		{
			RemoveEntitySpawn(spawnInfoToCheck);
			spawnInfoIndex--;
		}
	}
	
	if (spawnInfoToAdd.m_entityId.IsInvalid())
	{
		spawnInfoToAdd.m_entityId = EntityID::GetIDForIndex((uint32_t) m_spawnData.size());
	}
	m_spawnData.push_back(spawnInfoToAdd);
}


void Level::RemoveEntitySpawn(EntitySpawnInfo const& info)
{
	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_spawnData.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo const& spawnInfo = m_spawnData[spawnInfoIndex];
		if (spawnInfo == info)
		{
			m_spawnData[spawnInfoIndex] = m_spawnData.back();
			m_spawnData.pop_back();
			return;
		}
	}
}


Entity* Level::SpawnEntity(EntitySpawnInfo const& spawnInfo)
{
	if ((spawnInfo.m_type == ENTITY_TYPE_SMALL_MARIO || spawnInfo.m_type == ENTITY_TYPE_MARIO) && m_entitiesByType[ENTITY_TYPE_MARIO].size() > 0)
	{
		return nullptr;
	}

	Entity* spawnedEntity = new Entity(this, spawnInfo);
	return AddEntityToLevel(spawnedEntity);
}


Entity* Level::AddEntityToLevel(Entity* entity)
{
	Entity* addedEntity = nullptr;
	if (entity)
	{
		EntityList& entityTypeList = m_entitiesByType[entity->m_type];
		addedEntity = AddEntityToList(entityTypeList, entity);
		if (entity->m_entityId.IsInvalid())
		{
			addedEntity = AddEntityToList(m_allEntities, addedEntity, true);
		}
		else
		{
			addedEntity = AddEntityToList(m_allEntities, addedEntity);
		}
	}

	return addedEntity;
}


Entity* Level::RemoveEntityFromLevel(Entity* entity)
{
	Entity* removedEntity = nullptr;
	if (entity)
	{
		EntityType type = entity->m_type;
		if (type == ENTITY_TYPE_SMALL_MARIO)
		{
			type = ENTITY_TYPE_MARIO;
		}
		EntityList& entityTypeList = m_entitiesByType[type];
		removedEntity =	RemoveEntityFromList(entityTypeList, entity);
		removedEntity = RemoveEntityFromList(m_allEntities, removedEntity);
	}

	return removedEntity;
}


Entity* Level::AddEntityToList(EntityList& list, Entity* entity, bool assignId /*= false*/)
{
	if (entity)
	{
		for (int i = 0; i < list.size(); i++)
		{
			if (list[i] == nullptr)
			{
				list[i] = entity;
				if (assignId)
				{
					list[i]->m_entityId = EntityID::GetIDForIndex(i);
				}
				return list[i];
			}
		}

		list.push_back(entity);
		if (assignId)
		{
			entity->m_entityId = EntityID::GetIDForIndex((uint32_t) list.size() - 1);
		}
		return list.back();
	}

	return nullptr;
}


Entity* Level::RemoveEntityFromList(EntityList& list, Entity* entity)
{
	if (entity)
	{
		for (int i = 0; i < list.size(); i++)
		{
			if (list[i] && list[i] == entity)
			{
				list[i] = list.back();
				list.pop_back();
				return entity;
			}
		}
	}
	return nullptr;
}


void Level::LoadGridFromXmlElement(XmlElement* element)
{
	XmlElement* tileElement = element->FirstChildElement("Tile");
	while (tileElement)
	{
		int tileIndex = ParseXmlAttribute(*tileElement, "index", 0);
		std::string tileDefName = ParseXmlAttribute(*tileElement, "definition", "");
		TileDefinition const* tileDef = TileDefinition::GetDefinitionByName(tileDefName);
		m_grid[tileIndex].m_definition = tileDef;

		tileElement = tileElement->NextSiblingElement("Tile");
	}
}


void Level::LoadEntitySpawnDataFromXmlElement(XmlElement* element)
{
	XmlElement* spawnElement = element->FirstChildElement("EntitySpawn");
	while (spawnElement)
	{
		int type = ParseXmlAttribute(*spawnElement, "type", -1);
		Vec2 position = ParseXmlAttribute(*spawnElement, "pos", Vec2::ZERO);
		uint32_t id = ParseXmlAttribute(*spawnElement, "id", EntityID::s_INVALID_ID.m_id);
		uint32_t targetWaypointId = ParseXmlAttribute(*spawnElement, "targetWaypoint", EntityID::s_INVALID_ID.m_id);

		EntitySpawnInfo spawnInfo = EntitySpawnInfo((EntityType) type);
		spawnInfo.m_entityId.m_id = id;
		spawnInfo.m_startingPosition = position;
		spawnInfo.m_targetWaypointId.m_id = targetWaypointId;
		AddEntitySpawn(spawnInfo);

		spawnElement = spawnElement->NextSiblingElement("EntitySpawn");
	}
}


bool Level::CheckLevelEndCondition()
{
	if (m_gameOverStartTime < 0.0f && !m_isGameCompletelyOver)
	{
		bool shouldCollectAllCoins = (m_levelWinFlags & LEVEL_WIN_COLLECT_ALL_COINS) == LEVEL_WIN_COLLECT_ALL_COINS;
		if (shouldCollectAllCoins && (int) m_entitiesByType[ENTITY_TYPE_COIN].size() > 0)
			return false;

		bool shouldDefeatAllEnemies = (m_levelWinFlags & LEVEL_WIN_DEFEAT_ALL_ENEMIES) == LEVEL_WIN_DEFEAT_ALL_ENEMIES;
		if (shouldDefeatAllEnemies && (int) m_entitiesByType[ENTITY_TYPE_GOOMBA].size() > 0)
			return false;

		TriggerGameOver(GAME_OVER_STATE_WON);
		return true;
	}

	return false;
}


void Level::TriggerGameOver(GameOverState gameOverState)
{
	m_gameOverState = gameOverState;
	m_gameOverStartTime = (float) GetCurrentTimeSeconds();
	g_audioSystem->StopSound(m_bgMusicPlaybackId);
	if (m_gameOverState == GAME_OVER_STATE_LOSS)
	{
		SoundID lossMusicSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/GameLossMusic.mp3");
		//float volume = g_muteSfx ? 0.0f : 1.0f;
		m_bgMusicPlaybackId = g_audioSystem->StartSound(lossMusicSoundId, false, g_gameMusicVolume);
		m_gameOverDuration = g_gameConfigBlackboard.GetValue("gameOverDuration_Loss", 1.0f);
	}
	else if (m_gameOverState == GAME_OVER_STATE_WON)
	{
		SoundID wonMusicSoundId = g_audioSystem->CreateOrGetSound("Data/Audio/GameWonMusic.mp3");
		//float volume = g_muteSfx ? 0.0f : 1.0f;
		m_bgMusicPlaybackId = g_audioSystem->StartSound(wonMusicSoundId, false, g_gameMusicVolume);
		m_gameOverDuration = g_gameConfigBlackboard.GetValue("gameOverDuration_Victory", 1.0f);
	}
}


bool Level::DoEntitiesTouch(Entity* a, Entity* b) const
{
	AABB2 physicsBoundsA = a->GetWorldPhysicsBounds();
	AABB2 physicsBoundsB = b->GetWorldPhysicsBounds();
	AABB2CollisionData collisionData = GetCollisionDataForAABB2s(physicsBoundsA, physicsBoundsB);
	return collisionData.m_isTouching;
}


bool Level::WereEntitiesOverlappingPrevFrame(uint64_t overlappingEntityId) const
{
	for (int i = 0; i < m_prevFrameOverlappingEntityIds.size(); i++)
	{
		if (m_prevFrameOverlappingEntityIds[i] == overlappingEntityId)
			return true;
	}

	return false;
}


Entity* Level::GetEntityById(uint32_t id) const
{
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		Entity* e = m_allEntities[i];
		if (e && e->m_entityId.m_id == id)
		{
			return e;
		}
	}

	return nullptr;
}


void Level::HandleOverlapBetweenEntities(Entity* entityA, Entity* entityB)
{
	AABB2 refBounds = entityA->GetWorldPhysicsBounds();
	AABB2 boundsToCheck = entityB->GetWorldPhysicsBounds();
	AABB2CollisionData collisionData = GetCollisionDataForAABB2s(refBounds, boundsToCheck);

// 	if (entityA->HasOptions(ENTITY_OPTIONS_GENERATES_HIT_EVENTS))
// 	{
// 		entityA->OnHit(collisionData, entityB);
// 	}

	if (entityA->HasOptions(ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT))
	{
		entityA->OnOverlap(collisionData, entityB);
	}

// 	if (entityB->HasOptions(ENTITY_OPTIONS_GENERATES_HIT_EVENTS))
// 	{
// 		entityB->OnHit(collisionData.GetCollisionDataWithInvertedReference(), entityA);
// 	}

	if (entityB->HasOptions(ENTITY_OPTIONS_GENERATES_OVERLAP_EVENT))
	{
		entityB->OnOverlap(collisionData.GetCollisionDataWithInvertedReference(), entityA);
	}
}


bool Level::HandleTileEntityCollision(IntVec2 const& tileCoords, Entity*& e)
{
	Tile tile = GetTile(tileCoords);
	if (tile.IsEmpty())
		return false;

	AABB2 tileBounds = GetTileBounds(tileCoords);
	AABB2 entityBounds = e->GetWorldPhysicsBounds();
	if (!AreAABB2sTouching(tileBounds, entityBounds))
		return false;

	if (tile.DoesDamage())
	{
		if (e->m_type == ENTITY_TYPE_COIN || e->m_type == ENTITY_TYPE_RED_BLOCK || e->m_type == ENTITY_TYPE_RED_MUSHROOM || e->m_type == ENTITY_TYPE_BLUE_BLOCK || e->m_type == ENTITY_TYPE_SWITCH_BLOCK
			|| e->m_type == ENTITY_TYPE_BREAKABLE_BLOCK || e->m_type == ENTITY_TYPE_FLIPPABLE_BLOCK)
		{
			//do nothing
		}
		else
		{
			int damage = tile.GetDamage();
			e->TakeDamage(damage);
		}
	}

	if (tile.IsSemiSolid())
	{
		if (e->m_type == ENTITY_TYPE_FIREBALL)
			return false;

		Vec2 entityVelocity = e->GetVelocity();
		if (entityVelocity.y < 0.0f)
		{
			PushEntityOutOfTile(tileCoords, e);
		}

		if (e->GetPosition().y >= tileBounds.m_maxs.y - 0.2f)
		{
			e->SetGrounded(true);
			EventArgs args;
			args.AddProperty("tileIndex", tile.m_index);
			g_eventSystem->FireEvent("tile:OnStep", args);
		}
	}

	if (tile.IsSolid())
	{
		if (e->m_type == ENTITY_TYPE_FIREBALL)
		{
			e->Die();
			return true;
		}

		PushEntityOutOfTile(tileCoords, e);
		if (e->m_position.y >= tileBounds.m_maxs.y - 0.2f)
		{
			e->SetGrounded(true);
		}
	}

	return true;
}


void Level::SetTileDefinitionAtCoords(TileDefinition const* tileDefinition, int tileX, int tileY)
{
	if (tileDefinition == nullptr)
	{
		ERROR_RECOVERABLE(Stringf("Invalid Tile Definition provided for tile at coords %i, %i", tileX, tileY));
	}

	if (!IsTileCoordValid(tileX, tileY))
		return;

	int tileIndex = GetTileIndex(tileX, tileY);
	if (tileIndex >= 0 && tileIndex < LEVEL_TILE_COUNT)
	{
		m_grid[tileIndex].m_definition = tileDefinition;
		if (m_grid[tileIndex].IsSolid())
		{
			//g_console->AddLine(DevConsole::MINOR_INFO, "Is Solid");
		}
	}

}


void Level::SetTileDefinitionAtCoords(TileDefinition const* tileDefinition, IntVec2 const& tileCoords)
{
	SetTileDefinitionAtCoords(tileDefinition, tileCoords.x, tileCoords.y);
}


void Level::SetTileDefinitionAtIndex(TileDefinition const* tileDefinition, int tileIndex)
{
	if (tileDefinition == nullptr)
	{
		ERROR_RECOVERABLE(Stringf("Invalid Tile Definition provided for tile at index %i", tileIndex));
	}

	if (tileIndex >= 0 && tileIndex < LEVEL_TILE_COUNT)
	{
		m_grid[tileIndex].m_definition = tileDefinition;
		if (m_grid[tileIndex].IsSolid())
		{
			//g_console->AddLine(DevConsole::MINOR_INFO, "Is Solid");
		}
	}
}


void Level::SetTileEmpty(IntVec2 const& tileCoords)
{
	if (!IsTileCoordValid(tileCoords))
		return;

	static TileDefinition* emptyDefinition = TileDefinition::GetDefinitionByName("Empty");
	int index = GetTileIndex(tileCoords);
	m_grid[index].m_definition = emptyDefinition;
}


void Level::ClearAllTileData()
{
	static TileDefinition* definition = TileDefinition::GetDefinitionByName("Empty");
	for (int tileIndex = 0; tileIndex < LEVEL_TILE_COUNT; tileIndex++)
	{
		m_grid[tileIndex].m_definition = definition;
	}
}


void Level::TagTile(IntVec2 const& tileCoords, Rgba8 const& color)
{
	if (!IsTileCoordValid(tileCoords))
		return;

	int index = GetTileIndex(tileCoords);
	m_grid[index].TagTile();
	m_grid[index].m_taggedColor = color;
}


void Level::ClearAllSpawnData()
{
	m_spawnData.clear();
}


void Level::MarkTileAsSelected(IntVec2 const& tileCoords)
{
	if (!IsTileCoordValid(tileCoords))
		return;

	int index = GetTileIndex(tileCoords);
	m_grid[index].SelectTile();
}


void Level::MarkTileAsDeselected(IntVec2 const& tileCoords)
{
	if (!IsTileCoordValid(tileCoords))
		return;

	int index = GetTileIndex(tileCoords);
	m_grid[index].DeselectTile();
}


void Level::MarkEntitySpawnAsSelected(EntitySpawnInfo const& entitySpawn)
{
	if (entitySpawn.IsInvalid())
		return;

	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_spawnData.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo& info = m_spawnData[spawnInfoIndex];
		if (info == entitySpawn)
		{
			info.m_isSelected = true;
			return;
		}
	}
}


void Level::MarkEntitySpawnAsDeselected(EntitySpawnInfo const& entitySpawn)
{
	if (entitySpawn.IsInvalid())
		return;

	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_spawnData.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo& info = m_spawnData[spawnInfoIndex];
		if (info == entitySpawn)
		{
			info.m_isSelected = false;
			return;
		}
	}
}


void Level::MarkEverythingAsDeselected()
{
	for (int tileIndex = 0; tileIndex < LEVEL_TILE_COUNT; tileIndex++)
	{
		IntVec2 tileCoord = GetTileCoords(tileIndex);
		MarkTileAsDeselected(tileCoord);
	}

	for (int i = 0; i < (int) m_spawnData.size(); i++)
	{
		MarkEntitySpawnAsDeselected(m_spawnData[i]);
	}
}


void Level::AddOverlappingSpawnInfosToVector(std::vector<EntitySpawnInfo>& spawnInfoVector, EntitySpawnInfo const& refSpawnInfo) const
{
	for (int i = 0; i < (int) m_spawnData.size(); i++)
	{
		EntitySpawnInfo const& info = m_spawnData[i];
		if (DoSpawnInfosIntersect(info, refSpawnInfo))
		{
			spawnInfoVector.push_back(info);
		}
	}
}


bool Level::DoesTileIntersectSpawnInfo(IntVec2 const& tileCoords, EntitySpawnInfo const& info) const
{
	AABB2 worldCosmeticBounds = EntityDefinition::GetPhysicsBoundsForEntity(info.m_type, info.m_startingPosition);
	AABB2 tileBounds = GetTileBounds(tileCoords);
	bool doesIntersect = DoAABB2sOverlap(tileBounds, worldCosmeticBounds);
	return doesIntersect;
}


bool Level::DoesTileIntersectAnySpawnInfo(IntVec2 const& tileCoords) const
{
	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_spawnData.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo const& spawnInfo = m_spawnData[spawnInfoIndex];
		if (DoesTileIntersectSpawnInfo(tileCoords, spawnInfo))
		{
			return true;
		}
	}

	return false;
}


bool Level::DoesSpawnInfoIntersectSolidTiles(EntitySpawnInfo const& spawnInfo) const
{
	AABB2 cosmeticBounds = EntityDefinition::GetPhysicsBoundsForEntity(spawnInfo.m_type, spawnInfo.m_startingPosition);
	IntVec2 bottomLeftTileCoords = GetTileCoords(cosmeticBounds.m_mins);
	IntVec2 topRightTileCoords = GetTileCoords(cosmeticBounds.m_maxs);
	topRightTileCoords -= IntVec2(1, 1);
	for (int x = bottomLeftTileCoords.x; x <= topRightTileCoords.x; x++)
	{
		for (int y = bottomLeftTileCoords.y; y <= topRightTileCoords.y; y++)
		{
			IntVec2 tileCoords = IntVec2(x, y);
			if (IsTileCoordValid(x, y) && IsTileSolid(tileCoords))
			{
				return true;
			}
		}
	}
	return false;
}


bool Level::DoesSpawnInfoExist(EntitySpawnInfo const& spawnInfo) const
{
	if (spawnInfo.IsInvalid())
		return false;

	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_spawnData.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo const& info = m_spawnData[spawnInfoIndex];
		if (info == spawnInfo)
			return true;
	}

	return false;
}


bool Level::DoesSpawnInfoIntersectAnySpawnInfo(EntitySpawnInfo const& spawnInfo) const
{
	for (int i = 0; i < (int) m_spawnData.size(); i++)
	{
		EntitySpawnInfo const& info = m_spawnData[i];
		if (DoSpawnInfosIntersect(spawnInfo, info))
		{
			return true;
		}
	}

	return false;
}


bool Level::DoSpawnInfosIntersect(EntitySpawnInfo const& a, EntitySpawnInfo const& b) const
{
	AABB2 boundsA = EntityDefinition::GetPhysicsBoundsForEntity(a.m_type, a.m_startingPosition);
	AABB2 boundsB = EntityDefinition::GetPhysicsBoundsForEntity(b.m_type, b.m_startingPosition);
	return DoAABB2sOverlap(boundsA, boundsB);
}


bool Level::DoesEntityOverlapAnyOtherEntity(Entity* refEntity) const
{
	AABB2 refBounds = refEntity->GetWorldPhysicsBounds();
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		Entity* e = m_allEntities[i];
		if (e != refEntity)
		{
			AABB2 entityBounds = e->GetWorldPhysicsBounds();
			if (DoAABB2sOverlap(entityBounds, refBounds))
			{
				return true;
			}
		}
	}

	return false;
}


EntitySpawnInfo Level::GetEntitySpawnInfo(IntVec2 const& tileCoords) const
{
	if (!IsTileCoordValid(tileCoords))
		return EntitySpawnInfo::s_INVALID_SPAWN_INFO;

	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_spawnData.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo const& spawnInfo = m_spawnData[spawnInfoIndex];
		if (DoesTileIntersectSpawnInfo(tileCoords, spawnInfo))
		{
			return spawnInfo;
		}
	}

	return EntitySpawnInfo::s_INVALID_SPAWN_INFO;
}


EntitySpawnInfo Level::GetEntitySpawnInfo(Vec2 const& refPos) const
{
	IntVec2 refTileCoord = GetTileCoords(refPos);
	if (!IsTileCoordValid(refTileCoord))
		return EntitySpawnInfo::s_INVALID_SPAWN_INFO;

	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_spawnData.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo const& spawnInfo = m_spawnData[spawnInfoIndex];
		AABB2 entityPhysicsBounds = EntityDefinition::GetPhysicsBoundsForEntity(spawnInfo.m_type, spawnInfo.m_startingPosition);
		if (IsPointInsideAABB2D(refPos, entityPhysicsBounds))
		{
			return spawnInfo;
		}
	}

	return EntitySpawnInfo::s_INVALID_SPAWN_INFO;
}


EntitySpawnInfo const& Level::GetEntitySpawnInfoById(EntityID const& refId) const
{
	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_spawnData.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo const& spawnInfo = m_spawnData[spawnInfoIndex];
		if (spawnInfo.m_entityId == refId)
			return spawnInfo;
	}

	return EntitySpawnInfo::s_INVALID_SPAWN_INFO;
}


Entity* Level::GetMario() const
{
	return m_entitiesByType[ENTITY_TYPE_MARIO][0];
}


Entity* Level::GetEntity(IntVec2 const& tileCoords) const
{
	if (!IsTileCoordValid(tileCoords))
		return nullptr;

	for (int entityIndex = 0; entityIndex < (int) m_allEntities.size(); entityIndex++)
	{
		Entity* entity = m_allEntities[entityIndex];
		AABB2 worldPhysicsBounds = entity->GetWorldPhysicsBounds();
		AABB2 tileBounds = GetTileBounds(tileCoords);
		if (DoAABB2sOverlap(tileBounds, worldPhysicsBounds))
		{
			return entity;
		}
	}

	return nullptr;
}


void Level::AddNonEmptyEastTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds) const
{
	IntVec2 topRight = GetTileCoords(bounds.m_maxs);
	IntVec2 bottomRight = GetTileCoords(Vec2(bounds.m_maxs.x, bounds.m_mins.y));
	int x = bottomRight.x;
	int y = bottomRight.y;
	for (y ; y <= topRight.y - 1; y++)
	{
		if (IsTileCoordValid(x, y) && !IsTileEmpty(x, y))
		{
			vector.emplace_back(x, y);
		}
	}
}


void Level::AddNonEmptyWestTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds) const
{
	IntVec2 topleft = GetTileCoords(Vec2(bounds.m_mins.x, bounds.m_maxs.y));
	IntVec2 bottomLeft = GetTileCoords(bounds.m_mins);
	int x = bottomLeft.x - 1;
	int y = bottomLeft.y;
	for (y; y <= topleft.y - 1; y++)
	{
		if (IsTileCoordValid(x, y) && !IsTileEmpty(x, y))
		{
			vector.emplace_back(x, y);
		}
	}
}


void Level::AddNonEmptyNorthTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds) const
{
	IntVec2 topRight = GetTileCoords(bounds.m_maxs);
	IntVec2 topLeft = GetTileCoords(Vec2(bounds.m_mins.x, bounds.m_maxs.y));
	int x = topLeft.x - 1;
	int y = topLeft.y;
	for (x; x <= topRight.x; x++)
	{
		if (IsTileCoordValid(x, y) && !IsTileEmpty(x, y))
		{
			vector.emplace_back(x, y);
		}
	}
}


void Level::AddNonEmptySouthTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds) const
{
	IntVec2 bottomLeft = GetTileCoords(bounds.m_mins);
	IntVec2 bottomRight = GetTileCoords(Vec2(bounds.m_maxs.x, bounds.m_mins.y));
	int x = bottomLeft.x - 1;
	int y = bottomLeft.y - 1;
	for (x; x <= bottomRight.x; x++)
	{
		if (IsTileCoordValid(x, y) && !IsTileEmpty(x, y))
		{
			vector.emplace_back(x, y);
		}
	}
}


void Level::AddNonEmptyTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds) const
{
	IntVec2 bottomLeft = GetTileCoords(bounds.m_mins);
	IntVec2 topRight = GetTileCoords(bounds.m_maxs);
	for (int x = bottomLeft.x; x < topRight.x; x++)
	{
		for (int y = bottomLeft.y; y < topRight.y; y++)
		{
			if (IsTileCoordValid(x, y) && !IsTileEmpty(x, y))
			{
				vector.emplace_back(x, y);
			}
		}
	}
}


void Level::AddTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds) const
{
	IntVec2 bottomLeft = GetTileCoords(bounds.m_mins);
	IntVec2 topRight = GetTileCoords(bounds.m_maxs);
	for (int x = bottomLeft.x; x < topRight.x; x++)
	{
		for (int y = bottomLeft.y; y < topRight.y; y++)
		{
			if (IsTileCoordValid(x, y))
			{
				vector.emplace_back(x, y);
			}
		}
	}
}


AABB2 Level::GetUVForTile(Tile const& tile, int tileIndex) const
{
	//return sprite UVs (stored as AABB2s) for a given tile based on the type of the tile-set
 	Tileset const& tileset = tile.m_definition->m_tileSet;
	if (tileset.m_type == TILE_SET_SINGLE_SPRITE)
		return *tileset.m_spriteUVs;
	
	//Use 4 bit bitmasking for 16-Tile tile set (does not contain corner tiles)
	if (tileset.m_type == TILE_SET_16_SPRITES)
	{
		//final bitmask value for sprite
		int spriteBitmask = 0;
		//bit mask values for cardinal directions
		static int northBitMask = 1;
		static int westBitMask = 2;
		static int eastBitMask = 4;
		static int southBitMask = 8;

		//calculate bitmask value
		IntVec2 tilecoords = GetTileCoords(tileIndex);
		if (tilecoords.y != 0)
		{
			Tile southTile = GetTile(IntVec2(tilecoords.x, tilecoords.y - 1));
			bool isSouthSame = southTile.m_definition && southTile == tile;
			spriteBitmask += southBitMask * isSouthSame;
		}

		if (tilecoords.y != LEVEL_SIZE_Y - 1)
		{
			Tile northTile = GetTile(IntVec2(tilecoords.x, tilecoords.y + 1));
			bool isNorthSame = northTile.m_definition && northTile == tile;
			spriteBitmask += northBitMask * isNorthSame;
		}

		if (tilecoords.x != 0)
		{
			Tile eastTile = GetTile(IntVec2(tilecoords.x + 1, tilecoords.y));
			bool isEastSame = eastTile.m_definition && eastTile == tile;
			spriteBitmask += eastBitMask * isEastSame;
		}

		if (tilecoords.x != LEVEL_SIZE_X - 1)
		{
			Tile westTile = GetTile(IntVec2(tilecoords.x - 1, tilecoords.y));
			bool isWestSame = westTile.m_definition && westTile == tile;
			spriteBitmask += westBitMask * isWestSame;
		}

		//bitmask corresponds to UV array index
		return tileset.m_spriteUVs[spriteBitmask];
	}

	//Use 8 bit bitmasking for 48-Tile tile set (contains corner tiles)
	if (tileset.m_type == TILE_SET_48_SPRITES)
	{
		//lookup table for bitmask values corresponding to sprite index
		static std::map<int, int> s_spriteLookupTable {{2, 1},	{8, 2},	{10, 3}, {11, 4}, {16, 5}, {18, 6},	{22, 7}, {24, 8}, {26, 9}, {27, 10}, {30, 11}, {31, 12}, {64, 13},
														{66, 14}, {72, 15},	{74, 16}, {75, 17},	{80, 18}, {82, 19},	{86, 20}, {88, 21},	{90, 22}, {91, 23},	{94, 24}, {95, 25},
														{104, 26}, {106, 27}, {107, 28}, {120, 29}, {122, 30}, {123, 31}, {126, 32}, {127, 33},	{208, 34}, {210, 35}, {214, 36},
														{216, 37}, {218, 38}, {219, 39}, {222, 40},	{223, 41}, {248, 42}, {250, 43}, {251, 44},	{254, 45}, {255, 46}, {0, 47 }
		};

		//bitmask values for all 8 directions (N, S, E, W, NE, NW, SE, SW)
		static int northWestBitMask		= 1;
		static int northBitMask			= 2;
		static int northEastBitMask		= 4;
		static int westBitMask			= 8;
		static int eastBitMask			= 16;
		static int southWestBitMask		= 32;
		static int southBitMask			= 64;
		static int southEastBitMask		= 128;
		
		//final bitmask value for sprite
		int spriteBitmask = 0;

		//tile coordinates to check
		IntVec2 tilecoords = GetTileCoords(tileIndex);
		IntVec2 tileCoordsNW		= tilecoords + IntVec2(-1, 1);
		IntVec2 tileCoordsN			= tilecoords + IntVec2(0, 1);
		IntVec2 tileCoordsNE		= tilecoords + IntVec2(1, 1);
		IntVec2 tileCoordsW			= tilecoords + IntVec2(-1, 0);
		IntVec2 tileCoordsE			= tilecoords + IntVec2(1, 0);
		IntVec2 tileCoordsSW		= tilecoords + IntVec2(-1, -1);
		IntVec2 tileCoordsS			= tilecoords + IntVec2(0, -1);
		IntVec2 tileCoordsSE		= tilecoords + IntVec2(1, -1);
		
		//flags to see if tile coordinates are valid
		bool isNorthWestTileValid	= IsTileCoordValid(tileCoordsNW);
		bool isNorthTileValid		= IsTileCoordValid(tileCoordsN);
		bool isNorthEastTileValid	= IsTileCoordValid(tileCoordsNE);
		bool isWestTileValid		= IsTileCoordValid(tileCoordsW);
		bool isEastTileValid		= IsTileCoordValid(tileCoordsE);
		bool isSouthWestTileValid	= IsTileCoordValid(tileCoordsSW);
		bool isSouthTileValid		= IsTileCoordValid(tileCoordsS);
		bool isSouthEastTileValid	= IsTileCoordValid(tileCoordsSE);
		
		//flag to see if the tile in the cardinal directions is the same tile 
		//as the one passed to the function
		bool isSouthSame = false;
		bool isNorthSame = false;
		bool isEastSame = false;
		bool isWestSame = false;

		//calculate bitmask value
		if (isSouthTileValid)
		{
			Tile southTile = GetTile(tileCoordsS);
			isSouthSame = southTile.m_definition && southTile == tile;
			spriteBitmask += southBitMask * isSouthSame;
		}

		if (isNorthTileValid)
		{
			Tile northTile = GetTile(tileCoordsN);
			isNorthSame = northTile.m_definition && northTile == tile;
			spriteBitmask += northBitMask * isNorthSame;
		}

		if (isEastTileValid)
		{
			Tile eastTile = GetTile(tileCoordsE);
			isEastSame = eastTile.m_definition && eastTile == tile;
			spriteBitmask += eastBitMask * isEastSame;
		}

		if (isWestTileValid)
		{
			Tile westTile = GetTile(tileCoordsW);
			isWestSame = westTile.m_definition && westTile == tile;
			spriteBitmask += westBitMask * isWestSame;
		}

		if (isNorthWestTileValid && isNorthTileValid && isWestTileValid)
		{
			Tile northWestTile = GetTile(tileCoordsNW);
			bool isNorthWestSame = northWestTile.m_definition && northWestTile == tile;
			spriteBitmask += northWestBitMask * isNorthWestSame * isNorthSame * isWestSame;
		}

		if (isSouthWestTileValid && isSouthTileValid && isWestTileValid)
		{
			Tile southWestTile = GetTile(tileCoordsSW);
			bool isSouthWestSame = southWestTile.m_definition && southWestTile == tile;
			spriteBitmask += southWestBitMask * isSouthWestSame * isSouthSame * isWestSame;
		}

		if (isNorthEastTileValid && isNorthTileValid && isEastTileValid)
		{
			Tile northEastTile = GetTile(tileCoordsNE);
			bool isNorthEastSame = northEastTile.m_definition && northEastTile == tile;
			spriteBitmask += northEastBitMask * isNorthEastSame * isNorthSame * isEastSame;
		}

		if (isSouthEastTileValid && isSouthTileValid && isEastTileValid)
		{
			Tile southEastTile = GetTile(tileCoordsSE);
			bool isSouthEastSame = southEastTile.m_definition && southEastTile == tile;
			spriteBitmask += southEastBitMask * isSouthEastSame * isSouthSame * isEastSame;
		}
		
		//bitmask corresponds to a value in the lookup table for UV array index
		int spriteIndex = s_spriteLookupTable[spriteBitmask];
		return tileset.m_spriteUVs[spriteIndex];
	}

	//return empty uvs for invalid tile set
	return AABB2();
}


SpawnData const& Level::GetLevelSpawnData() const
{
	return m_spawnData;
}


IntVec2 Level::GetTileCoords(int tileIndex) const
{
	int x = tileIndex & LEVEL_INDEX_X_MASK;
	int y = (tileIndex & LEVEL_INDEX_Y_MASK) >> LEVEL_X_BITS;
	return IntVec2(x, y);
}


IntVec2 Level::GetTileCoords(Vec2 const& worldPosition) const
{
	int x = RoundDownToInt(worldPosition.x);
	int y = RoundDownToInt(worldPosition.y);
	return IntVec2(x, y);
}


int Level::GetTileIndex(int tileX, int tileY) const
{
	return tileY << LEVEL_X_BITS | tileX;
}


int Level::GetTileIndex(IntVec2 const& tileCoords) const
{
	return GetTileIndex(tileCoords.x, tileCoords.y);
}


Vec2 Level::GetTileCenter(int tileIndex) const
{
	IntVec2 gridCoords = GetTileCoords(tileIndex);
	return gridCoords.GetVec2() + Vec2(0.5f, 0.5f);
}


Vec2 Level::GetTileCenter(IntVec2 const& tileCoords) const
{
	return tileCoords.GetVec2() + Vec2(0.5f, 0.5f);
}


Vec2 Level::GetTileBottomCenter(IntVec2 const& tileCoords) const
{
	return GetTileCenter(tileCoords) - Vec2(0.0f, 0.5f);
}


Vec2 Level::GetTileTopCenter(IntVec2 const& tileCoords) const
{
	return GetTileCenter(tileCoords) + Vec2(0.0f, 0.5f);
}


bool Level::IsTileSolid(Vec2 const& worldPosition, bool treatSemiSolidAsSolid) const
{
	IntVec2 tileCoords = GetTileCoords(worldPosition);
	if (!IsTileCoordValid(tileCoords))
	{
		return false;
	}
	
	int tileIndex = GetTileIndex(tileCoords.x, tileCoords.y);
	return m_grid[tileIndex].IsSolid(treatSemiSolidAsSolid);
}


bool Level::IsTileSolid(IntVec2 const& tileCoords, bool treatSemiSolidAsSolid) const
{
	if (!IsTileCoordValid(tileCoords))
	{
		return false;
	}

	int tileIndex = GetTileIndex(tileCoords.x, tileCoords.y);
	return m_grid[tileIndex].IsSolid(treatSemiSolidAsSolid);
}


bool Level::IsTileEmpty(IntVec2 const& tileCoords) const
{
	if (!IsTileCoordValid(tileCoords))
	{
		return false;
	}

	int tileIndex = GetTileIndex(tileCoords.x, tileCoords.y);
	return m_grid[tileIndex].IsEmpty();
}


bool Level::IsTileEmpty(int x, int y) const
{
	return IsTileEmpty(IntVec2(x, y));
}


bool Level::DoesTileDoDamage(IntVec2 const& tileCoords) const
{
	if (!IsTileCoordValid(tileCoords))
	{
		return false;
	}

	int tileIndex = GetTileIndex(tileCoords);
	return m_grid[tileIndex].DoesDamage();
}


bool Level::CanStepOnTile(IntVec2 const& tileCoords) const
{
	if (!IsTileCoordValid(tileCoords))
		return false;

	Entity* overlappingEntity = GetEntity(tileCoords);
	Tile tile = GetTile(tileCoords);
	bool canStepOn = false;
	canStepOn |= overlappingEntity != nullptr && overlappingEntity->HasOptions(ENTITY_OPTIONS_CAN_BE_STEPPED_ON);
	canStepOn |= tile.IsSolid(true);
	return canStepOn;
}


bool Level::IsTileCoordValid(int x, int y) const
{
	return x >= 0 && x < LEVEL_SIZE_X && y >= 0 && y < LEVEL_SIZE_Y;
}


bool Level::IsTileCoordValid(IntVec2 const& tileCoords) const
{
	return IsTileCoordValid(tileCoords.x, tileCoords.y);
}


Tile& Level::GetTileReference(IntVec2 const& tileCoords)
{
	if (!IsTileCoordValid(tileCoords))
	{
		ERROR_RECOVERABLE(Stringf("Trying to access tile with invalid tile coordinates: %i, %i", tileCoords.x, tileCoords.y));
		//return Tile();
	}

	int tileIndex = GetTileIndex(tileCoords);
	return m_grid[tileIndex];
}


void Level::UpdateSpawnInfo_TargetWaypoint(EntityID const& spawnInfoToUpdateId, EntityID const& newTargetWaypoint)
{
	for (int i = 0; i < (int) m_spawnData.size(); i++)
	{
		if (m_spawnData[i].m_entityId == spawnInfoToUpdateId)
		{
			m_spawnData[i].m_targetWaypointId = newTargetWaypoint;
			return;
		}
	}
}


AABB2 Level::GetTileBounds(IntVec2 const& tileCoords) const
{
	Vec2 tileCoordsFloats = tileCoords.GetVec2();
	return AABB2(tileCoordsFloats.x, tileCoordsFloats.y, tileCoordsFloats.x + 1.0f, tileCoordsFloats.y + 1.0f);
}


AABB2 Level::GetTileBounds(int index) const
{
	IntVec2 tileCoords = GetTileCoords(index);
	return GetTileBounds(tileCoords);
}


Tile Level::GetTile(IntVec2 const& tileCoords) const
{
	if (!IsTileCoordValid(tileCoords))
	{
		//ERROR_RECOVERABLE(Stringf("Trying to access tile with invalid tile coordinates: %i, %i", tileCoords.x, tileCoords.y));
		return Tile();
	}

	int tileIndex = GetTileIndex(tileCoords);
	return m_grid[tileIndex];
}


Tile Level::GetTile(int tileIndex) const
{
	if (tileIndex < 0 || tileIndex >= LEVEL_TILE_COUNT)
	{
		//ERROR_RECOVERABLE(Stringf("Trying to access tile with invalid tile index: %i", tileIndex));
		return Tile();
	}

	return m_grid[tileIndex];
}


void Level::UpdateCamera()
{
	EntityList playerList = m_entitiesByType[ENTITY_TYPE_MARIO];
	if (playerList.size() > 0)
	{
		Entity* player = playerList[0];
		Vec2 refPos = player->GetPosition();
		refPos += Vec2(0.0f, 5.0f);
		AABB2 camBox = m_gameplayCam.GetOrthoCamBoundingBox();
		Vec2 camBoxHalfDims = camBox.GetDimensions() * 0.5f;

		float minX = 0.0f + camBoxHalfDims.x;
		float maxX = LEVEL_SIZE_X - camBoxHalfDims.x;
		float x = Clamp(refPos.x, minX, maxX);

		float minY = 0.0f + camBoxHalfDims.y;
		float maxY = LEVEL_SIZE_Y - camBoxHalfDims.y;
		float y = Clamp(refPos.y, minY, maxY);

		Vec2 camPos = Vec2(x, y);
		m_gameplayCam.SetPosition(camPos);

		if (m_gameOverStartTime < 0.0f && !m_isGameCompletelyOver)
		{
			float greaterDimension = GetMax(camBoxHalfDims.x, camBoxHalfDims.y);
			if ((camPos - refPos).GetLengthSquared() > Square(greaterDimension))
			{
				TriggerGameOver(GAME_OVER_STATE_LOSS);
			}
		}
	}
}


void Level::AddVertsForTileToVector(std::vector<Vertex_PCU>& verts, Tile const& tile, int tileIndex) const
{
	Camera currentCamera = g_theRenderer->GetCurrentCamera();
	AABB2 camBounds = currentCamera.GetOrthoCamBoundingBox();
	Vec2 tileCenter = GetTileCenter(tileIndex);
	AABB2 tileBounds = AABB2(tileCenter, 1.0f, 1.0f);

	if (!DoAABB2sOverlap(tileBounds, camBounds))
		return;

	if (!tile.IsEmpty())
	{
		TileDefinition const* definition = tile.m_definition;
		Tileset const& tileset = definition->m_tileSet;
		if (tileset.m_type != TILE_SET_TYPE_NONE)
		{
			Rgba8 tileColor = Rgba8::WHITE;
			AABB2 tileUVs = GetUVForTile(tile, tileIndex);
			AddVertsForAABB2ToVector(verts, tileBounds, tileColor, tileUVs.m_mins, tileUVs.m_maxs);
		}
	}
}


void Level::CollectGarbage()
{
	for (int listIndex = 0; listIndex < ENTITY_TYPE_COUNT; listIndex++)
	{
		EntityList& list = m_entitiesByType[listIndex];
		for (int entityIndex = 0; entityIndex < list.size(); entityIndex++)
		{
			Entity*& e = list[entityIndex];
			if (e && e->IsGarbage())
			{
				Entity* removedEntity = RemoveEntityFromLevel(e);
				delete removedEntity;
				removedEntity = nullptr;
				entityIndex--;
			}
		}
	}
}