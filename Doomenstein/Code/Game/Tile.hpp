#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include<string>
#include <vector>
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"

class Shader;
class Texture;

struct TileMaterial
{
public:
	static void LoadTileMaterialsFromXML(std::string xmlFilePath);
	static TileMaterial* GetTileMaterialByName(std::string matName);
	static void DeleteTileMaterials();
	static bool DoesMaterialExist(std::string materialName);

public:
	TileMaterial(XmlElement const& matElement);

public:
	std::string m_name;

	Shader const* m_shader = nullptr;
	Texture const* m_diffuse = nullptr;
	Rgba8 m_tint = Rgba8::WHITE;
	AABB2 m_uvs = AABB2::ZERO_TO_ONE;
	
	bool m_isVisible = true;

protected:
	static std::vector<TileMaterial*> s_tileMaterials;
};


struct TileDefinition
{
public:
	static void LoadTileDefinitionsFromXML(std::string xmlFilePath);
	static TileDefinition* GetTileDefinitionByName(std::string defName);
	static void DeleteTileDefinitions();
	static bool DoesDefinitionExist(std::string definitionName);

public:
	TileDefinition(XmlElement const& defElement);

public:
	std::string m_name;

	TileMaterial const* m_wallMaterial = nullptr;
	TileMaterial const* m_floorMaterial = nullptr;
	TileMaterial const* m_ceilingMaterial = nullptr;

	bool m_isSolid = false;

protected:
	static std::vector<TileDefinition*> s_tileDefinitions;
};


struct TileMapping
{
public:
	TileMapping(XmlElement const& mappingElement);

public:
	TileDefinition const* m_definition = nullptr;
	Rgba8 m_color;
};


struct TileSet
{
public:
	static void LoadTileSetsFromXML(std::string xmlFilePath);
	static TileSet* GetTileSetByName(std::string setName);
	static void DeleteTileSets();
	static bool DoesSetExist(std::string setName);

public:
	TileSet(XmlElement const& setElement);
	TileDefinition const* GetTileDefinition(Rgba8 const& color) const;

private:
	bool DoesMappingExist(Rgba8 const& color);

public:
	std::string m_name;
	TileDefinition const* m_defaultTile = nullptr;

protected:
	std::vector<TileMapping> m_tileMappings;

protected:
	static std::vector<TileSet*> s_tileSets;
};


struct Tile
{
public:
	bool IsSolid() const { return m_tileDef->m_isSolid; }

public:
	TileDefinition const* m_tileDef = nullptr;

	AABB3 m_worldBounds;
	IntVec2 m_gridCoords;

	bool m_hasFloor = false;
};