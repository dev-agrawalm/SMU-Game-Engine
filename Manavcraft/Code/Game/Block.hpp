#pragma once
#include <string>
#include <vector>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

struct BlockDef
{
public:
	static void GenerateAllBlockDefs();
	static BlockDef const* GetBlockDefByName(std::string blockDefName);
	static uint8_t GetBlockDefIdByName(std::string blockDefName);
	static BlockDef const* GetBlockDefById(uint8_t blockDefId);
	static Texture const& GetTexture();

public:
	BlockDef(XmlElement const& element);
	BlockDef();

public:
	std::string m_name;
	bool m_isRendered = false;
	bool m_isSolid = false;
	bool m_isOpaque = false;
	AABB2 m_topUVs = AABB2::ZERO_TO_ONE;
	AABB2 m_bottomUVs = AABB2::ZERO_TO_ONE;
	AABB2 m_sideUVs = AABB2::ZERO_TO_ONE;
	int m_internalIndoorLight = 0;
	int m_internalOutdoorLight = 0;

private:
	static std::vector<BlockDef> s_blockDefinitions;
	static SpriteSheet const* s_spriteSheet;
};


enum BlockBitFlags : uint8_t
{
	BLOCK_BIT_FLAG_NONE					= 0,
	BLOCK_BIT_FLAG_DIRTY_LIGHT			= 1 << 0,
	BLOCK_BIT_FLAG_IS_SKY				= 1 << 1,
	//BLOCK_BIT_FLAG_DIRTY_LIGHT		= 1 << 1,
};


struct Block
{
public:
	void SetDirtyLightFlag(bool isDirty);
	void SetSkyFlag(bool isSky);
	void SetIndoorLightExposure(int indoorLight);
	void SetOutdoorLightExposure(int outdoorLight);

	int GetIndoorLightExposure() const;
	int GetOutdoorLightExposure() const;
	int GetInternalIndoorLight() const;
	int GetInternalOutdoorLight() const;

	bool IsLightDirty() const;
	bool IsSky() const;
	bool IsOpaque() const;
	bool IsSolid() const;
	bool EmitsLight() const;
public:
	uint8_t m_blockDefId;
	uint8_t m_blockFlags = BLOCK_BIT_FLAG_NONE;
	uint8_t m_lightExposure = 0;
};
