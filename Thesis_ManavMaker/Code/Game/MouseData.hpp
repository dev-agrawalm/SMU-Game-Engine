#pragma once
#include "Game/Tile.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/EntitySpawnInfo.hpp"

enum class MouseTool
{
	INSPECT_TOOL,
	PAINTING_TOOL,
	DRAG_TOOL,
	MULTI_SELECT_TOOL,
	DUPLICATE_TOOl,
	ERASE_TOOL,
	FILL_TOOL,
	LINE_TOOL
};


struct TileSelection
{
public:
	TileSelection(TileDefinition const* tileDef, IntVec2 const& tileCoords);

public:
	TileDefinition const* m_tileDef = nullptr;
	IntVec2 m_tileCoords;
};

typedef std::vector<TileSelection> TileSelectionData;
typedef std::vector<EntitySpawnInfo> EntitySpawns;

struct MouseSelectionData
{
public:
	AABB2 m_selectionBounds;
	TileSelectionData m_tileSelections;
	EntitySpawns m_entitySpawnSelections;
};


struct MouseData
{
public:
	bool IsSelectionDataEmpty() const;
	bool IsEntitySpawnAlreadySelected(EntitySpawnInfo const& entitySpawn) const;
	bool IsTileAlreadySelected(TileSelection const& tileSelection) const;
	AABB2 GetSelectionBounds() const;
	MouseSelectionData const& GetSelectionData() const;

	void AddEntitySpawnToSelection(EntitySpawnInfo const& entitySpawn);
	void RemoveEntitySpawnFromSelection(EntitySpawnInfo const& entitySpawn);
	void AddTileSelectionToSelection(TileSelection const& tileSelection);
	void RemoveTileFromSelection(IntVec2 const& tileCoords);
	void SetSelectionBounds(AABB2 const& selectionBounds);
	void EmptySelectionData();

public:
	//MouseTool m_tool = MouseTool::INSPECT_TOOL;

	bool m_isEntity = false;
	TileDefinition const* m_tileDefinition = nullptr;
	EntityType m_entityType = ENTITY_TYPE_NONE;
	EntitySpawnInfo m_spawnInfo = EntitySpawnInfo::s_INVALID_SPAWN_INFO;

	MouseSelectionData m_selectionData;

	int m_mouseWheel = 0;
	IntVec2 m_previousTileCoords = IntVec2::ZERO;
	IntVec2 m_currentTileCoords = IntVec2::ZERO;
	Vec2 m_previousWorldPos = Vec2::ZERO;
	Vec2 m_currentWorldPos = Vec2::ZERO;

	IntVec2 m_dragStartTileCoord;
	Vec2 m_dragStartPos;

	bool m_renderMouse = false;
};
