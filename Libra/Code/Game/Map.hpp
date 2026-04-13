#pragma once
#include<vector>
#include "Engine/Math/IntVec2.hpp"
#include "Game/Entity.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include<vector>

struct	Tile;
struct	Vec2;
struct	Vertex_PCU;
class	World;
struct	MapDefinition;

typedef std::vector<Entity*> EntityList;


struct RayCastResult
{
	Vec2	m_startPosition;
	Vec2	m_forwardNormal;
	float	m_maxDistance		= 0.0f;
	bool	m_didImpact			= false;
	float	m_impactDistance	= -1.0f;
	float	m_impactFraction	= 0.0f;
	Vec2	m_impactPosition;
	Vec2	m_impactSurfaceNormal;
	Entity* m_impactEntity		= nullptr;
};


class Map
{
public:
	Map(World* world, MapDefinition const* mapDef);
	~Map();
	
	void				Update(float deltaSeconds);
	void				Render() const;
	void				DeleteMap();
	void				GenerateValidMap();
	void				AddEntityToMap(Entity* entity);
	void				DeleteGarbage();
	Entity*				SpawnEntityOfType(EntityType type, Vec2 const& position = Vec2(0.0f,0.0f), float orientationDegrees = 0.0f);
	Entity*				RemoveEntityFromMap(Entity* entity);

	//Getters
	RayCastResult		RayCast(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, bool treatWaterAsSolid) const;
	Tile				GetTile(Vec2 const& worldPosition)														  const;
	Tile				GetTile(IntVec2 const& gridCoords)														  const;
	Vec2				GetWorldCoords(IntVec2 const& gridCoords)												  const;
	Vec2				GetEntryPosition()																		  const;
	IntVec2				GetMapDimensions()																		  const;
	IntVec2				GetGridCoords(Vec2 const& worldPosition)												  const;
	void				GetGridCoords(int gridArrayIndex, int& x, int& y)										  const;
	int					GetGridArrayIndex(IntVec2 tileCoords)													  const;
	bool				HasLineOfSight(Vec2 const& startPos, Vec2 const& endPos, float rangeOfVision = 0.0f)	  const;  //rangeOfVision = 0 means that there is no limit on the range
	bool				IsPointInSolid(Vec2 const& point, bool treatWaterAsSolid)								  const;
	bool				IsTileSolid(Tile const& tile, bool treatWaterAsSolid)									  const;
	bool				HasPlayerReachedExit(Entity* player)													  const;
	EntityList	const&	GetEntitiesByType(EntityType type)														  const;
	EntityList	const&	GetNonBulletEntitiesByFaction(EntityFaction faction)									  const;
	Tile				GetRandomTileForEntityAwayFromPlayer(bool canEntitySwim, float minDistanceFromPlayer = 0) const;

	//Mutators
private:
	void				GenerateGrid();
	void				CreateGridBorder(int gridDimensionsX, int gridDimensionsY);
	void				CheckAndReadMapDataFromImage();
	void				RenderHealthBarsForTanks()																const;
	void				RenderGrid()																			const;
	void				RenderEntitiesOfList(EntityList const& list)											const;
	void				AddVertsForTile(std::vector<Vertex_PCU>& mapVerts, Tile tile)							const;
	Entity*				CreateNewEntityOfType(EntityType entityType, Vec2 const& position, float orientationDegrees);
	void				AddEntityToList(Entity* entity, EntityList& list);
	void				RemoveEntityFromList(Entity* entity, EntityList& list);
	void				UpdateEntitiesOfList(EntityList& list, float deltaSeconds);
	void				CheckAndCorrectTankCollisions();
	void				CheckAndCorrectCollisionsWithTilesForEntityList(EntityList& listToCheck);
	void				CheckAndCorrectCollisionsWithinEntityList(EntityList& list);
	void				ResolveCollisionsBetweenEntities(Entity* a, Entity* b);
	bool				PushEntityOutOfTileIfColliding(Entity* entity, IntVec2 tileCoords);
	void				SpawnTileWormsOnGrid(std::string wormTileType, int wormLength, int numWorms);
	void				CreateSafeZonesInGrid();
	void				SpawnEnemies();
	bool				CheckMapValidity(float inaccessibleTileHeatValue);
	void				PopulateHeatMap(TileHeatMap& out_heatMap, IntVec2 const& startingTileCoords, float maxCost, bool treatWaterAsSolid = true);
	bool				CheckAndSetHeatValueAtTileCoords(TileHeatMap& out_heatMap, IntVec2 tileCoords, float referenceHeatValue, bool treatWaterAsSolid);
	void				CreateDistanceHeatMapForGoodTanks();

private:
	World*				 m_world;
	MapDefinition const* m_mapDef;
	std::vector<Tile>	 m_grid;
	EntityList			 m_allEntities;
	EntityList			 m_entityListsByType[NUM_ENTITY_TYPES]; 
	EntityList			 m_tankEntityListByFaction[NUM_ENTITY_FACTIONS];
	EntityList			 m_tankEntities;
	EntityList			 m_bulletEntities;
	EntityList			 m_explosionEntities;
	TileHeatMap			 m_distanceFromGoodEntityHeatMap;
	TileHeatMap			 m_tileGridHeatMap;
};
