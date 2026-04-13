#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::vector<TileMaterial*> TileMaterial::s_tileMaterials;

void TileMaterial::LoadTileMaterialsFromXML(std::string xmlFilePath)
{
	XmlDocument tileMatDoc;
	XmlError result = tileMatDoc.LoadFile(xmlFilePath.c_str());
	GUARANTEE_OR_DIE(result == XmlError::XML_SUCCESS, "Tile Material xml file path invalid");

	XmlElement* tileMatRootElement = tileMatDoc.RootElement();
	GUARANTEE_OR_DIE(tileMatRootElement && _stricmp(tileMatRootElement->Name(), "MaterialDefinitions") == 0, "Unable to find definitions element in tile material xml");

	XmlElement const* tileMatElement = tileMatRootElement->FirstChildElement("TileMaterial");
	while (tileMatElement)
	{
		TileMaterial* material = new TileMaterial(*tileMatElement);
		ASSERT_OR_DIE(!DoesMaterialExist(material->m_name), "Duplicate Data in Tile Materials");

		s_tileMaterials.push_back(material);

		tileMatElement = tileMatElement->NextSiblingElement("TileMaterial");
	}
}


TileMaterial* TileMaterial::GetTileMaterialByName(std::string matName)
{
	for (int matIndex = 0; matIndex < s_tileMaterials.size(); matIndex++)
	{
		if (s_tileMaterials[matIndex]->m_name == matName)
		{
			return s_tileMaterials[matIndex];
		}
	}

	return nullptr;
}


void TileMaterial::DeleteTileMaterials()
{
	for (int tileMatIndex = 0; tileMatIndex < s_tileMaterials.size(); tileMatIndex++)
	{
		delete s_tileMaterials[tileMatIndex];
		s_tileMaterials[tileMatIndex] = nullptr;
	}

	s_tileMaterials.clear();
}


bool TileMaterial::DoesMaterialExist(std::string materialName)
{
	for (int matIndex = 0; matIndex < s_tileMaterials.size(); matIndex++)
	{
		if (s_tileMaterials[matIndex]->m_name == materialName)
		{
			return true;
		}
	}

	return false;
}


TileMaterial::TileMaterial(XmlElement const& matElement)
{
	m_name = ParseXmlAttribute(matElement, "name", "None");

	std::string shaderName = ParseXmlAttribute(matElement, "shader", "Default");
	m_shader = g_theRenderer->CreateOrGetShader(shaderName.c_str());

	std::string diffuseTextureName = ParseXmlAttribute(matElement, "diffuse", "Default");
	m_diffuse = g_theRenderer->CreateOrGetTexture(diffuseTextureName.c_str());

	m_tint = ParseXmlAttribute(matElement, "tint", m_tint);

	IntVec2 spriteSheetCellDims = ParseXmlAttribute(matElement, "cellCount", IntVec2(1, 1));
	IntVec2 spriteCellPos = ParseXmlAttribute(matElement, "cell", IntVec2(0, 0));
	SpriteSheet spriteSheet = SpriteSheet(*m_diffuse, spriteSheetCellDims);
	int spriteIndexInSpriteSheet = spriteCellPos.x + spriteCellPos.y * spriteSheetCellDims.x;
	spriteSheet.GetSpriteUVs(m_uvs.m_mins, m_uvs.m_maxs, spriteIndexInSpriteSheet);

	m_isVisible = ParseXmlAttribute(matElement, "isVisible", m_isVisible);
}


std::vector<TileDefinition*> TileDefinition::s_tileDefinitions;

void TileDefinition::LoadTileDefinitionsFromXML(std::string xmlFilePath)
{
	XmlDocument tileDefDoc;
	XmlError result = tileDefDoc.LoadFile(xmlFilePath.c_str());
	GUARANTEE_OR_DIE(result == XmlError::XML_SUCCESS, "Tile Definition xml file path invalid");

	XmlElement* tileDefRootElement = tileDefDoc.RootElement();
	GUARANTEE_OR_DIE(tileDefRootElement && _stricmp(tileDefRootElement->Name(), "TileDefinitions") == 0, "Unable to find definitions element in tile definition xml");

	XmlElement const* tileDefElement = tileDefRootElement->FirstChildElement("TileDefinition");
	while (tileDefElement)
	{
		TileDefinition* definition = new TileDefinition(*tileDefElement);
		ASSERT_OR_DIE(!DoesDefinitionExist(definition->m_name), "Duplicate Data in Tile Definitions");

		s_tileDefinitions.push_back(definition);

		tileDefElement = tileDefElement->NextSiblingElement("TileDefinition");
	}
}


TileDefinition* TileDefinition::GetTileDefinitionByName(std::string defName)
{
	for (int defIndex = 0; defIndex < s_tileDefinitions.size(); defIndex++)
	{
		if (s_tileDefinitions[defIndex]->m_name == defName)
		{
			return s_tileDefinitions[defIndex];
		}
	}

	return nullptr;
}


void TileDefinition::DeleteTileDefinitions()
{
	for (int tileDefIndex = 0; tileDefIndex < s_tileDefinitions.size(); tileDefIndex++)
	{
		delete s_tileDefinitions[tileDefIndex];
		s_tileDefinitions[tileDefIndex] = nullptr;
	}

	s_tileDefinitions.clear();
}


bool TileDefinition::DoesDefinitionExist(std::string definitionName)
{
	for (int defIndex = 0; defIndex < s_tileDefinitions.size(); defIndex++)
	{
		if (s_tileDefinitions[defIndex]->m_name == definitionName)
		{
			return true;
		}
	}

	return false;
}


TileDefinition::TileDefinition(XmlElement const& defElement)
{
	m_name = ParseXmlAttribute(defElement, "name", "None");
	
	std::string ceilingMaterialName = ParseXmlAttribute(defElement, "ceilingMaterial", "None");
	m_ceilingMaterial = TileMaterial::GetTileMaterialByName(ceilingMaterialName);
	ASSERT_OR_DIE(m_ceilingMaterial != nullptr, "Invalid Ceiling Material Name for tile definition: " + m_name);

	std::string floorMaterialName = ParseXmlAttribute(defElement, "floorMaterial", "None");
	m_floorMaterial = TileMaterial::GetTileMaterialByName(floorMaterialName);
	ASSERT_OR_DIE(m_floorMaterial != nullptr, "Invalid Floor Material Name for tile definition: " + m_name);

	std::string wallMaterialName = ParseXmlAttribute(defElement, "wallMaterial", "None");
	m_wallMaterial = TileMaterial::GetTileMaterialByName(wallMaterialName);
	ASSERT_OR_DIE(m_wallMaterial != nullptr, "Invalid Wall Material Name for tile definition: " + m_name);

	m_isSolid = ParseXmlAttribute(defElement, "isSolid", m_isSolid);
}



TileMapping::TileMapping(XmlElement const& mappingElement)
{
	m_color = ParseXmlAttribute(mappingElement, "color", m_color);

	std::string tileDefName = "None";
	tileDefName = ParseXmlAttribute(mappingElement, "tile", tileDefName);
	m_definition = TileDefinition::GetTileDefinitionByName(tileDefName);
}


std::vector<TileSet*> TileSet::s_tileSets;

void TileSet::LoadTileSetsFromXML(std::string xmlFilePath)
{
	XmlDocument tileSetDoc;
	XmlError result = tileSetDoc.LoadFile(xmlFilePath.c_str());
	GUARANTEE_OR_DIE(result == XmlError::XML_SUCCESS, "Tile Set xml file path invalid");

	XmlElement* tileSetRootElement = tileSetDoc.RootElement();
	GUARANTEE_OR_DIE(tileSetRootElement && _stricmp(tileSetRootElement->Name(), "TileSets") == 0, "Unable to find definitions element in tile set xml");

	XmlElement const* tileSetElement = tileSetRootElement->FirstChildElement("TileSet");
	while (tileSetElement)
	{
		TileSet* definition = new TileSet(*tileSetElement);
		ASSERT_OR_DIE(!DoesSetExist(definition->m_name), "Duplicate Data in Tile Sets");
		s_tileSets.push_back(definition);

		tileSetElement = tileSetElement->NextSiblingElement("TileSet");
	}
}


TileSet* TileSet::GetTileSetByName(std::string setName)
{
	for (int setIndex = 0; setIndex < s_tileSets.size(); setIndex++)
	{
		if (s_tileSets[setIndex]->m_name == setName)
		{
			return s_tileSets[setIndex];
		}
	}

	return nullptr;
}


void TileSet::DeleteTileSets()
{
	for (int tileSetIndex = 0; tileSetIndex < s_tileSets.size(); tileSetIndex++)
	{
		s_tileSets[tileSetIndex]->m_tileMappings.clear();
		delete s_tileSets[tileSetIndex];
		s_tileSets[tileSetIndex] = nullptr;
	}

	s_tileSets.clear();
}


bool TileSet::DoesSetExist(std::string setName)
{
	for (int setIndex = 0; setIndex < s_tileSets.size(); setIndex++)
	{
		if (s_tileSets[setIndex]->m_name == setName)
		{
			return true;
		}
	}

	return false;
}


TileSet::TileSet(XmlElement const& setElement)
{
	m_name = ParseXmlAttribute(setElement, "name", "Default");

	XmlElement const* tileMappingElement = setElement.FirstChildElement("TileMapping");
	while (tileMappingElement)
	{
		TileMapping tileMapping = TileMapping(*tileMappingElement);
		ASSERT_OR_DIE(!DoesMappingExist(tileMapping.m_color), "Duplicate tile mapping in tile set definition. Tile Set: " + m_name);

		m_tileMappings.push_back(tileMapping);

		tileMappingElement = tileMappingElement->NextSiblingElement("TileMapping");
	}

	std::string defaultTileName = ParseXmlAttribute(setElement, "defaultTile", "Default");
	m_defaultTile = TileDefinition::GetTileDefinitionByName(defaultTileName);
	ASSERT_OR_DIE(m_defaultTile != nullptr, "Invalid default tile for tile set(name): " + m_name);
}


TileDefinition const* TileSet::GetTileDefinition(Rgba8 const& color) const
{
	for (int tileMapIndex = 0; tileMapIndex < m_tileMappings.size(); tileMapIndex++)
	{
		TileMapping const& tileMapping = m_tileMappings[tileMapIndex];
		if (tileMapping.m_color == color)
		{
			return tileMapping.m_definition;
		}
	}

	return m_defaultTile;
}


bool TileSet::DoesMappingExist(Rgba8 const& color)
{
	for (int mappingIndex = 0; mappingIndex < m_tileMappings.size(); mappingIndex++)
	{
		Rgba8 mappingColor = m_tileMappings[mappingIndex].m_color;
		if (mappingColor == color)
			return true;
	}

	return false;
}
