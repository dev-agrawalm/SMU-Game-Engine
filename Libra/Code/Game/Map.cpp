#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Bullet.hpp"
#include "Game/Aries.hpp"
#include "Game/Leo.hpp"
#include "Game/Scorpio.hpp"
#include "Game/World.hpp"
#include "Engine/Core/Image.hpp"
#include "Game/Capricorn.hpp"
#include "Game/Sagittarius.hpp"
#include "Game/Explosion.hpp"


Map::Map(World* world, MapDefinition const* mapDef) :
	m_world(world),
	m_mapDef(mapDef)
{
	m_distanceFromGoodEntityHeatMap = TileHeatMap(m_mapDef->m_gridDimensions);
	m_distanceFromGoodEntityHeatMap.SetAllValues(99.0f);
	m_tileGridHeatMap = TileHeatMap(m_mapDef->m_gridDimensions);

	GenerateValidMap();
	SpawnEnemies();
}


Map::~Map()
{
}


void Map::Update(float deltaSeconds)
{
	UpdateEntitiesOfList(m_tankEntityListByFaction[ENTITY_FACTION_GOOD], deltaSeconds);
	UpdateEntitiesOfList(m_tankEntityListByFaction[ENTITY_FACTION_NEUTRAL], deltaSeconds);
	CreateDistanceHeatMapForGoodTanks();
	UpdateEntitiesOfList(m_tankEntityListByFaction[ENTITY_FACTION_EVIL], deltaSeconds);
	CheckAndCorrectTankCollisions();
	UpdateEntitiesOfList(m_bulletEntities, deltaSeconds);
	UpdateEntitiesOfList(m_explosionEntities, deltaSeconds);

	DeleteGarbage();
}


void Map::Render() const
{
	RenderGrid();
	RenderEntitiesOfList(m_tankEntities);
	RenderEntitiesOfList(m_bulletEntities);

	g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
	RenderEntitiesOfList(m_explosionEntities);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	RenderHealthBarsForTanks();
}


void Map::RenderGrid() const
{
	unsigned int numVertsPerTile = NUM_VERTS_QUAD;
	size_t numVerts = static_cast<size_t>(numVertsPerTile) * m_grid.size();
	std::vector<Vertex_PCU> mapVerts;
	mapVerts.reserve(numVerts);

	for (size_t tileIndex = 0; tileIndex < m_grid.size(); tileIndex++)
	{
		AddVertsForTile(mapVerts, m_grid[tileIndex]);
	}

	Texture const* tilesTexture = TileDefinition::GetTileSpriteSheetTexture();
	g_theRenderer->BindTexture(0, tilesTexture);
	g_theRenderer->DrawVertexArray(static_cast<int>(mapVerts.size()), mapVerts.data());
}


void Map::RenderEntitiesOfList(EntityList const& list) const
{
	for (int entityIndex = 0; entityIndex < list.size(); entityIndex++)
	{
		Entity* const& entity = list[entityIndex];
		if (entity)
		{
			entity->Render();
		}
	}
}


void Map::UpdateEntitiesOfList(EntityList& list, float deltaSeconds)
{
	for (int entityIndex = 0; entityIndex < list.size(); entityIndex++)
	{
		Entity*& entity = list[entityIndex];
		if (entity)
		{
			entity->Update(deltaSeconds);
		}
	}
}


void Map::AddVertsForTile(std::vector<Vertex_PCU>& mapVerts, Tile tile) const
{
	AABB2 tileBoundingBox	= tile.GetBoundingBox();
//	float tileHeatValue		= m_distanceFromGoodEntityHeatMap.GetValueAtTile(GetGridArrayIndex(tile.m_gridCoords));
//	tileHeatValue			= RangeMapClamped(tileHeatValue, 0.0f, 20.0f, 255.0f, 0.0f);
	Rgba8 tileTint			= tile.m_tileDefinition->m_tint;
// 	if(!IsTileSolid(tile, true))
// 		tileTint.a				= static_cast<unsigned char>(tileHeatValue);
	Vec2 tileUVMins			= tile.m_tileDefinition->m_uvMins;
	Vec2 tileUVMaxs			= tile.m_tileDefinition->m_uvMaxs;
	AddVertsForAABB2ToVector(mapVerts, tileBoundingBox, tileTint, tileUVMins, tileUVMaxs);
}


void Map::DeleteMap()
{
	for (int entityIndex = 0; entityIndex < m_allEntities.size(); entityIndex++)
	{
		Entity*& entity = m_allEntities[entityIndex];
		if (entity)
		{
			Entity* removedEntity = RemoveEntityFromMap(entity);
			delete removedEntity;
			removedEntity = nullptr;
		}
	}
	
	m_grid.clear();
	m_grid.resize(0);
}


void Map::GenerateValidMap()
{
	float inaccessibleTileHeatValue = 99.0f;
	bool isMapValid					= false;
	int mapGenerationCount			= 0;
	while (!isMapValid)
	{
		GUARANTEE_OR_DIE(mapGenerationCount < 1000, "Unable to generate a valid map in 1000 iterations");

		GenerateGrid();
		isMapValid = CheckMapValidity(inaccessibleTileHeatValue);
		mapGenerationCount++;
	}

	TileDefinition const* borderTileDef = TileDefinition::GetTileDefinition(m_mapDef->m_borderTileType);
	for (int tileIndex = 0; tileIndex < m_grid.size(); tileIndex++)
	{
		float heatValueAtTile = m_tileGridHeatMap.GetValueAtTile(tileIndex);
		Tile& tile = m_grid[tileIndex];
		if (heatValueAtTile == inaccessibleTileHeatValue && !IsTileSolid(tile, true))
		{
			m_grid[tileIndex].m_tileDefinition = borderTileDef;
		}
	}
}


Entity* Map::SpawnEntityOfType(EntityType type, Vec2 const& position /*= Vec2(0.0f,0.0f)*/, float orientationDegrees /*= 0.0f*/)
{
	Entity* entity = CreateNewEntityOfType(type, position, orientationDegrees);
	AddEntityToMap(entity);
	return entity;
}


void Map::DeleteGarbage()
{
	for (int entityIndex = 0; entityIndex < m_allEntities.size(); entityIndex++)
	{
		Entity*& entity = m_allEntities[entityIndex];
		if (entity && entity->m_isGarbage)
		{
			Entity* garbageEntity = RemoveEntityFromMap(entity);
			delete garbageEntity;
			garbageEntity = nullptr;
		}
	}
}


Entity* Map::CreateNewEntityOfType(EntityType entityType, Vec2 const& position, float orientationDegrees)
{
	switch (entityType)
	{
		case ENTITY_TYPE_GOOD_PLAYER:			return new Player		(position, ENTITY_FACTION_GOOD, entityType, this);
		case ENTITY_TYPE_GOOD_BULLET:			return new Bullet		(position, orientationDegrees, ENTITY_FACTION_GOOD,		entityType, this);
		case ENTITY_TYPE_EVIL_BULLET:			return new Bullet		(position, orientationDegrees, ENTITY_FACTION_EVIL,		entityType, this);
		case ENTITY_TYPE_EVIL_SCORPIO:			return new Scorpio		(position, orientationDegrees, ENTITY_FACTION_EVIL,		entityType, this);
		case ENTITY_TYPE_EVIL_LEO:				return new Leo			(position, orientationDegrees, ENTITY_FACTION_EVIL,		entityType, this);
		case ENTITY_TYPE_EVIL_ARIES:			return new Aries		(position, orientationDegrees, ENTITY_FACTION_EVIL,		entityType, this);
		case ENTITY_TYPE_EVIL_CAPRICORN:		return new Capricorn	(position, orientationDegrees, ENTITY_FACTION_EVIL,		entityType, this);
		case ENTITY_TYPE_EVIL_GUIDED_MISSILE:	return new Bullet		(position, orientationDegrees, ENTITY_FACTION_EVIL,		entityType, this);
		case ENTITY_TYPE_EVIL_SAGITTARIUS:		return new Sagittarius	(position, orientationDegrees, ENTITY_FACTION_EVIL,		entityType, this);
		case ENTITY_TYPE_TANK_EXPLOSION:		//fall through
		case ENTITY_TYPE_BULLET_EXPLOSION:		//fall through
		case ENTITY_TYPE_PLAYER_EXPLOSION:		return new Explosion	(position, orientationDegrees, ENTITY_FACTION_NEUTRAL,	entityType, this);
		default:
			ERROR_RECOVERABLE("Invalid Entity type requested to be spawned");
			break;
	}

	return nullptr;
}


void Map::GenerateGrid()
{
	int gridDimensionsX = m_mapDef->m_gridDimensions.x;
	int gridDimensionsY = m_mapDef->m_gridDimensions.y;
	unsigned int numTiles = gridDimensionsX * gridDimensionsY;
	size_t gridSize = static_cast<size_t>(numTiles);
	m_grid.clear();
	m_grid.resize(gridSize);

	std::string fillTile = m_mapDef->m_fillTileType;
	if(fillTile == "")
		ERROR_AND_DIE("No fill tile type provided");

	TileDefinition const* fillTileDef = TileDefinition::GetTileDefinition(fillTile);
	for (int y = 0; y < gridDimensionsY; y++)
	{
		for (int x = 0; x < gridDimensionsX; x++)
		{
			IntVec2 tileGridCoords = IntVec2(x, y);
			int gridArrayIndex = GetGridArrayIndex(tileGridCoords);
			m_grid[gridArrayIndex].m_gridCoords = tileGridCoords;
			m_grid[gridArrayIndex].m_tileDefinition	= fillTileDef;
		}
	}

	CreateGridBorder(gridDimensionsX, gridDimensionsY);

	std::string wormTileType = m_mapDef->m_worm1TileType;
	if(wormTileType != "")
		SpawnTileWormsOnGrid(wormTileType, m_mapDef->m_worm1Length, m_mapDef->m_numWorms1);

	wormTileType = m_mapDef->m_worm2TileType;
	if (wormTileType != "")
		SpawnTileWormsOnGrid(wormTileType, m_mapDef->m_worm2Length, m_mapDef->m_numWorms2);

	CreateSafeZonesInGrid();

	Tile entryTile = GetTile(m_mapDef->m_entryPointGridCoords);
	int entryTileIndex = GetGridArrayIndex(entryTile.m_gridCoords);
	m_grid[entryTileIndex].m_tileDefinition	= TileDefinition::GetTileDefinition("MAP_ENTRY");
	
	Tile exitTile = GetTile(m_mapDef->m_exitPointGridCoords);
	int exitTileIndex = GetGridArrayIndex(exitTile.m_gridCoords);
	m_grid[exitTileIndex].m_tileDefinition	= TileDefinition::GetTileDefinition("MAP_EXIT");

	CheckAndReadMapDataFromImage();
}


void Map::CreateGridBorder(int gridDimensionsX, int gridDimensionsY)
{
	std::string borderTile = m_mapDef->m_borderTileType;
	if (borderTile == "")
		ERROR_AND_DIE("No border tile type provided");

	TileDefinition const* borderTileDef = TileDefinition::GetTileDefinition(borderTile);
	for (int x = 0; x < gridDimensionsX; x++)
	{
		IntVec2 bottomRowCoords = IntVec2(x, 0);
		IntVec2 topRowCoords = IntVec2(x, gridDimensionsY - 1);
		int bottomRowGridArrayIndex = GetGridArrayIndex(bottomRowCoords);
		int topRowGridArrayIndex = GetGridArrayIndex(topRowCoords);

		m_grid[bottomRowGridArrayIndex].m_tileDefinition = borderTileDef;
		m_grid[topRowGridArrayIndex].m_tileDefinition = borderTileDef;
	}

	for (int y = 0; y < gridDimensionsY; y++)
	{
		IntVec2 leftMostColumnCoords = IntVec2(0, y);
		IntVec2 rightMostColumnCoords = IntVec2(gridDimensionsX - 1, y);
		int leftMostColumnGridArrayIndex = GetGridArrayIndex(leftMostColumnCoords);
		int rightMostColumnGridArrayIndex = GetGridArrayIndex(rightMostColumnCoords);

		m_grid[leftMostColumnGridArrayIndex].m_tileDefinition = borderTileDef;
		m_grid[rightMostColumnGridArrayIndex].m_tileDefinition = borderTileDef;
	}
}


void Map::CheckAndReadMapDataFromImage()
{
	if (m_mapDef->m_mapImageName == "")
		return;

	Image image(m_mapDef->m_mapImageName.c_str());
	IntVec2 imageDimensions = image.GetDimensions();
	for (int y = 0; y < imageDimensions.y; y++)
	{
		for (int x = 0; x < imageDimensions.x; x++)
		{
			Rgba8 texelData = image.GetTexelRgba8(x, y);
			if (texelData.a > 0)
			{
				TileDefinition const* tileDef = TileDefinition::GetTileDefinition(texelData);
				GUARANTEE_OR_DIE(tileDef, Stringf("Map image contains invalid color"));

				int tileProbability = g_rng->GetRandomIntInRange(0, 254);
				if (tileProbability < texelData.a)
				{
					IntVec2 tileCoords = IntVec2(x, y) + m_mapDef->m_mapImageOffset;
					int tileIndex = GetGridArrayIndex(tileCoords);
					if(tileIndex>=0 && tileIndex < m_grid.size())
						m_grid[tileIndex].m_tileDefinition = tileDef;
				}
			}
		}
	}
}


void Map::RenderHealthBarsForTanks() const
{
	std::vector<Vertex_PCU> healthBarVertexes;
	healthBarVertexes.reserve(m_tankEntities.size() * 12); //12 vertexes per health bar per entity //a health bar is two quads and 6 vertexes per quad

	for (int tankIndex = 0; tankIndex < (int) m_tankEntities.size(); tankIndex++)
	{
		Entity* const& tank = m_tankEntities[tankIndex];
		if(tank->m_isDead)
			continue;
		
		Vec2 healthBarCenter = tank->m_position + Vec2(0.0f, 1.0f) * 0.6f;
		AABB2 healthBarOuterBox = AABB2(healthBarCenter, 0.5f, 0.04f);
		Vec2 healthBarInnerBoxMins = healthBarOuterBox.m_mins;
		float tankHealthFraction = (float) tank->m_health / (float) tank->m_maxHealth;
		float healthBarInnerBoxWidth = healthBarOuterBox.GetDimensions().x * tankHealthFraction;
		float healthBarInnerBoxHeight = healthBarOuterBox.GetDimensions().y;
		Vec2 healthBarInnerBoxMaxs = healthBarInnerBoxMins + Vec2(healthBarInnerBoxWidth, healthBarInnerBoxHeight);
		AABB2 healthBarInnerBox = AABB2(healthBarInnerBoxMins, healthBarInnerBoxMaxs);

		AddVertsForAABB2ToVector(healthBarVertexes, healthBarOuterBox, Rgba8::BLACK);
		AddVertsForAABB2ToVector(healthBarVertexes, healthBarInnerBox, Rgba8::RED);
	}

	g_theRenderer->SetDiffuseTexture(nullptr);
	g_theRenderer->DrawVertexArray((int) healthBarVertexes.size(), healthBarVertexes.data());
}


void Map::SpawnTileWormsOnGrid(std::string wormTileType, int wormLength, int numWorms)
{
	IntVec2 const directions[4] = {
		NORTH,
		SOUTH,
		WEST,
		EAST
	};
	TileDefinition const* wormTileDef = TileDefinition::GetTileDefinition(wormTileType);
	std::string const borderTileDefName = TileDefinition::GetTileDefinition(m_mapDef->m_borderTileType)->m_name;

	for (numWorms; numWorms > 0; numWorms--)
	{
		int randomTileX							  = g_rng->GetRandomIntInRange(1, m_mapDef->m_gridDimensions.x - 2);
		int randomTileY							  = g_rng->GetRandomIntInRange(1, m_mapDef->m_gridDimensions.y - 2);
		IntVec2 startingTileCoords				  = IntVec2(randomTileX, randomTileY);
		IntVec2 currentTileCoords				  = startingTileCoords;
		int currentTileIndex					  = GetGridArrayIndex(startingTileCoords);
		m_grid[currentTileIndex].m_tileDefinition = wormTileDef;
		int newWormLength						  = wormLength - 1;

		for (newWormLength; newWormLength > 0; newWormLength--)
		{
			int randomDirectionIndex	  = g_rng->GetRandomIntLessThan(4);
			IntVec2 newTileCoords		  = currentTileCoords + directions[randomDirectionIndex];
			int newTileIndex			  = GetGridArrayIndex(newTileCoords);
			newTileIndex				  = Clamp(newTileIndex, 0, (int) (m_grid.size() - 1));
			std::string newTileDefName	  = m_grid[newTileIndex].m_tileDefinition->m_name;
			if (newTileDefName != borderTileDefName)
			{
				m_grid[newTileIndex].m_tileDefinition = wormTileDef;
				currentTileCoords = newTileCoords;
			}
			else
				newWormLength++;
		}
	}
}


void Map::CreateSafeZonesInGrid()
{
	int gridDimensionsX	= m_mapDef->m_gridDimensions.x;
	int gridDimensionsY	= m_mapDef->m_gridDimensions.y;
	std::string borderTile = m_mapDef->m_borderTileType;
	TileDefinition const* borderTileDef = TileDefinition::GetTileDefinition(borderTile);

	//Start safe zone
	int startSafeZoneLength	= m_mapDef->m_startSafeZoneSqLength;
	std::string startSafeZoneTileType = m_mapDef->m_startSafeZoneTileType;
	TileDefinition const* startSafeZoneTileDef = TileDefinition::GetTileDefinition(startSafeZoneTileType);
	for (int y = 1; y < startSafeZoneLength + 1; y++)
	{
		y = Clamp(y, 0, gridDimensionsY - 1);
		for (int x = 1; x < startSafeZoneLength + 1; x++)
		{
			x = Clamp(x, 0, gridDimensionsX - 1);
			int gridArrayIndex = GetGridArrayIndex(IntVec2(x, y));
			m_grid[gridArrayIndex].m_tileDefinition = startSafeZoneTileDef;

			if (x == startSafeZoneLength &&
				y <= startSafeZoneLength &&
				y >= (startSafeZoneLength - (int) (startSafeZoneLength * 0.5f)))
			{
				m_grid[gridArrayIndex].m_tileDefinition = borderTileDef;
			}
			if (y == startSafeZoneLength &&
				x <= startSafeZoneLength &&
				x >= (startSafeZoneLength - (int) (startSafeZoneLength * 0.5f)))
			{
				m_grid[gridArrayIndex].m_tileDefinition = borderTileDef;
			}
		}
	}

	//end safe zone
	int endSafeZoneLength = m_mapDef->m_endSafeZoneSqLength;
	std::string endSafeZoneTileType	= m_mapDef->m_endSafeZoneTileType;
	TileDefinition const* endSafeZoneTileDef = TileDefinition::GetTileDefinition(endSafeZoneTileType);
	for (int y = (gridDimensionsY - 1) - endSafeZoneLength; y < (gridDimensionsY - 1); y++)
	{
		y = Clamp(y, 0, gridDimensionsY - 1);
		for (int x = (gridDimensionsX - 1) - endSafeZoneLength; x < (gridDimensionsX - 1); x++)
		{
			x = Clamp(x, 0, gridDimensionsX - 1);
			int gridArrayIndex = GetGridArrayIndex(IntVec2(x, y));
			m_grid[gridArrayIndex].m_tileDefinition = endSafeZoneTileDef;

			if (x == ((gridDimensionsX - 1) - endSafeZoneLength) &&
				y >= ((gridDimensionsY - 1) - endSafeZoneLength) &&
				y <= (((gridDimensionsY - 1) - endSafeZoneLength) + (int) (endSafeZoneLength * 0.5f)))
			{
				m_grid[gridArrayIndex].m_tileDefinition = borderTileDef;
			}
			if (y == ((gridDimensionsY - 1) - endSafeZoneLength) &&
				x >= ((gridDimensionsX - 1) - endSafeZoneLength) &&
				x <= ((gridDimensionsX - 1) - endSafeZoneLength) + (int) (endSafeZoneLength * 0.5f))
			{
				m_grid[gridArrayIndex].m_tileDefinition = borderTileDef;
			}
		}
	}
}


bool Map::HasPlayerReachedExit(Entity* player) const
{
	Vec2 playerPosition = player->m_position;
	Vec2 exitPosition	= GetTile(m_mapDef->m_exitPointGridCoords).GetCenter();
	float tileWidth		= g_gameConfigBlackboard.GetValue("tileWidth", 1.0f);

	return IsPointInsideDisk2D(playerPosition, exitPosition, tileWidth * 0.5f);
}


void Map::SpawnEnemies()
{
	int numAries		= m_mapDef->m_numOfEnemyAries;
	int numLeos			= m_mapDef->m_numOfEnemyLeos;
	int numScorpios		= m_mapDef->m_numOfEnemyScorpios;
	int numCapricorns	= m_mapDef->m_numOfEnemyCapricorns;
	int numSagittarius	= m_mapDef->m_numOfEnemySagittarius;
	float minDistanceFromPlayer = static_cast<float>((m_mapDef->m_startSafeZoneSqLength - 1) * 2);

	for (int ariesIndex = 0; ariesIndex < numAries; ariesIndex++)
	{
		Tile tile = GetRandomTileForEntityAwayFromPlayer(false, minDistanceFromPlayer);
		AABB2 tileBox = tile.GetBoundingBox();
		float orientationDegrees = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
		SpawnEntityOfType(ENTITY_TYPE_EVIL_ARIES, tileBox.GetCenter(), orientationDegrees);
	}
	
	for (int leoIndex = 0; leoIndex < numLeos; leoIndex++)
	{
		Tile tile = GetRandomTileForEntityAwayFromPlayer(false, minDistanceFromPlayer);
		AABB2 tileBox = tile.GetBoundingBox();
		float orientationDegrees = g_rng->GetRandomFloatInRange(DEGREES_0, DEGREES_360);
		SpawnEntityOfType(ENTITY_TYPE_EVIL_LEO, tileBox.GetCenter(), orientationDegrees);
	}
	
	for (int scorpioIndex = 0; scorpioIndex < numScorpios; scorpioIndex++)
	{
		Tile tile = GetRandomTileForEntityAwayFromPlayer(false, minDistanceFromPlayer);
		AABB2 tileBox = tile.GetBoundingBox();
		SpawnEntityOfType(ENTITY_TYPE_EVIL_SCORPIO, tileBox.GetCenter());
	}
	
	for (int capricornIndex = 0; capricornIndex < numCapricorns; capricornIndex++)
	{
		Tile tile = GetRandomTileForEntityAwayFromPlayer(true, minDistanceFromPlayer);
		AABB2 tileBox = tile.GetBoundingBox();
		SpawnEntityOfType(ENTITY_TYPE_EVIL_CAPRICORN, tileBox.GetCenter());
	}

	for (int sagittariusIndex = 0; sagittariusIndex < numSagittarius; sagittariusIndex++)
	{
		Tile tile = GetRandomTileForEntityAwayFromPlayer(false, minDistanceFromPlayer);
		AABB2 tileBox = tile.GetBoundingBox();
		SpawnEntityOfType(ENTITY_TYPE_EVIL_SAGITTARIUS, tileBox.GetCenter());
	}
}


bool Map::CheckMapValidity(float inaccessibleTileHeatValue)
{
	IntVec2 startingTileCoords = m_mapDef->m_entryPointGridCoords;
	PopulateHeatMap(m_tileGridHeatMap, startingTileCoords, inaccessibleTileHeatValue, true);

	int exitTileIndex = GetGridArrayIndex(m_mapDef->m_exitPointGridCoords);
	return (m_tileGridHeatMap.GetValueAtTile(exitTileIndex) != inaccessibleTileHeatValue);
}


void Map::PopulateHeatMap(TileHeatMap& out_heatMap, IntVec2 const& startingTileCoords, float maxCost, bool treatWaterAsSolid /*= true*/)
{
	out_heatMap.SetAllValues(maxCost);
	std::vector<IntVec2> tileCoordsStack;

	int startingTileIndex = GetGridArrayIndex(startingTileCoords);
	out_heatMap.SetValueAtTile(startingTileIndex, 0.0f);
	tileCoordsStack.push_back(startingTileCoords);

	while ( (int) tileCoordsStack.size() > 0)
	{
		IntVec2 tileCoords = tileCoordsStack.back();
		tileCoordsStack.pop_back();
		int tileIndex		  = GetGridArrayIndex(tileCoords);
		float heatValueAtTile = out_heatMap.GetValueAtTile(tileIndex);

		IntVec2 northTileCoords = tileCoords + NORTH;
		if (CheckAndSetHeatValueAtTileCoords(out_heatMap, northTileCoords, heatValueAtTile, treatWaterAsSolid))
		{
			tileCoordsStack.push_back(northTileCoords);
		}


		IntVec2 southTileCoords = tileCoords + SOUTH;
		if (CheckAndSetHeatValueAtTileCoords(out_heatMap, southTileCoords, heatValueAtTile, treatWaterAsSolid))
		{
			tileCoordsStack.push_back(southTileCoords);
		}

		IntVec2 eastTileCoords	= tileCoords + EAST;
		if (CheckAndSetHeatValueAtTileCoords(out_heatMap, eastTileCoords, heatValueAtTile, treatWaterAsSolid))
		{
			tileCoordsStack.push_back(eastTileCoords);
		}

		IntVec2 westTileCoords	= tileCoords + WEST;
		if (CheckAndSetHeatValueAtTileCoords(out_heatMap, westTileCoords, heatValueAtTile, treatWaterAsSolid))
		{
			tileCoordsStack.push_back(westTileCoords);
		}
	}
}


bool Map::CheckAndSetHeatValueAtTileCoords(TileHeatMap& out_heatMap, IntVec2 tileCoords, float referenceHeatValue, bool treatWaterAsSolid)
{
	int tileIndex = GetGridArrayIndex(tileCoords);
	if (tileIndex >= 0 && tileIndex < (int) m_grid.size())
	{
		Tile tile				= GetTile(tileCoords);
		float heatValueAtTile	= out_heatMap.GetValueAtTile(tileIndex);
		if (!IsTileSolid(tile, treatWaterAsSolid) && heatValueAtTile > referenceHeatValue + 1.0f)
		{
			out_heatMap.SetValueAtTile(tileIndex, referenceHeatValue + 1.0f);
			return true;
		}
	}

	return false;
}


void Map::CreateDistanceHeatMapForGoodTanks()
{
	float maxHeatValue = 99.0f;
	EntityList& nonBulletGoodEntities = m_tankEntityListByFaction[ENTITY_FACTION_GOOD];
	for (int entityIndex = 0; entityIndex < nonBulletGoodEntities.size(); entityIndex++)
	{
		Entity*& entity = nonBulletGoodEntities[entityIndex];
		IntVec2 entityTileCoords = GetGridCoords(entity->m_position);
		int tileIndex = GetGridArrayIndex(entityTileCoords);
		float heatValueAtTile = m_distanceFromGoodEntityHeatMap.GetValueAtTile(tileIndex);
		if(heatValueAtTile > 0.0f)
			PopulateHeatMap(m_distanceFromGoodEntityHeatMap, entityTileCoords, maxHeatValue);
	}

	EntityList& scorpios = m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO];
	for (int scorpioIndex = 0; scorpioIndex < scorpios.size(); scorpioIndex++)
	{
		Entity*& scorpio = scorpios[scorpioIndex];
		if (scorpio && scorpio->IsAlive())
		{
			IntVec2 scorpioGridCoords = GetGridCoords(scorpio->m_position);
			int scorpioTileIndex = GetGridArrayIndex(scorpioGridCoords);
			m_distanceFromGoodEntityHeatMap.SetValueAtTile(scorpioTileIndex, maxHeatValue);
		}
	}
}


Tile Map::GetRandomTileForEntityAwayFromPlayer(bool canEntitySwim, float minDistanceFromPlayer /*= 0*/) const
{
	int gridDimensionsX = m_mapDef->m_gridDimensions.x;
	int gridDimensionsY = m_mapDef->m_gridDimensions.y;

	int randomTileX = g_rng->GetRandomIntInRange(1, gridDimensionsX - 2);
	int randomTileY = g_rng->GetRandomIntInRange(1, gridDimensionsY - 2);
	Tile tile		= GetTile(IntVec2(randomTileX, randomTileY));
	TileDefinition const *& tileDef	= tile.m_tileDefinition;
	float tileDistanceFromPlayer = m_distanceFromGoodEntityHeatMap.GetValueAtTile(GetGridArrayIndex(tile.m_gridCoords));
	while (IsTileSolid(tile, !canEntitySwim) || tileDistanceFromPlayer < minDistanceFromPlayer)
	{
		randomTileX = g_rng->GetRandomIntInRange(1, gridDimensionsX - 2);
		randomTileY = g_rng->GetRandomIntInRange(1, gridDimensionsY - 2);
		tile		= GetTile(IntVec2(randomTileX, randomTileY));
		tileDef		= tile.m_tileDefinition;
	}

	return tile;
}


void Map::CheckAndCorrectTankCollisions()
{
	CheckAndCorrectCollisionsWithinEntityList(m_tankEntities);
	CheckAndCorrectCollisionsWithTilesForEntityList(m_tankEntities);
}


void Map::CheckAndCorrectCollisionsWithTilesForEntityList(EntityList& listToCheck)
{
	for (int entityIndex = 0; entityIndex < listToCheck.size(); entityIndex++)
	{
		Entity*& entity = listToCheck[entityIndex];
		if (entity && entity->IsAlive() && entity->m_isPushedByWalls)
		{
			Vec2 entityWorldPosition = entity->GetPosition();
			IntVec2 entityGridCoords = GetGridCoords(entityWorldPosition);

			PushEntityOutOfTileIfColliding(entity, entityGridCoords + NORTH);
			PushEntityOutOfTileIfColliding(entity, entityGridCoords + SOUTH);
			PushEntityOutOfTileIfColliding(entity, entityGridCoords + WEST);
			PushEntityOutOfTileIfColliding(entity, entityGridCoords + EAST);

			PushEntityOutOfTileIfColliding(entity, entityGridCoords + SOUTH_EAST);
			PushEntityOutOfTileIfColliding(entity, entityGridCoords + SOUTH_WEST);
			PushEntityOutOfTileIfColliding(entity, entityGridCoords + NORTH_WEST);
			PushEntityOutOfTileIfColliding(entity, entityGridCoords + NORTH_EAST);
		}
	}
}


void Map::CheckAndCorrectCollisionsWithinEntityList(EntityList& list)
{
	for (int outerEntityIndex = 0; outerEntityIndex < list.size(); outerEntityIndex++)
	{
		Entity*& outerLoopEntity = list[outerEntityIndex];
		for (int innerEntityIndex = 0; innerEntityIndex < list.size(); innerEntityIndex++)
		{
			Entity*& innerLoopEntity = list[innerEntityIndex];
			if (outerEntityIndex != innerEntityIndex)
			{
				ResolveCollisionsBetweenEntities(outerLoopEntity, innerLoopEntity);
			}
		}
	}
}


void Map::ResolveCollisionsBetweenEntities(Entity* a, Entity* b)
{
	if(a->m_isDead || b->m_isDead)
		return;

	bool isAStatic = a->m_doesPushEntities && !a->m_isPushedByEntities;
	bool isBStatic = b->m_doesPushEntities && !b->m_isPushedByEntities;
	
	if (isAStatic && !isBStatic)
	{
		PushDiskOutOfDisk2D(b->m_position, b->m_physicsRadius,a->m_position, a->m_physicsRadius);
		return;
	}
	
	if (!isAStatic && isBStatic)
	{
		PushDiskOutOfDisk2D(a->m_position, a->m_physicsRadius, b->m_position, b->m_physicsRadius);
		return;
	}

	PushDisksOutOfEachOther2D(a->m_position, a->m_physicsRadius, b->m_position, b->m_physicsRadius);
}


bool Map::PushEntityOutOfTileIfColliding(Entity* entity, IntVec2 tileCoords)
{
	int	tileGridArrayIndex		= GetGridArrayIndex(tileCoords);
	tileGridArrayIndex			= Clamp(tileGridArrayIndex, 0, (int)m_grid.size());
	Tile tile					= m_grid[tileGridArrayIndex];

	if (IsTileSolid(tile, !entity->m_canSwim))
	{
		AABB2	tileBoundingBox  = tile.GetBoundingBox();
		float	entityDiskRadius = entity->GetPhysicsRadius();
		Vec2&	diskCenter		 = entity->m_position;
		return PushDiskOutOfAABB2D(diskCenter, entityDiskRadius, tileBoundingBox);
	}

	return false;
}


IntVec2 Map::GetGridCoords(Vec2 const& worldPosition) const
{
	float tileWidth = g_gameConfigBlackboard.GetValue("tileWidth", 1.0f);
	int x			= RoundDownToInt(worldPosition.x / tileWidth);
	int y			= RoundDownToInt(worldPosition.y / tileWidth);
	return IntVec2(x, y);
}


void Map::GetGridCoords(int gridArrayIndex, int& x, int& y) const
{
	int gridDimensionsX = m_mapDef->m_gridDimensions.x;
	y = gridArrayIndex / gridDimensionsX;
	x = gridArrayIndex - (y * gridDimensionsX);
}


Vec2 Map::GetWorldCoords(IntVec2 const& gridCoords) const
{
	float tileWidth = g_gameConfigBlackboard.GetValue("tileWidth", 1.0f);
	float worldX	= ((gridCoords.x * tileWidth) + (tileWidth * 0.5f));
	float worldY	= ((gridCoords.y * tileWidth) + (tileWidth * 0.5f));
	return Vec2(worldX, worldY);
}


Vec2 Map::GetEntryPosition() const
{
	return GetWorldCoords(m_mapDef->m_entryPointGridCoords);
}


IntVec2 Map::GetMapDimensions() const
{
	return m_mapDef->m_gridDimensions;
}


int Map::GetGridArrayIndex(IntVec2 tileCoords) const
{
	int gridDimensionsX = m_mapDef->m_gridDimensions.x;
	int gridArrayIndex	= tileCoords.x + (tileCoords.y * gridDimensionsX);
	return gridArrayIndex;
}


bool Map::IsPointInSolid(Vec2 const& point, bool treatWaterAsSolid) const
{
	Tile tileAtPoint = GetTile(point);
	return IsTileSolid(tileAtPoint, treatWaterAsSolid);
}


bool Map::IsTileSolid(Tile const& tile, bool treatWaterAsSolid) const
{
	return tile.m_tileDefinition->m_isSolid || (treatWaterAsSolid && tile.m_tileDefinition->m_isWater);
}


RayCastResult Map::RayCast(Vec2 const& startPos, Vec2 const& forwardNormal, float maxDistance, bool treatWaterAsSolid) const
{
	RayCastResult result;
	result.m_forwardNormal = forwardNormal;
	result.m_startPosition = startPos;
	result.m_maxDistance = maxDistance;

	IntVec2 startTileCoords = GetGridCoords(startPos);
	Tile startTile = GetTile(startTileCoords);
	if (IsTileSolid(startTile, treatWaterAsSolid))
	{
		result.m_didImpact = true;
		result.m_impactDistance = 0.0f;
		result.m_impactPosition = startPos;
		result.m_impactFraction = 0.0f;
		result.m_impactSurfaceNormal = -forwardNormal;
		return result;
	}

	Vec2 rayDisplacement = forwardNormal * maxDistance;
	
	float xDeltaT					 = 1.0f / abs(rayDisplacement.x);
	int tileStepX					 = rayDisplacement.x >= 0.0f ? 1 : -1;
	int offsetToLeadingEdgeX		 = (tileStepX + 1) / 2;
	float firstVerticalIntersectionX = static_cast<float>(startTileCoords.x + offsetToLeadingEdgeX);
	float tOfNextXCrossing			 = abs(firstVerticalIntersectionX - startPos.x) * xDeltaT;

	float yDeltaT					 = 1.0f / abs(rayDisplacement.y);
	int tileStepY					 = rayDisplacement.y >= 0.0f ? 1 : -1;
	int offsetToLeadingEdgeY		 = (tileStepY + 1) / 2;
	float firstVerticalIntersectionY = static_cast<float>(startTileCoords.y + offsetToLeadingEdgeY);
	float tOfNextYCrossing			 = abs(firstVerticalIntersectionY - startPos.y) * yDeltaT;

	IntVec2 tileCoords = startTileCoords;
	while (true)
	{
		if (tOfNextXCrossing <= tOfNextYCrossing)
		{
			if (tOfNextXCrossing > 1)
			{
				result.m_didImpact = false;
				return result;
			}

			tileCoords.x += tileStepX;
			Tile tile = GetTile(tileCoords);
			if (IsTileSolid(tile, treatWaterAsSolid))
			{
				result.m_didImpact = true;
				result.m_impactFraction = tOfNextXCrossing;
				result.m_impactDistance = tOfNextXCrossing * maxDistance;
				result.m_impactPosition = startPos + forwardNormal * result.m_impactDistance;
				result.m_impactSurfaceNormal = Vec2((float) -tileStepX, 0.0f);
				return result;
			}

			tOfNextXCrossing += xDeltaT;
		}
		else if (tOfNextYCrossing < tOfNextXCrossing)
		{
			if (tOfNextYCrossing > 1)
			{
				result.m_didImpact = false;
				return result;
			}

			tileCoords.y += tileStepY;
			Tile tile = GetTile(tileCoords);
			if (IsTileSolid(tile, treatWaterAsSolid))
			{
				result.m_didImpact = true;
				result.m_impactFraction = tOfNextYCrossing;
				result.m_impactDistance = tOfNextYCrossing * maxDistance;
				result.m_impactPosition = startPos + forwardNormal * result.m_impactDistance;
				result.m_impactSurfaceNormal = Vec2(0.0f, (float) -tileStepY);
				return result;
			}
			
			tOfNextYCrossing += yDeltaT;
		}
	}
}


bool Map::HasLineOfSight(Vec2 const& startPos, Vec2 const& endPos, float rangeOfVision) const
{
	Vec2 distanceVec = endPos - startPos;
	float distance = distanceVec.NormalizeAndGetPreviousLength();
	if (rangeOfVision > 0.0f && distance > rangeOfVision)
	{
		return false;
	}

	RayCastResult raycastResult = RayCast(startPos, distanceVec, distance, false);
	return !raycastResult.m_didImpact;
}


EntityList const& Map::GetEntitiesByType(EntityType type) const
{
	return m_entityListsByType[type];
}


EntityList const& Map::GetNonBulletEntitiesByFaction(EntityFaction faction) const
{
	return m_tankEntityListByFaction[faction];
}


Tile Map::GetTile(Vec2 const& worldPosition) const
{
	IntVec2 gridCoords = GetGridCoords(worldPosition);
	int tileIndex = GetGridArrayIndex(gridCoords);
	return m_grid[tileIndex];
}


Tile Map::GetTile(IntVec2 const& gridCoords) const
{
	int tileIndex = GetGridArrayIndex(gridCoords);
	return m_grid[tileIndex];
}


void Map::AddEntityToMap(Entity* entity)
{	
	AddEntityToList(entity, m_allEntities);
	AddEntityToList(entity, m_entityListsByType[entity->m_type]);
	if (entity->m_type < TANK_ENUM_COUNT)
	{
		EntityList& tankListByFaction = m_tankEntityListByFaction[entity->m_faction];
		AddEntityToList(entity, tankListByFaction);
		AddEntityToList(entity, m_tankEntities);
	}
	else if(entity->m_type < BULLET_ENUM_COUNT)
	{
		AddEntityToList(entity, m_bulletEntities);
	}
	else if(entity->m_type < EXPLOSIONS_ENUM_COUNT)
	{
		AddEntityToList(entity, m_explosionEntities);
	}
}


Entity* Map::RemoveEntityFromMap(Entity* entity)
{
	if (entity)
	{
		RemoveEntityFromList(entity, m_allEntities);
		RemoveEntityFromList(entity, m_entityListsByType[entity->m_type]);
		if (entity->m_type < TANK_ENUM_COUNT)
		{
			EntityList& nonBulletEntityListByFaction = m_tankEntityListByFaction[entity->m_faction];
			RemoveEntityFromList(entity, nonBulletEntityListByFaction);
			RemoveEntityFromList(entity, m_tankEntities);
		}
		else if (entity->m_type < BULLET_ENUM_COUNT)
		{
			RemoveEntityFromList(entity, m_bulletEntities);
		}
		else if (entity->m_type < EXPLOSIONS_ENUM_COUNT)
		{
			RemoveEntityFromList(entity, m_explosionEntities);
		}

		entity->m_map = nullptr;
		return entity;
	}

	return nullptr;
}


void Map::AddEntityToList(Entity* entity, EntityList& list)
{
	for (int entityIndex = 0; entityIndex < list.size(); entityIndex++)
	{
		Entity*& e = list[entityIndex];
		if (e == nullptr)
		{
			e = entity;
			return;
		}
	}
	list.push_back(entity);
}


void Map::RemoveEntityFromList(Entity* entity, EntityList& list)
{
	auto entityIterator = std::find(list.begin(), list.end(), entity);
	if (entityIterator != list.end())
	{
		list.erase(entityIterator);
	}
}
