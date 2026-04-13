#pragma once
#include<vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/Tile.hpp"

class	Map;
class	Entity;
class	Texture;
class	SpriteSheet;
struct	IntVec2;

struct TileDefinition
{
public:
	static void PopulateTileDefinitionsFromXml();
	static TileDefinition const* GetTileDefinition(std::string definitionName);
	static TileDefinition const* GetTileDefinition(Rgba8 const& tileMapColor);
	static void DeleteAllTileDefinitions();
	static Texture const* GetTileSpriteSheetTexture();

public:
	TileDefinition(XmlElement const* xmlElement);

public:
	std::string m_name	= "";
	bool m_isSolid		= false;
	bool m_isWater		= false;
	Vec2 m_uvMins		= Vec2::ZERO;
	Vec2 m_uvMaxs		= Vec2::ONE;
	Rgba8 m_tint		= Rgba8::WHITE;
	Rgba8 m_mapImageColor = Rgba8::BLACK_TRANSPARENT;

protected:
	static std::vector<TileDefinition*> s_tileDefinitions;
	static Texture*	s_terrainTexture;
	static SpriteSheet* s_terrainSpriteSheet;
};


struct MapDefinition
{
public:
	static void PopulateMapDefinitionsFromXml();
	static MapDefinition const* GetMapDefinition(std::string definitionName);
	static void DeleteAllMapDefinitions();

public:
	MapDefinition(XmlElement const* xmlElement);

public:
	std::string m_name = "";
	std::string m_mapImageName = "";
	IntVec2  m_mapImageOffset = IntVec2(0, 0);
	IntVec2	 m_gridDimensions = IntVec2(0, 0);
	IntVec2	 m_entryPointGridCoords = IntVec2(0, 0);
	IntVec2	 m_exitPointGridCoords = IntVec2(0, 0);

	int	m_numOfEnemyLeos = 0;
	int	m_numOfEnemyAries = 0;
	int	m_numOfEnemyScorpios = 0;
	int	m_numOfEnemyCapricorns = 0;
	int	m_numOfEnemySagittarius = 0;

	std::string m_borderTileType = "";
	std::string m_fillTileType = "";

	int	m_startSafeZoneSqLength = 0;
	std::string m_startSafeZoneTileType = "";

	int	m_endSafeZoneSqLength = 0;
	std::string m_endSafeZoneTileType = "";

	int	m_numWorms1 = 0;
	int	m_worm1Length = 0;
	std::string m_worm1TileType = "";

	int	m_numWorms2 = 0;
	int	m_worm2Length = 0;
	std::string m_worm2TileType = "";

protected:
	static std::vector<MapDefinition*> s_mapDefinitions;
};


class World
{
public:
	World();
	~World();

	void	CheckInput();
	void	Update(float deltaSeconds);
	void	Render() const;
	void	DestroyWorld();
	void	RespawnPlayerAtLastLocation();

	//getters
	Entity* GetPlayer() const;
	IntVec2 GetCurrentMapGridDimensions();

private:
	void InitialiseMapsFromXML();

private:
	std::vector<Map*> m_maps;
	int m_currentMapIndex;
	Entity*	m_player = nullptr;
};
