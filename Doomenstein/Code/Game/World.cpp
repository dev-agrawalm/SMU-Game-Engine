#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/PlayerController.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/ActorDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/Weapon.hpp"
#include "Game/GameMode.hpp"
#include "Game/Game.hpp"

void World::Create()
{
	std::string tileMaterialXmlPath		= "Data/XMLData/Definitions/TileMaterials.xml";
	std::string tileDefinitionXmlPath	= "Data/XMLData/Definitions/TileDefinitions.xml";
	std::string tileSetXmlPath			= "Data/XMLData/Definitions/TileSets.xml";
	std::string mapDefinitionXmlPath	= "Data/XMLData/Definitions/MapDefinitions.xml";
	std::string actorDefinitionXmlPath	= "Data/XMLData/Definitions/ActorDefinitions.xml";
	std::string weaponDefinitionXmlPath	= "Data/XMLData/Definitions/WeaponDefinitions.xml";

	TileMaterial	::LoadTileMaterialsFromXML(tileMaterialXmlPath);
	TileDefinition	::LoadTileDefinitionsFromXML(tileDefinitionXmlPath);
	TileSet			::LoadTileSetsFromXML(tileSetXmlPath);
	MapDefinition	::LoadMapDefinitionsFromXml(mapDefinitionXmlPath);
	ProjectileActorDefinitions::LoadProjectileActorDefinitionsFromXML(weaponDefinitionXmlPath);
	WeaponDefinition::LoadWeaponDefinitionsFromXML(weaponDefinitionXmlPath);
	ActorDefinition	::LoadActorDefinitionsFromXMLFile(actorDefinitionXmlPath);

	std::string defaultMap = g_gameConfigBlackboard.GetValue("defaultMap", "");
	MapDefinition const* mapDefinition = MapDefinition::GetMapDefinitionByName(defaultMap);
	ASSERT_OR_DIE(mapDefinition != nullptr, "Invalid map name given in blackboard: " + defaultMap);
	LoadMap(defaultMap);

	m_gameMode = new GameMode(this);
	m_gameMode->Init();
}


void World::Destroy()
{
	for (int mapIndex = 0; mapIndex < (int) m_maps.size(); mapIndex++)
	{
		Map*& map = m_maps[mapIndex];
		if (map)
		{
			map->Destroy();
			delete map;
			map = nullptr;
		}
	}

	for (int playerIndex = 0; playerIndex < 4; playerIndex++)
	{
		if (m_players[playerIndex])
		{
			m_players[playerIndex]->PossessActor(nullptr);
			delete m_players[playerIndex];
			m_players[playerIndex] = nullptr;
		}
	}

	WeaponDefinition::DeleteAllWeaponDefinitions();
	ProjectileActorDefinitions::DeleteAllProjectileActorDefinitions();
	ActorDefinition	::DeleteAllActorDefinitions();
	MapDefinition	::DeleteMapDefinitions();
	TileSet			::DeleteTileSets();
	TileDefinition	::DeleteTileDefinitions();
	TileMaterial	::DeleteTileMaterials();
}


void World::Update(float deltaSeconds)
{
	if (m_gameMode->IsTimerPaused())
		m_gameMode->ResumeTimer();

	for (int playerIndex = 0; playerIndex < m_numPlayers; playerIndex++)
	{
		PlayerController*& player = m_players[playerIndex];
		if (player)
			player->PreMapUpdate(deltaSeconds);
	}

	if (m_currentMap)
		m_currentMap->Update(deltaSeconds);

	for (int playerIndex = 0; playerIndex < m_numPlayers; playerIndex++)
	{
		PlayerController*& player = m_players[playerIndex];
		if (player)
			player->PostMapUpdate(deltaSeconds);
	}

	m_gameMode->Update();
}


void World::Render() const
{
	g_theRenderer->ClearScreen(Rgba8::BLACK);	
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	for (int playerIndex = 0; playerIndex < 4; playerIndex++)
	{
		PlayerController* const& player = m_players[playerIndex];
		if (player && m_currentMap)
		{
			Camera const& playerWorldCam = player->GetWorldCamera();
			g_theRenderer->BeginCamera(playerWorldCam);
			{
				Texture* defaultDepthStencil = g_theRenderer->GetDefaultDepthStencil();
				g_theRenderer->ClearDepth(defaultDepthStencil, 1.0f, 0);
				g_theRenderer->SetDepthOptions(DepthTest::LESS_EQUAL, true);
				m_currentMap->Render();
			}
			g_theRenderer->EndCamera(playerWorldCam);

			Camera const& playerScreenCamera = player->GetScreenCamera();
			g_theRenderer->BeginCamera(playerScreenCamera);
			{
				player->RenderHUD();
			}
			g_theRenderer->EndCamera(playerScreenCamera);
		}
	}

	if (m_players[0])
		DebugRenderWorldToCamera(m_players[0]->GetWorldCamera());
}


void World::LoadMap(std::string mapName)
{
	if (m_currentMap)
	{
		MapDefinition const* currentMapDef = m_currentMap->GetMapDefinition();
		if (currentMapDef->m_name == mapName)
		{
			g_console->AddLine(DevConsole::MAJOR_INFO, "Map already loaded");
			return;
		}
	}

	for (int mapIndex = 0; mapIndex < m_maps.size(); mapIndex++)
	{
		MapDefinition const* mapDef = m_maps[mapIndex]->GetMapDefinition();
		if (mapDef->m_name == mapName )
		{
			m_currentMapIndex = mapIndex;
			m_currentMap = m_maps[mapIndex];
			g_console->AddLine(DevConsole::MINOR_INFO, "Map Loaded: " + mapName);
			return;
		}
	}

	MapDefinition const* mapDef = MapDefinition::GetMapDefinitionByName(mapName);
	if (mapDef)
	{
		Map* map = new Map(this);
		m_maps.push_back(map);
		m_currentMap = map;
		m_currentMapIndex = (int) m_maps.size() - 1;
		map->Create(mapDef);
		g_console->AddLine(DevConsole::MINOR_INFO, "Map Loaded: " + mapName);
		return;
	}

	g_console->AddLine(DevConsole::MAJOR_INFO, "Invalid Map: " + mapName);
}


void World::SetMapToLoad(std::string mapName)
{
	m_mapToLoad = mapName;
}


AABB2 World::GetNormalisedViewport(int playerIndex)
{
	AABB2 normalisedViewport;
	normalisedViewport.m_mins.x = 0.0f;
	normalisedViewport.m_maxs.x = 1.0f;
	normalisedViewport.m_mins.y = (float) playerIndex / m_numPlayers;
	normalisedViewport.m_maxs.y = ClampZeroToOne((float) (playerIndex + 1) / m_numPlayers);
	return normalisedViewport;
}


AABB2 World::GetViewportForNormalisedViewport(AABB2 const& normalisedViewport)
{
	AABB2 viewport;
	Vec2 clientDims = g_window->GetWindowDims();
	viewport.m_mins = normalisedViewport.m_mins * clientDims;
	viewport.m_maxs = normalisedViewport.m_maxs * clientDims;
	return viewport;
}


void World::CalculateViewportsForPlayers()
{
	for (int playerIndex = 0; playerIndex < m_numPlayers; playerIndex++)
	{
		PlayerController* player = m_players[playerIndex];
		if (player)
		{
			AABB2 normalisedViewport = GetNormalisedViewport(playerIndex);
			AABB2 viewport = GetViewportForNormalisedViewport(normalisedViewport);
			player->SetNormalisedViewport(normalisedViewport);
			player->SetWorldCameraViewport(viewport);
			player->SetScreenCameraViewport(viewport);
		}
	}
}


void World::AddPlayerToGame(int controllerIndex)
{
	if (m_numPlayers == 4)
		return;

	for (int playerIndex = 0; playerIndex < 4; playerIndex++)
	{
		if (m_players[playerIndex] == nullptr)
		{
			m_players[playerIndex] = new PlayerController(this, playerIndex, controllerIndex);
			m_gameMode->SpawnActorForPlayer(m_players[playerIndex]);
			//m_players[playerIndex]->SpawnAndPossessRandomActorFromSpawnPoint();
			m_numPlayers++;
			break;
		}
	}
	
	CalculateViewportsForPlayers();
}


void World::RemovePlayerFromGame(int controllerIndex)
{
	if (m_numPlayers == 0)
		return;

	for (int playerItr = 0; playerItr < m_numPlayers; playerItr++)
	{
		PlayerController* player = m_players[playerItr];
		if (player && player->GetControllerIndex() == controllerIndex)
		{
			player->PossessActor(nullptr);
			delete m_players[playerItr];
			m_players[playerItr] = m_players[m_numPlayers - 1];
			m_players[m_numPlayers - 1] = nullptr;
			m_numPlayers--;
			break;
		}
	}

	CalculateViewportsForPlayers();
}


void World::OnGameWon()
{
	Destroy();
	g_game->SetGameState(GameState::GAME_STATE_GAME_WON);
}


void World::OnGameLost()
{
	Destroy();
	g_game->SetGameState(GameState::GAME_STATE_GAME_OVER);
}


int World::GetNumPlayers()
{
	return m_numPlayers;
}


PlayerController* World::GetPlayer(int playerIndex)
{
	if (playerIndex < 0 || playerIndex >= 4)
		return nullptr;
	return m_players[playerIndex];
}


bool World::IsControllerOccupied(int controllerIndex) const
{
	for (int playerIndex = 0; playerIndex < 4; playerIndex++)
	{
		PlayerController const* player = m_players[playerIndex];
		if (player)
		{
			int controller = player->GetControllerIndex();
			if (controller == controllerIndex)
				return true;
		}
	}

	return false;
}


Map* World::GetCurrentMap() const
{
	return m_currentMap;
}


GameMode* World::GetGameMode() const
{
	return m_gameMode;
}


std::vector<MapDefinition*> MapDefinition::s_mapDefinitions;

void MapDefinition::LoadMapDefinitionsFromXml(std::string xmlFilePath)
{
	XmlDocument mapDefDoc;
	XmlError result = mapDefDoc.LoadFile(xmlFilePath.c_str());
	GUARANTEE_OR_DIE(result == 0, "Map definition xml file path invalid");

	XmlElement* mapDefRootElement = mapDefDoc.RootElement();
	GUARANTEE_OR_DIE(mapDefRootElement && _stricmp(mapDefRootElement->Name(), "MapDefinitions") == 0, "Unable to find definitions element in map definition xml");

	XmlElement const* mapDefElement = mapDefRootElement->FirstChildElement("MapDefinition");
	while (mapDefElement)
	{
		MapDefinition* mapDefinition = new MapDefinition(*mapDefElement);
		ASSERT_OR_DIE(!DoesMapDefinitionExist(mapDefinition->m_name), "Duplicate Data is Map Definitions");
		s_mapDefinitions.push_back(mapDefinition);

		mapDefElement = mapDefElement->NextSiblingElement("MapDefinition");
	}
}


MapDefinition const* MapDefinition::GetMapDefinitionByName(std::string defName)
{
	for (int defIndex = 0; defIndex < s_mapDefinitions.size(); defIndex++)
	{
		if (s_mapDefinitions[defIndex]->m_name == defName)
		{
			return s_mapDefinitions[defIndex];
		}
	}

	return nullptr;
}


void MapDefinition::DeleteMapDefinitions()
{
	for (int defIndex = 0; defIndex < s_mapDefinitions.size(); defIndex++)
	{
		delete s_mapDefinitions[defIndex];
		s_mapDefinitions[defIndex] = nullptr;
	}

	s_mapDefinitions.clear();
}


bool MapDefinition::DoesMapDefinitionExist(std::string definitionName)
{
	for (int defIndex = 0; defIndex < s_mapDefinitions.size(); defIndex++)
	{
		if (s_mapDefinitions[defIndex]->m_name == definitionName)
		{
			return true;
		}
	}

	return false;
}


MapDefinition::MapDefinition(XmlElement const& defElement)
{
	m_name = ParseXmlAttribute(defElement, "name", "TestMap");

	std::string imagePath = ParseXmlAttribute(defElement, "image", "");
	m_sourceImage = Image(imagePath.c_str());

	std::string tileSetName = ParseXmlAttribute(defElement, "tileSet", "Default");
	m_tileSet = TileSet::GetTileSetByName(tileSetName);
	ASSERT_OR_DIE(m_tileSet != nullptr, "Invalid tile set for map definition: " + m_name);

	m_tileDims = ParseXmlAttribute(defElement, "tileSize", m_tileDims);

	m_spawnInfoXmlPath = ParseXmlAttribute(defElement, "spawns", m_spawnInfoXmlPath);
	m_bgMusicPath = ParseXmlAttribute(defElement, "music", m_bgMusicPath);
}
