#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Player.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include<stdio.h>

std::vector<TileDefinition*> TileDefinition::s_tileDefinitions;
Texture* TileDefinition::s_terrainTexture;
SpriteSheet* TileDefinition::s_terrainSpriteSheet;


TileDefinition::TileDefinition(XmlElement const* xmlElement)
{
	m_name				 = ParseXmlAttribute(*xmlElement, "name",			m_name);
	m_isSolid			 = ParseXmlAttribute(*xmlElement, "isSolid",		m_isSolid);
	m_isWater			 = ParseXmlAttribute(*xmlElement, "isWater",		m_isWater);
	m_tint				 = ParseXmlAttribute(*xmlElement, "tint",			m_tint);
	m_mapImageColor		 = ParseXmlAttribute(*xmlElement, "mapImageColor",	m_mapImageColor);
	IntVec2 spriteCoords = ParseXmlAttribute(*xmlElement, "spriteCoords",	IntVec2(0, 0));
	int spriteIndexInSpriteSheet = spriteCoords.x + spriteCoords.y * 8;
	s_terrainSpriteSheet->GetSpriteUVs(m_uvMins, m_uvMaxs, spriteIndexInSpriteSheet);
}


void TileDefinition::PopulateTileDefinitionsFromXml()
{
	XmlDocument tileDefinitionsDoc;
	XmlError result = tileDefinitionsDoc.LoadFile("Data/DataXmls/TileDefinitions.xml");
	GUARANTEE_OR_DIE(result == XmlError::XML_SUCCESS, "Failed to load tile definitions data file");

	XmlElement* tileDefinitionsRootElement = tileDefinitionsDoc.RootElement();
	GUARANTEE_OR_DIE((tileDefinitionsRootElement && _stricmp(tileDefinitionsRootElement->Name(), "TileDefinitions") == 0), "Unable to find the tile definitions element");

	std::string spriteSheetFilePath = ParseXmlAttribute(*tileDefinitionsRootElement, "spriteSheetPath", "");
	IntVec2 spriteSheetDimensions	= ParseXmlAttribute(*tileDefinitionsRootElement, "spriteSheetDimensions", IntVec2(0, 0));
	s_terrainTexture				= g_theRenderer->CreateOrGetTexture(spriteSheetFilePath.c_str());
	s_terrainSpriteSheet			= new SpriteSheet(*s_terrainTexture, spriteSheetDimensions);

	XmlElement* tileDefinitionElement = tileDefinitionsRootElement->FirstChildElement("TileDefinition");
	while (tileDefinitionElement)
	{
		TileDefinition* tileDef = new TileDefinition(tileDefinitionElement);
		s_tileDefinitions.push_back(tileDef);

		tileDefinitionElement = tileDefinitionElement->NextSiblingElement("TileDefinition");
	}
}


TileDefinition const* TileDefinition::GetTileDefinition(std::string definitionName)
{
	for (int tileDefIndex = 0; tileDefIndex < (int) s_tileDefinitions.size(); tileDefIndex++)
	{
		TileDefinition*& tileDef = s_tileDefinitions[tileDefIndex];
		if (tileDef->m_name == definitionName)
			return tileDef;
	}

	return nullptr;
}


TileDefinition const* TileDefinition::GetTileDefinition(Rgba8 const& tileMapColor)
{
	for (int tileDefIndex = 0; tileDefIndex < (int) s_tileDefinitions.size(); tileDefIndex++)
	{
		TileDefinition*& tileDef = s_tileDefinitions[tileDefIndex];
		if (tileDef->m_mapImageColor.r == tileMapColor.r
			&& tileDef->m_mapImageColor.g == tileMapColor.g
			&& tileDef->m_mapImageColor.b == tileMapColor.b)
		{
			return tileDef;
		}
	}

	return nullptr;
}


void TileDefinition::DeleteAllTileDefinitions()
{
	for (int tileDefIndex = 0; tileDefIndex < (int) s_tileDefinitions.size(); tileDefIndex++)
	{
		delete s_tileDefinitions[tileDefIndex];
		s_tileDefinitions[tileDefIndex] = nullptr;
	}

	s_tileDefinitions.clear();
}


Texture const* TileDefinition::GetTileSpriteSheetTexture()
{
	return s_terrainTexture;
}


std::vector<MapDefinition*> MapDefinition::s_mapDefinitions;

MapDefinition::MapDefinition(XmlElement const* xmlElement)
{
	m_name					= ParseXmlAttribute(*xmlElement, "name",					m_name);
	m_mapImageName			= ParseXmlAttribute(*xmlElement, "mapImageName",			m_mapImageName);
	m_mapImageOffset		= ParseXmlAttribute(*xmlElement, "mapImageOffset",			m_mapImageOffset);
	m_gridDimensions		= ParseXmlAttribute(*xmlElement, "gridDimensions",			m_gridDimensions);
	m_entryPointGridCoords	= ParseXmlAttribute(*xmlElement, "entryPointGridCoords",	m_entryPointGridCoords);
	m_exitPointGridCoords	= ParseXmlAttribute(*xmlElement, "exitPointGridCoords",		m_exitPointGridCoords);
	m_numOfEnemyLeos		= ParseXmlAttribute(*xmlElement, "numOfEnemyLeos",			m_numOfEnemyLeos);
	m_numOfEnemyAries		= ParseXmlAttribute(*xmlElement, "numOfEnemyAries",			m_numOfEnemyAries);
	m_numOfEnemyScorpios	= ParseXmlAttribute(*xmlElement, "numOfEnemyScorpios",		m_numOfEnemyScorpios);
	m_numOfEnemyCapricorns	= ParseXmlAttribute(*xmlElement, "numOfEnemyCapricorns",	m_numOfEnemyCapricorns);
	m_numOfEnemySagittarius = ParseXmlAttribute(*xmlElement, "numOfEnemySagittarius",	m_numOfEnemySagittarius);
	m_borderTileType		= ParseXmlAttribute(*xmlElement, "borderTileType",			m_borderTileType);
	m_fillTileType			= ParseXmlAttribute(*xmlElement, "fillTileType",			m_fillTileType);
	m_startSafeZoneSqLength = ParseXmlAttribute(*xmlElement, "startSafeZoneSqLength",	m_startSafeZoneSqLength);
	m_startSafeZoneTileType = ParseXmlAttribute(*xmlElement, "startSafeZoneTileType",	m_startSafeZoneTileType);
	m_endSafeZoneSqLength	= ParseXmlAttribute(*xmlElement, "endSafeZoneSqLength",		m_endSafeZoneSqLength);
	m_endSafeZoneTileType	= ParseXmlAttribute(*xmlElement, "endSafeZoneTileType",		m_endSafeZoneTileType);
	m_numWorms1				= ParseXmlAttribute(*xmlElement, "numWorms1",				m_numWorms1);
	m_worm1Length			= ParseXmlAttribute(*xmlElement, "worm1Length",				m_worm1Length);
	m_worm1TileType			= ParseXmlAttribute(*xmlElement, "worm1TileType",			m_worm1TileType);
	m_numWorms2				= ParseXmlAttribute(*xmlElement, "numWorms2",				m_numWorms2);
	m_worm2Length			= ParseXmlAttribute(*xmlElement, "worm2Length",				m_worm2Length);
	m_worm2TileType			= ParseXmlAttribute(*xmlElement, "worm2TileType",			m_worm2TileType);
}


void MapDefinition::PopulateMapDefinitionsFromXml()
{
	XmlDocument mapDefinitionsDoc;
	XmlError result = mapDefinitionsDoc.LoadFile("Data/DataXmls/MapDefinitions.xml");
	GUARANTEE_OR_DIE(result == XmlError::XML_SUCCESS, "Failed to load map definitions data file");

	XmlElement* mapDefinitionsRootElement = mapDefinitionsDoc.RootElement();
	GUARANTEE_OR_DIE((mapDefinitionsRootElement && _stricmp(mapDefinitionsRootElement->Name(), "MapDefinitions") == 0), "Unable to find the map definitions element");

	XmlElement* mapDefinitionElement = mapDefinitionsRootElement->FirstChildElement("MapDefinition");
	while (mapDefinitionElement)
	{
		MapDefinition* mapDef = new MapDefinition(mapDefinitionElement);
		s_mapDefinitions.push_back(mapDef);

		mapDefinitionElement = mapDefinitionElement->NextSiblingElement("MapDefinition");
	}
}


MapDefinition const* MapDefinition::GetMapDefinition(std::string definitionName)
{
	for (int mapDefIndex = 0; mapDefIndex < (int) s_mapDefinitions.size(); mapDefIndex++)
	{
		MapDefinition*& mapDef = s_mapDefinitions[mapDefIndex];
		if (mapDef->m_name == definitionName)
			return mapDef;
	}

	return nullptr;
}


void MapDefinition::DeleteAllMapDefinitions()
{
	for (int mapDefIndex = 0; mapDefIndex < (int) s_mapDefinitions.size(); mapDefIndex++)
	{
		delete s_mapDefinitions[mapDefIndex];
		s_mapDefinitions[mapDefIndex] = nullptr;
	}

	s_mapDefinitions.clear();
}


World::World()
{
	TileDefinition::PopulateTileDefinitionsFromXml();
	MapDefinition::PopulateMapDefinitionsFromXml();
	
	Map* map1 = new Map(this, MapDefinition::GetMapDefinition("map1"));
	m_maps.push_back(map1);
	Map* map2 = new Map(this, MapDefinition::GetMapDefinition("map2"));
	m_maps.push_back(map2);
	Map* map3 = new Map(this, MapDefinition::GetMapDefinition("map3"));
	m_maps.push_back(map3);


	m_currentMapIndex = 0;
	Map*& currentMap = m_maps[m_currentMapIndex];
	Vec2 playerStartPosition = currentMap->GetEntryPosition();
	m_player = currentMap->SpawnEntityOfType(EntityType::ENTITY_TYPE_GOOD_PLAYER, playerStartPosition);
}


World::~World()
{

}


void World::CheckInput()
{
	if (g_inputSystem->IsKeyPressed('R'))
	{
		if (m_maps[m_currentMapIndex])
		{
			m_maps[m_currentMapIndex]->GenerateValidMap();
		}
	}
}


void World::Update(float deltaSeconds)
{
	Map*& currentMap = m_maps[m_currentMapIndex];

	CheckInput();

	if (currentMap)
	{
		currentMap->Update(deltaSeconds);

		if (currentMap->HasPlayerReachedExit(m_player))
		{
			if (m_currentMapIndex < m_maps.size() - 1)
			{
				SoundID sfxId = g_audioSystem->CreateOrGetSound("Data/Audio/ExitMap.wav");
				g_audioSystem->StartSound(sfxId, false, 0.5f);
				m_player = currentMap->RemoveEntityFromMap(m_player);
				m_currentMapIndex++;
				Map*& newMap = m_maps[m_currentMapIndex];
				m_player->SetMap(newMap);
				m_player->SetPosition(newMap->GetEntryPosition());
				newMap->AddEntityToMap(m_player);
			}
			else
			{
				g_game->SetGameState(GameState::GAME_STATE_GAME_WON);
			}
		}
	}
}


void World::Render() const
{
	Map* const& currentMap = m_maps[m_currentMapIndex];

	if (currentMap)
	{
		currentMap->Render();
	}
}


void World::DestroyWorld()
{
	for (int mapIndex = 0; mapIndex < m_maps.size(); mapIndex++)
	{
		m_maps[mapIndex]->DeleteMap();
		delete m_maps[mapIndex];
		m_maps[mapIndex] = nullptr;
	}

	m_maps.clear();
	m_maps.resize(0);

	TileDefinition::DeleteAllTileDefinitions();
}


void World::RespawnPlayerAtLastLocation()
{
	if(m_player == nullptr || m_player->IsAlive())
		return;

	int playerHealth = g_gameConfigBlackboard.GetValue("playerHealth", 0);
	m_player->SetLifeStatus(true);
	m_player->SetHealth(playerHealth);
	m_currentMapIndex = Clamp(m_currentMapIndex, 0, (int) m_maps.size() - 1);
	Map*& currentMap = m_maps[m_currentMapIndex];
	m_player->SetPosition(currentMap->GetEntryPosition());
}


Entity* World::GetPlayer() const
{
	if (m_player)
		return m_player;

	return nullptr;
}


IntVec2 World::GetCurrentMapGridDimensions()
{
	Map*& map = m_maps[m_currentMapIndex];
	if (map)
	{
		return map->GetMapDimensions();
	}

	return IntVec2(0, 0);
}


void World::InitialiseMapsFromXML()
{
	
}
