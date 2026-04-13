#include "Game/Block.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::vector<BlockDef> BlockDef::s_blockDefinitions;
SpriteSheet const* BlockDef::s_spriteSheet;

void BlockDef::GenerateAllBlockDefs()
{
	s_blockDefinitions.reserve(sizeof(BlockDef) * 64);

	XmlDocument blockDefDoc;
	XmlError result = blockDefDoc.LoadFile("Data/XMLData/BlockDefinitions.xml");
	GUARANTEE_OR_DIE(result == XmlError::XML_SUCCESS, "Unable to load block definitions file");

	XmlElement* rootElement = blockDefDoc.RootElement();
	GUARANTEE_OR_DIE(_stricmp(rootElement->Name(), "BlockDefinitions") == 0, "Block definitions file does not have BlockDefinitions element");

	std::string spriteSheetTextureName = ParseXmlAttribute(*rootElement, "spriteSheet", "");
	IntVec2 spriteSheetCellCount = ParseXmlAttribute(*rootElement, "cellCount", IntVec2(0, 0));
	Texture* spriteSheetTexture = g_theRenderer->CreateOrGetTexture(spriteSheetTextureName.c_str());
	s_spriteSheet = new SpriteSheet(*spriteSheetTexture, spriteSheetCellCount);

	XmlElement const* blockDefinitionElement = rootElement->FirstChildElement("Definition");
	while (blockDefinitionElement)
	{
		BlockDef blockDef = BlockDef(*blockDefinitionElement);
		s_blockDefinitions.push_back(blockDef);
		
		blockDefinitionElement = blockDefinitionElement->NextSiblingElement("Definition");
	}
}


BlockDef const* BlockDef::GetBlockDefByName(std::string blockDefName)
{
	for (int defIndex = 0; defIndex < (int) s_blockDefinitions.size(); defIndex++)
	{
		BlockDef const& definition = s_blockDefinitions[defIndex];
		if (definition.m_name == blockDefName)
		{
			return &definition;
		}
	}

	return nullptr;
}


uint8_t BlockDef::GetBlockDefIdByName(std::string blockDefName)
{
	for (int defIndex = 0; defIndex < (int) s_blockDefinitions.size(); defIndex++)
	{
		BlockDef const& definition = s_blockDefinitions[defIndex];
		if (definition.m_name == blockDefName)
		{
			return (uint8_t) defIndex;
		}
	}

	return (uint8_t) -1;
}


BlockDef const* BlockDef::GetBlockDefById(uint8_t blockDefId)
{
	if (blockDefId >= 0 && blockDefId < (int) s_blockDefinitions.size())
		return &s_blockDefinitions[blockDefId];

	return nullptr;
}


Texture const& BlockDef::GetTexture()
{
	return s_spriteSheet->GetTexture();
}


BlockDef::BlockDef(XmlElement const& element)
{
	m_name			= ParseXmlAttribute(element, "name",		m_name);
	m_isOpaque		= ParseXmlAttribute(element, "isOpaque",	m_isOpaque);
	m_isSolid		= ParseXmlAttribute(element, "isSolid",		m_isSolid);;
	m_isRendered	= ParseXmlAttribute(element, "isRendered",	m_isRendered);;
	
	IntVec2 topSpriteCoords		= ParseXmlAttribute(element, "topSpriteCoords",		IntVec2(0,0));
	IntVec2 sideSpriteCoords	= ParseXmlAttribute(element, "sideSpriteCoords",	IntVec2(0, 0));
	IntVec2 bottomSpriteCoords	= ParseXmlAttribute(element, "bottomSpriteCoords",	IntVec2(0, 0));
	s_spriteSheet->GetSpriteUVs(m_topUVs.m_mins,	m_topUVs.m_maxs,	topSpriteCoords);
	s_spriteSheet->GetSpriteUVs(m_bottomUVs.m_mins, m_bottomUVs.m_maxs, bottomSpriteCoords);
	s_spriteSheet->GetSpriteUVs(m_sideUVs.m_mins,	m_sideUVs.m_maxs,	sideSpriteCoords);

	m_internalOutdoorLight = ParseXmlAttribute(element, "internalOutdoorLight", m_internalOutdoorLight);
	m_internalIndoorLight = ParseXmlAttribute(element, "internalIndoorLight", m_internalIndoorLight);
}


BlockDef::BlockDef()
{

}


void Block::SetDirtyLightFlag(bool isDirty)
{
	if (isDirty)
	{
		m_blockFlags |= BLOCK_BIT_FLAG_DIRTY_LIGHT;
	}
	else
	{
		m_blockFlags &= ~BLOCK_BIT_FLAG_DIRTY_LIGHT;
	}
}


void Block::SetSkyFlag(bool isSky)
{
	if (isSky)
	{
		m_blockFlags |= BLOCK_BIT_FLAG_IS_SKY;
	}
	else
	{
		m_blockFlags &= !BLOCK_BIT_FLAG_IS_SKY;
	}
}


void Block::SetIndoorLightExposure(int indoorLight)
{
	int outdoorLightExposure = m_lightExposure & BLOCK_OUTDOOR_LIGHT_MASK;
	m_lightExposure = uint8_t(outdoorLightExposure | indoorLight);
}


void Block::SetOutdoorLightExposure(int outdoorLight)
{
	int indoorLightExposure = m_lightExposure & BLOCK_INDOOR_LIGHT_MASK;
	m_lightExposure = uint8_t((outdoorLight << 4) | indoorLightExposure);
}


int Block::GetIndoorLightExposure() const
{
	return m_lightExposure & BLOCK_INDOOR_LIGHT_MASK;
}


int Block::GetOutdoorLightExposure() const
{
	return (m_lightExposure & BLOCK_OUTDOOR_LIGHT_MASK) >> BLOCK_BITS_PER_LIGHT;
}


int Block::GetInternalIndoorLight() const
{
	BlockDef const* blockDef = BlockDef::GetBlockDefById(m_blockDefId);
	return blockDef->m_internalIndoorLight;
}


int Block::GetInternalOutdoorLight() const
{
	BlockDef const* blockDef = BlockDef::GetBlockDefById(m_blockDefId);
	return blockDef->m_internalOutdoorLight;
}


bool Block::IsLightDirty() const
{
	return (m_blockFlags & BLOCK_BIT_FLAG_DIRTY_LIGHT) == BLOCK_BIT_FLAG_DIRTY_LIGHT;
}


bool Block::IsSky() const
{
	return (m_blockFlags & BLOCK_BIT_FLAG_IS_SKY) == BLOCK_BIT_FLAG_IS_SKY;
}


bool Block::IsOpaque() const
{
	BlockDef const* blockDef = BlockDef::GetBlockDefById(m_blockDefId);
	return blockDef->m_isOpaque;
}


bool Block::IsSolid() const
{
	BlockDef const* blockDef = BlockDef::GetBlockDefById(m_blockDefId);
	return blockDef->m_isSolid;
}


bool Block::EmitsLight() const
{
	BlockDef const* blockDef = BlockDef::GetBlockDefById(m_blockDefId);
	return blockDef->m_internalIndoorLight > 0 || blockDef->m_internalOutdoorLight > 0;
}
