#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Game/EntitySpawnInfo.hpp"
#include<vector>
#include "Game/MouseData.hpp"

class Level;
class LevelEditor;
struct TileDefinition;
class EditorCommand;

//enum to identify the type of operation
enum EditorOperationType : int
{
	EDITOR_OPERATION_NONE = -1,
	EDITOR_OPERATION_TILE_PAINTING,
	EDITOR_OPERATION_TILE_ERASING,
	EDITOR_OPERATION_ENTITY_PAINTING,
	EDITOR_OPERATION_ENTITY_ERASING,
	EDITOR_OPERATION_TILE_DRAG,
	EDITOR_OPERATION_ENTITY_DRAG,
	EDITOR_OPERATION_WAYPOINT_ASSIGNMENT,
	EDITOR_OPERATION_SELECTIION_DRAG,
};

//Class that represents an atomic operation performed by the editor/user
class EditorOperation
{
public:
	friend EditorCommand;

	EditorOperation(EditorOperationType type);
	virtual ~EditorOperation();
	virtual bool Undo(LevelEditor* editor) = 0;
	virtual bool Execute(LevelEditor* editor) = 0;
	virtual bool IsValid(Level* level) = 0;

protected:
	EditorOperationType m_type = EDITOR_OPERATION_NONE;
};


class TilePaintingOperation : public EditorOperation
{
public:
	friend EditorCommand;

	TilePaintingOperation(IntVec2 const& tileCoords, TileDefinition const* definition);
	TilePaintingOperation(IntVec2 const& tileCoords, TileDefinition const* definition, TileDefinition const* originalTileDef);
	bool Undo(LevelEditor* editor) override;
	bool Execute(LevelEditor* editor) override;
	bool IsValid(Level* level) override;

private:
	IntVec2 m_tileCoords;
	TileDefinition const* m_definition = nullptr;
	TileDefinition const* m_originalTileDef = nullptr;
};


class TileErasingOperation : public EditorOperation
{
public:
	friend EditorCommand;

	TileErasingOperation(IntVec2 const& tileCoords, TileDefinition const* prevDefinition);
	bool Undo(LevelEditor* editor) override;
	bool Execute(LevelEditor* editor) override;
	bool IsValid(Level* level) override;

private:
	IntVec2 m_tileCoords;
	TileDefinition const* m_previousDefinition = nullptr;
};

typedef std::vector<EntitySpawnInfo> EntitySpawns;

class EntityPaintingOperation : public EditorOperation
{
public:
	friend EditorCommand;

	EntityPaintingOperation(EntitySpawnInfo const& spawnInfo);
	EntityPaintingOperation(EntitySpawnInfo const& spawnInfo, EntitySpawns const& overwrittenSpawnInfo);
	bool Undo(LevelEditor* editor) override;
	bool Execute(LevelEditor* editor) override;
	bool IsValid(Level* level) override;

private:
	EntitySpawnInfo m_spawnInfo = EntitySpawnInfo::s_INVALID_SPAWN_INFO;
	EntitySpawns m_overwrittenSpawns;;
};


class EntityErasingOperation : public EditorOperation
{
public:
	friend EditorCommand;

	EntityErasingOperation(EntitySpawnInfo const& spawnInfo);
	bool Undo(LevelEditor* editor) override;
	bool Execute(LevelEditor* editor) override;
	bool IsValid(Level* level) override;
private:
	EntitySpawnInfo m_spawnInfo = EntitySpawnInfo::s_INVALID_SPAWN_INFO;
};


class TileDragOperation : public EditorOperation
{
public:
	friend EditorCommand;

	TileDragOperation(IntVec2 const& startTileCoord, IntVec2 const& endTileCoord, TileDefinition const* draggingTileDef);
	TileDragOperation(IntVec2 const& startTileCoord, IntVec2 const& endTileCoord, TileDefinition const* draggingTileDef, TileDefinition const* overwrittenTileDef);

	bool Undo(LevelEditor* editor) override;
	bool Execute(LevelEditor* editor) override;
	bool IsValid(Level* level) override;

private:
	IntVec2 m_startTileCoord;
	IntVec2 m_endTileCoord;
	TileDefinition const* m_draggingTileDef = nullptr;
	TileDefinition const* m_overwrittenTileDef = nullptr;
};


class EntityDragOperation : public EditorOperation
{
public:
	friend EditorCommand;

	EntityDragOperation(EntitySpawnInfo const& originalSpawnInfo, EntitySpawnInfo const& finalSpawnInfo);
	EntityDragOperation(EntitySpawnInfo const& originalSpawnInfo, EntitySpawnInfo const& finalSpawnInfo, std::vector<EntitySpawnInfo> const& overwrittenSpawnInfos);

	bool Undo(LevelEditor* editor) override;
	bool Execute(LevelEditor* editor) override;
	bool IsValid(Level* level) override;

private:
	EntitySpawnInfo m_finalSpawnInfo = EntitySpawnInfo::s_INVALID_SPAWN_INFO;
	EntitySpawnInfo m_originalSpawnInfo = EntitySpawnInfo::s_INVALID_SPAWN_INFO;
	std::vector<EntitySpawnInfo> m_overwrittenEntitySpawns;
};


class WaypointAssignmentOperation : public EditorOperation
{
public:
	friend EditorCommand;
	
public:
	WaypointAssignmentOperation(EntitySpawnInfo const& refSpawnInfo, EntityID const& targetWaypointId);

	bool Undo(LevelEditor* editor) override;
	bool Execute(LevelEditor* editor) override;
	bool IsValid(Level* level) override;

private:
	EntityID m_refEntityId = EntityID::s_INVALID_ID;
	EntityID m_originalWaypointId = EntityID::s_INVALID_ID;
	EntityID m_targetWaypointId = EntityID::s_INVALID_ID;
};


class SelectionDragOperation : public EditorOperation
{
public:
	friend EditorCommand;

public:
	SelectionDragOperation(MouseSelectionData const& originalSelectionData, MouseSelectionData const& newSelectionData);

	bool Undo(LevelEditor* editor) override;
	bool Execute(LevelEditor* editor) override;
	bool IsValid(Level* level) override;

private:
	MouseSelectionData m_originalSelectionData;
	MouseSelectionData m_newSelectionData;
	EntitySpawns m_overwrittenEntitySpawns;
	TileSelectionData m_overwrittenTiles;
};