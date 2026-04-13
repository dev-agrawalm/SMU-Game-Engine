#pragma once
#include<vector>
#include <string>
#include "Engine/Core/Image.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec3.hpp"

struct AABB2;
class Map;
class Entity;
class PlayerController;
class GameMode;

struct TileSet;

struct MapDefinition
{
public:
	static void LoadMapDefinitionsFromXml(std::string xmlFilePath);
	static MapDefinition const* GetMapDefinitionByName(std::string defName);
	static void DeleteMapDefinitions();
	static bool DoesMapDefinitionExist(std::string definitionName);

public:
	MapDefinition(XmlElement const& defElement);

public:
	std::string m_name;

	Image m_sourceImage;
	std::string m_spawnInfoXmlPath;
	std::string m_bgMusicPath;

	Vec3 m_tileDims = Vec3(3.0f, 3.0f, 3.0f);
	Vec3 m_origin = Vec3(0.0f, 0.0f, 0.0f);

	TileSet const* m_tileSet = nullptr;

protected:
	static std::vector<MapDefinition*> s_mapDefinitions;
};


class World
{
public:
	void Create();
	void Destroy();
	void Update(float deltaSeconds);
	void Render() const;

	void LoadMap(std::string mapName);
	void SetMapToLoad(std::string mapName);
	AABB2 GetNormalisedViewport(int playerIndex);
	AABB2 GetViewportForNormalisedViewport(AABB2 const& normalisedViewport);
	void CalculateViewportsForPlayers();
	void AddPlayerToGame(int controllerIndex);
	void RemovePlayerFromGame(int controllerIndex);

	void OnGameWon();
	void OnGameLost();

	int GetNumPlayers();
	PlayerController* GetPlayer(int playerIndex);
	bool IsControllerOccupied(int controllerIndex) const;
	Map* GetCurrentMap() const;
	GameMode* GetGameMode() const;
private:
	std::vector<Map*> m_maps;
	Map* m_currentMap = nullptr;
	int m_currentMapIndex = -1;

	int m_numPlayers = 0;
	PlayerController* m_players[4] = {};
	//std::vector<PlayerController*> m_players;

	std::string m_mapToLoad = "";

	GameMode* m_gameMode;
};
