#pragma once
#include <vector>
#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/Entity.hpp"
#include "Game/EntitySpawnInfo.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"

struct Vertex_PCU;
struct Vec2;
class LevelEditor;

struct LevelRaycastResult : public BaseRaycastResult2D
{
	IntVec2 m_impactTileCoords;
};

typedef std::vector<Entity*> EntityList;
typedef std::vector<EntitySpawnInfo> SpawnData;

class Level
{
public:
	friend class LevelEditor;

	enum LevelWinConditions : uint32_t
	{
		LEVEL_WIN_REACH_LEVEL_END,
		LEVEL_WIN_COLLECT_ALL_COINS,
		LEVEL_WIN_DEFEAT_ALL_ENEMIES
	};

	enum GameOverState
	{
		GAME_OVER_STATE_NONE,
		GAME_OVER_STATE_WON,
		GAME_OVER_STATE_LOSS
	};

public:
	Level();
	bool LoadLevelFromPath(std::string const& levelPath);
	void Init_EditorMode();
	void Init_PlayMode();
	void Deinit_EditorMode();
	void Deinit_PlayMode();
	void InitGrid();
	void Update();
	void CheckInput();

	void Render(bool isEditor) const;
	void RenderTiles() const;
	void RenderBackground() const;
	void RenderWaypointArrows(Rgba8 const& arrowColors) const;

	void AddLevelWinCondition(LevelWinConditions condition);

	LevelRaycastResult RaycastLevel(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, bool treatSemiSolidAsSolid = false);
	bool PushEntityOutOfTile(IntVec2 const& tileCoords, Entity*& entity);
	bool PushEntitiesOutOfEachOther(Entity*& a, Entity*& b);
	bool PushEntityOutOfEntity(Entity*& mobileEntity, Entity const* staticEntity);
	void HandleEntityEntityCollisions();
	void HandleOverlapBetweenEntities(Entity* a, Entity* b);
	
	void	AddEntitySpawn(EntitySpawnInfo const& info);
	void	RemoveEntitySpawn(EntitySpawnInfo const& info);
	Entity* SpawnEntity(EntitySpawnInfo const& spawnInfo);
	Entity* AddEntityToLevel(Entity* entity);
	Entity* RemoveEntityFromLevel(Entity* entity);
	Entity* GetEntityById(uint32_t id) const;

	void SetTileDefinitionAtCoords(TileDefinition const* tileDefinition, int tileX, int tileY);
	void SetTileDefinitionAtCoords(TileDefinition const* tileDefinition, IntVec2 const& tileCoords);
	void SetTileDefinitionAtIndex(TileDefinition const* tileDefinition, int tileIndex);
	void SetTileEmpty(IntVec2 const& tileCoords);
	void ClearAllTileData();
	void TagTile(IntVec2 const& tileCoords, Rgba8 const& color);
	void ClearAllSpawnData();
	Tile& GetTileReference(IntVec2 const& tileCoords);

	void UpdateSpawnInfo_TargetWaypoint(EntityID const& spawnInfoToUpdateId, EntityID const& newTargetWaypoint);

	void				MarkTileAsSelected(IntVec2 const& tileCoords);
	void				MarkTileAsDeselected(IntVec2 const& tileCoords);
	void				MarkEntitySpawnAsSelected(EntitySpawnInfo const& entitySpawn);
	void				MarkEntitySpawnAsDeselected(EntitySpawnInfo const& entitySpawn);
	void				MarkEverythingAsDeselected();

	void				AddOverlappingSpawnInfosToVector(std::vector<EntitySpawnInfo>& spawnInfoVector, EntitySpawnInfo const& refSpawnInfo)			const;
	bool				DoesTileIntersectSpawnInfo(IntVec2 const& tileCoords, EntitySpawnInfo const& info)												const;
	bool				DoesTileIntersectAnySpawnInfo(IntVec2 const& tileCoords)																		const;
	bool				DoesSpawnInfoIntersectSolidTiles(EntitySpawnInfo const& spawnInfo)																const;
	bool				DoesSpawnInfoExist(EntitySpawnInfo const& spawnInfo)																			const;
	bool				DoesSpawnInfoIntersectAnySpawnInfo(EntitySpawnInfo const& spawnInfo)															const;
	bool				DoSpawnInfosIntersect(EntitySpawnInfo const& a, EntitySpawnInfo const& b)														const;
	bool				DoesEntityOverlapAnyOtherEntity(Entity* refEntity)																				const;
																																						
	int					GetTileIndex(IntVec2 const& tileCoords)																							const;
	int					GetTileIndex(int tileX, int tileY)																								const;
	Vec2				GetTileCenter(int tileIndex)																									const;
	Vec2				GetTileBottomCenter(IntVec2 const& tileCoords)																					const;
	Vec2				GetTileTopCenter(IntVec2 const& tileCoords)																						const;
	Vec2				GetTileCenter(IntVec2 const& tileCoords)																						const;
	bool				IsLevelPaused()																													const;
	bool				IsTileSolid(Vec2 const& worldPosition, bool treatSemiSolidAsSolid = false)														const;
	bool				IsTileSolid(IntVec2 const& tileCoords, bool treatSemiSolidAsSolid = false)														const;
	bool				IsTileEmpty(IntVec2 const& tileCoords)																							const;
	bool				IsTileEmpty(int x, int y)																										const;
	bool				DoesTileDoDamage(IntVec2 const& tileCoords)																						const;
	bool				IsTileCoordValid(int x, int y)																									const;
	bool				IsTileCoordValid(IntVec2 const& tileCoords)																						const;
	bool				CanStepOnTile(IntVec2 const& tileCoords)																						const;
	bool				DoEntitiesTouch(Entity* a, Entity* b)																							const;
	Tile				GetTile(IntVec2 const& tileCoords)																								const;
	Tile				GetTile(int tileIndex)																											const;
	AABB2				GetTileBounds(IntVec2 const& tileCoords)																						const;
	AABB2				GetTileBounds(int index)																										const;
	IntVec2				GetTileCoords(Vec2 const& worldPosition)																						const;
	IntVec2				GetTileCoords(int tileIndex)																									const;
	Entity*				GetEntity(IntVec2 const& tileCoords)																							const;
	EntitySpawnInfo		GetEntitySpawnInfo(IntVec2 const& tileCoords)																					const;
	EntitySpawnInfo		GetEntitySpawnInfo(Vec2 const& refPos)																							const;
	AABB2				GetUVForTile(Tile const& tile, int tileIndex)																					const;
	SpawnData const&	GetLevelSpawnData()																												const;
	EntitySpawnInfo	const&	GetEntitySpawnInfoById(EntityID const& refId)																				const;
	Entity* GetMario() const;
																																			 
	void				AddNonEmptyEastTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds)									const;
	void				AddNonEmptyWestTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds)									const;
	void				AddNonEmptyNorthTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds)									const;
	void				AddNonEmptySouthTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds)									const;
	void				AddNonEmptyTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds)										const;
	void				AddTileCoordsForBoundsToVector(std::vector<IntVec2>& vector, AABB2 const& bounds)												const;
	bool				WereEntitiesOverlappingPrevFrame(uint64_t touchingEntityId)																		const;
	bool				CheckLevelEndCondition();

private:
	void UpdateCamera();
	void AddVertsForTileToVector(std::vector<Vertex_PCU>& verts, Tile const& tile, int tileIndex) const;
	void CollectGarbage();
	void HandleCollisionBetweenEntityAndTiles();
	bool HandleTileEntityCollision(IntVec2 const& tileCoords, Entity*& e);
	//void HandleCollisionsBetweenEntitiesInList(EntityList& m_allEntities);

	Entity* AddEntityToList(EntityList& list, Entity* entity, bool assignId = false);
	Entity* RemoveEntityFromList(EntityList& list, Entity* entity);

	void LoadGridFromXmlElement(XmlElement* element);
	void LoadEntitySpawnDataFromXmlElement(XmlElement* element);

	void TriggerGameOver(GameOverState gameOverState);
private:
	Tile m_grid[LEVEL_TILE_COUNT] = {};
	EntityList m_entitiesByType[ENTITY_TYPE_COUNT] = {};
	EntityList m_allEntities;
	SpawnData m_spawnData;
	Camera m_gameplayCam;
	Camera m_uiCam;
	uint32_t m_levelWinFlags;

	float m_gameOverStartTime = -1.0f;
	float m_gameOverDuration = 0.0f;
	bool m_isGameCompletelyOver = false;
	GameOverState m_gameOverState = GAME_OVER_STATE_NONE;

	SoundPlaybackID m_bgMusicPlaybackId;

	std::vector<uint64_t> m_curFrameOverlappingEntityIds;
	std::vector<uint64_t> m_prevFrameOverlappingEntityIds;

	Clock m_levelClock;
};
