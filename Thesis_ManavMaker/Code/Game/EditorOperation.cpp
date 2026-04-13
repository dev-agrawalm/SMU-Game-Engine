#include "Game/EditorOperation.hpp"
#include "Game/LevelEditor.hpp"
#include "Game/Level.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Tile.hpp"

EditorOperation::EditorOperation(EditorOperationType type)
	: m_type(type)
{

}


EditorOperation::~EditorOperation()
{

}


TilePaintingOperation::TilePaintingOperation(IntVec2 const& tileCoords, TileDefinition const* definition)
	: EditorOperation(EDITOR_OPERATION_TILE_PAINTING)
	, m_tileCoords(tileCoords)
	, m_definition(definition)
{

}


TilePaintingOperation::TilePaintingOperation(IntVec2 const& tileCoords, TileDefinition const* definition, TileDefinition const* originalTileDef)
	: EditorOperation(EDITOR_OPERATION_TILE_PAINTING)
	, m_tileCoords(tileCoords)
	, m_definition(definition)
	, m_originalTileDef(originalTileDef)
{

}


bool TilePaintingOperation::Undo(LevelEditor* editor)
{
	if (m_originalTileDef == nullptr)
	{
		editor->EraseTile(m_tileCoords);
		editor->DeselectTile(m_tileCoords);
	}
	else
	{
		editor->PaintTile(m_tileCoords, m_originalTileDef);
	}
	return true;
}


bool TilePaintingOperation::Execute(LevelEditor* editor)
{
	editor->PaintTile(m_tileCoords, m_definition);
	return true;
}


bool TilePaintingOperation::IsValid(Level* level)
{
	return !level->DoesTileIntersectAnySpawnInfo(m_tileCoords) && m_definition != m_originalTileDef;
}


TileErasingOperation::TileErasingOperation(IntVec2 const& tileCoords, TileDefinition const* prevDefinition)
	: EditorOperation(EDITOR_OPERATION_TILE_ERASING)
	, m_tileCoords(tileCoords)
	, m_previousDefinition(prevDefinition)
{

}


bool TileErasingOperation::Undo(LevelEditor* editor)
{
	editor->PaintTile(m_tileCoords, m_previousDefinition);
	return true;
}


bool TileErasingOperation::Execute(LevelEditor* editor)
{
	editor->EraseTile(m_tileCoords);
	return true;
}


bool TileErasingOperation::IsValid(Level* level)
{
	return m_previousDefinition != nullptr && !level->IsTileEmpty(m_tileCoords);
}


EntityPaintingOperation::EntityPaintingOperation(EntitySpawnInfo const& spawnInfo)
	: EditorOperation(EDITOR_OPERATION_ENTITY_PAINTING)
	, m_spawnInfo(spawnInfo)
{

}


EntityPaintingOperation::EntityPaintingOperation(EntitySpawnInfo const& spawnInfo, EntitySpawns const& overwrittenSpawnInfo)
	: EditorOperation(EDITOR_OPERATION_ENTITY_PAINTING)
	, m_spawnInfo(spawnInfo)
	, m_overwrittenSpawns(overwrittenSpawnInfo)
{

}


bool EntityPaintingOperation::Undo(LevelEditor* editor)
{
	editor->GetLevel()->RemoveEntitySpawn(m_spawnInfo);
	for (int i = 0; i < m_overwrittenSpawns.size(); i++)
	{
		EntitySpawnInfo overwrittenSpawnInfo = m_overwrittenSpawns[i];
		if (overwrittenSpawnInfo.IsValid())
		{
			editor->GetLevel()->AddEntitySpawn(overwrittenSpawnInfo);
		}
	}
	return true;
}


bool EntityPaintingOperation::Execute(LevelEditor* editor)
{
	for (int i = 0; i < m_overwrittenSpawns.size(); i++)
	{
		EntitySpawnInfo overwrittenSpawnInfo = m_overwrittenSpawns[i];
		if (overwrittenSpawnInfo.IsValid())
		{
			editor->GetLevel()->RemoveEntitySpawn(overwrittenSpawnInfo);
		}
	}
	editor->GetLevel()->AddEntitySpawn(m_spawnInfo);
	return true;
}


bool EntityPaintingOperation::IsValid(Level* level)
{
	return !level->DoesSpawnInfoIntersectSolidTiles(m_spawnInfo);
}


EntityErasingOperation::EntityErasingOperation(EntitySpawnInfo const& spawnInfo)
	: EditorOperation(EDITOR_OPERATION_ENTITY_ERASING)
	, m_spawnInfo(spawnInfo)
{

}


bool EntityErasingOperation::Undo(LevelEditor* editor)
{
	editor->GetLevel()->AddEntitySpawn(m_spawnInfo);
	return true;
}


bool EntityErasingOperation::Execute(LevelEditor* editor)
{
	IntVec2 tileCoords = editor->GetLevel()->GetTileCoords(m_spawnInfo.m_startingPosition);
	editor->EraseTile(tileCoords);
	return true;
}


bool EntityErasingOperation::IsValid(Level* level)
{
	return level->DoesSpawnInfoExist(m_spawnInfo);
}


TileDragOperation::TileDragOperation(IntVec2 const& startTileCoord, IntVec2 const& endTileCoord, TileDefinition const* draggingTileDef)
	: EditorOperation(EDITOR_OPERATION_TILE_DRAG)
	, m_startTileCoord(startTileCoord)
	, m_endTileCoord(endTileCoord)
	, m_draggingTileDef(draggingTileDef)
{

}


TileDragOperation::TileDragOperation(IntVec2 const& startTileCoord, IntVec2 const& endTileCoord, TileDefinition const* draggingTileDef, TileDefinition const* overwrittenTileDef)
	: EditorOperation(EDITOR_OPERATION_TILE_DRAG)
	, m_startTileCoord(startTileCoord)
	, m_endTileCoord(endTileCoord)
	, m_draggingTileDef(draggingTileDef)
	, m_overwrittenTileDef(overwrittenTileDef)
{

}


bool TileDragOperation::Undo(LevelEditor* editor)
{
	editor->GetLevel()->SetTileEmpty(m_endTileCoord);
	editor->GetLevel()->SetTileDefinitionAtCoords(m_draggingTileDef, m_startTileCoord);
	
	editor->DeselectTile(m_endTileCoord);
	editor->SelectTile(m_startTileCoord);
	
	if (m_overwrittenTileDef != nullptr)
	{
		editor->GetLevel()->SetTileDefinitionAtCoords(m_overwrittenTileDef, m_endTileCoord);
	}
	return true;
}


bool TileDragOperation::Execute(LevelEditor* editor)
{
	Level* level = editor->GetLevel();
	Tile latestStartTile = level->GetTile(m_startTileCoord);

	level->SetTileDefinitionAtCoords(m_draggingTileDef, m_endTileCoord);
	editor->SelectTile(m_endTileCoord);
	return true;
}


bool TileDragOperation::IsValid(Level* level)
{
	return level->IsTileCoordValid(m_endTileCoord) && !level->DoesTileIntersectAnySpawnInfo(m_endTileCoord);
}


EntityDragOperation::EntityDragOperation(EntitySpawnInfo const& originalSpawnInfo, EntitySpawnInfo const& finalSpawnInfo)
	:EditorOperation(EDITOR_OPERATION_ENTITY_DRAG)
	, m_originalSpawnInfo(originalSpawnInfo)
	, m_finalSpawnInfo(finalSpawnInfo)
{

}


EntityDragOperation::EntityDragOperation(EntitySpawnInfo const& originalSpawnInfo, EntitySpawnInfo const& finalSpawnInfo, std::vector<EntitySpawnInfo> const& overwrittenSpawnInfos)
	:EditorOperation(EDITOR_OPERATION_ENTITY_DRAG)
	, m_originalSpawnInfo(originalSpawnInfo)
	, m_finalSpawnInfo(finalSpawnInfo)
	, m_overwrittenEntitySpawns(overwrittenSpawnInfos)
{

}


bool EntityDragOperation::Undo(LevelEditor* editor)
{
	editor->GetLevel()->RemoveEntitySpawn(m_finalSpawnInfo);
	for (int i = 0; i < m_overwrittenEntitySpawns.size(); i++)
	{
		editor->GetLevel()->AddEntitySpawn(m_overwrittenEntitySpawns[i]);
	}
	editor->GetLevel()->AddEntitySpawn(m_originalSpawnInfo);

	editor->DeselectSpawnInfo(m_finalSpawnInfo);
	editor->SelectSpawnInfo(m_originalSpawnInfo);
	return true;
}


bool EntityDragOperation::Execute(LevelEditor* editor)
{
	Level* level = editor->GetLevel();
	EntitySpawnInfo latestStartSpawnInfo = level->GetEntitySpawnInfo(m_originalSpawnInfo.m_startingPosition);
	if (!latestStartSpawnInfo.m_isSelected)
	{
		editor->GetLevel()->RemoveEntitySpawn(m_originalSpawnInfo);
		editor->DeselectSpawnInfo(m_originalSpawnInfo);
	}

	editor->GetLevel()->AddEntitySpawn(m_finalSpawnInfo);
	for (int i = 0; i < m_overwrittenEntitySpawns.size(); i++)
	{
		editor->GetLevel()->RemoveEntitySpawn(m_overwrittenEntitySpawns[i]);
	}

	editor->SelectSpawnInfo(m_finalSpawnInfo);
	return true;
}


bool EntityDragOperation::IsValid(Level* level)
{
	EntityType finalSpawnInfoType = m_finalSpawnInfo.m_type;
	EntityDefinition const* def = EntityDefinition::GetEntityDefinitionByType(finalSpawnInfoType);
	//AABB2 entityBounds = GetPhysicsBoundsForEntity(m_finalSpawnInfo.m_type, m_finalSpawnInfo.m_startingPosition);
	AABB2 entityBounds = def->GetPhysicsBounds(m_finalSpawnInfo.m_startingPosition);
	Vec2 mins = entityBounds.m_mins;
	Vec2 maxs = entityBounds.m_maxs;
	bool isInBounds = mins.x >= 0.0f && maxs.x <= (float) LEVEL_SIZE_X && mins.y >= 0.0f && maxs.y <= (float) LEVEL_SIZE_Y;
	bool intersectsSolidTiles = level->DoesSpawnInfoIntersectSolidTiles(m_finalSpawnInfo);
	
	bool success = isInBounds && !intersectsSolidTiles;
	return success;
}


	
WaypointAssignmentOperation::WaypointAssignmentOperation(EntitySpawnInfo const& refSpawnInfo, EntityID const& targetWaypointId)
	: EditorOperation(EDITOR_OPERATION_WAYPOINT_ASSIGNMENT)
	, m_refEntityId(refSpawnInfo.m_entityId)
	, m_originalWaypointId(refSpawnInfo.m_targetWaypointId)
	, m_targetWaypointId(targetWaypointId)
{
}


bool WaypointAssignmentOperation::Undo(LevelEditor* editor)
{
	editor->GetLevel()->UpdateSpawnInfo_TargetWaypoint(m_refEntityId, m_originalWaypointId);
	return true;
}


bool WaypointAssignmentOperation::Execute(LevelEditor* editor)
{
	editor->GetLevel()->UpdateSpawnInfo_TargetWaypoint(m_refEntityId, m_targetWaypointId);
	return true;
}


bool WaypointAssignmentOperation::IsValid(Level* level)
{
	EntitySpawnInfo const& refSpawnInfo = level->GetEntitySpawnInfoById(m_refEntityId);
	EntitySpawnInfo const& targetWaypointSpawnInfo = level->GetEntitySpawnInfoById(m_targetWaypointId);
	return refSpawnInfo.IsValid() && targetWaypointSpawnInfo.IsValid() && targetWaypointSpawnInfo.m_type == ENTITY_TYPE_WAYPOINT;
}


SelectionDragOperation::SelectionDragOperation(MouseSelectionData const& originalSelectionData, MouseSelectionData const& newSelectionData)
	: EditorOperation(EDITOR_OPERATION_SELECTIION_DRAG)
	, m_originalSelectionData(originalSelectionData)
	, m_newSelectionData(newSelectionData)
{
	m_overwrittenEntitySpawns.reserve(100);
}


bool SelectionDragOperation::Undo(LevelEditor* editor)
{
	Level* level = editor->GetLevel();

	EntitySpawns const& oldEntitySpawns = m_originalSelectionData.m_entitySpawnSelections;
	EntitySpawns const& newEntitySpawns = m_newSelectionData.m_entitySpawnSelections;

	for (int entitySpawnIndex = 0; entitySpawnIndex < newEntitySpawns.size(); entitySpawnIndex++)
	{
		EntitySpawnInfo newEntitySpawn = newEntitySpawns[entitySpawnIndex];
		level->RemoveEntitySpawn(newEntitySpawn);
		editor->DeselectSpawnInfo(newEntitySpawn);
	}

	for (int entitySpawnIndex = 0; entitySpawnIndex < oldEntitySpawns.size(); entitySpawnIndex++)
	{
		EntitySpawnInfo oldEntitySpawn = oldEntitySpawns[entitySpawnIndex];
		EntityType oldSpawnInfoType = oldEntitySpawn.m_type;
		EntityDefinition const* def = EntityDefinition::GetEntityDefinitionByType(oldSpawnInfoType);
		AABB2 entityBounds = def->GetPhysicsBounds(oldEntitySpawn.m_startingPosition);
		Vec2 mins = entityBounds.m_mins;
		Vec2 maxs = entityBounds.m_maxs;
		bool isInBounds = mins.x >= 0.0f && maxs.x <= (float) LEVEL_SIZE_X && mins.y >= 0.0f && maxs.y <= (float) LEVEL_SIZE_Y;
		bool intersectsSolidTiles = level->DoesSpawnInfoIntersectSolidTiles(oldEntitySpawn);

		bool success = isInBounds && !intersectsSolidTiles;
		if (success)
		{
			level->AddEntitySpawn(oldEntitySpawn);
			editor->SelectSpawnInfo(oldEntitySpawn);
		}
	}

	for (int entitySpawnIndex = 0; entitySpawnIndex < m_overwrittenEntitySpawns.size(); entitySpawnIndex++)
	{
		level->AddEntitySpawn(m_overwrittenEntitySpawns[entitySpawnIndex]);
	}

	TileSelectionData const& oldTileSelectionData = m_originalSelectionData.m_tileSelections;
	TileSelectionData const& newTileSelectionData = m_newSelectionData.m_tileSelections;
	for (int tileIndex = 0; tileIndex < newTileSelectionData.size(); tileIndex++)
	{
		IntVec2 newTileCoord = newTileSelectionData[tileIndex].m_tileCoords;
		level->SetTileEmpty(newTileCoord);
		editor->DeselectTile(newTileCoord);
	}

	for (int tileIndex = 0; tileIndex < oldTileSelectionData.size(); tileIndex++)
	{
		TileSelection oldTileSelection = oldTileSelectionData[tileIndex];
		IntVec2 oldTileCoords = oldTileSelection.m_tileCoords;
		if (level->IsTileCoordValid(oldTileCoords) && !level->DoesTileIntersectAnySpawnInfo(oldTileCoords))
		{
			level->SetTileDefinitionAtCoords(oldTileSelection.m_tileDef, oldTileCoords);
			editor->SelectTile(oldTileCoords);
		}
	}

	for (int tileIndex = 0; tileIndex < m_overwrittenTiles.size(); tileIndex++)
	{
		TileSelection overwrittenTile = m_overwrittenTiles[tileIndex];
		IntVec2 overwrittenTileCoords = overwrittenTile.m_tileCoords;
		TileDefinition const* overwrittenTileDef = overwrittenTile.m_tileDef;
		level->SetTileDefinitionAtCoords(overwrittenTileDef, overwrittenTileCoords);
	}

	m_overwrittenTiles.clear();
	m_overwrittenEntitySpawns.clear();
	return true;
}


bool SelectionDragOperation::Execute(LevelEditor* editor)
{
	Level* level = editor->GetLevel();
	
	EntitySpawns const& oldEntitySpawns = m_originalSelectionData.m_entitySpawnSelections;
	EntitySpawns const& newEntitySpawns = m_newSelectionData.m_entitySpawnSelections;
	
	for (int entitySpawnIndex = 0; entitySpawnIndex < oldEntitySpawns.size(); entitySpawnIndex++)
	{
		EntitySpawnInfo oldEntitySpawn = oldEntitySpawns[entitySpawnIndex];
		level->RemoveEntitySpawn(oldEntitySpawn);
		editor->DeselectSpawnInfo(oldEntitySpawn);
	}

	for (int entitySpawnIndex = 0; entitySpawnIndex < newEntitySpawns.size(); entitySpawnIndex++)
	{
		EntitySpawnInfo newEntitySpawn = newEntitySpawns[entitySpawnIndex];
		EntityType newSpawnInfoType = newEntitySpawn.m_type;
		EntityDefinition const* def = EntityDefinition::GetEntityDefinitionByType(newSpawnInfoType);
		AABB2 entityBounds = def->GetPhysicsBounds(newEntitySpawn.m_startingPosition);
		Vec2 mins = entityBounds.m_mins;
		Vec2 maxs = entityBounds.m_maxs;
		bool isInBounds = mins.x >= 0.0f && maxs.x <= (float) LEVEL_SIZE_X && mins.y >= 0.0f && maxs.y <= (float) LEVEL_SIZE_Y;
		bool intersectsSolidTiles = level->DoesSpawnInfoIntersectSolidTiles(newEntitySpawn);

		bool success = isInBounds && !intersectsSolidTiles;
		if (success)
		{
			level->AddOverlappingSpawnInfosToVector(m_overwrittenEntitySpawns, newEntitySpawn);
			level->AddEntitySpawn(newEntitySpawn);
			editor->SelectSpawnInfo(newEntitySpawn);
		}
	}

	TileSelectionData const& oldTileSelectionData = m_originalSelectionData.m_tileSelections;
	TileSelectionData const& newTileSelectionData = m_newSelectionData.m_tileSelections;
	for (int tileIndex = 0; tileIndex < oldTileSelectionData.size(); tileIndex++)
	{
		IntVec2 startTileCoord = oldTileSelectionData[tileIndex].m_tileCoords;
		level->SetTileEmpty(startTileCoord);
		editor->DeselectTile(startTileCoord);
	}

	for (int tileIndex = 0; tileIndex < newTileSelectionData.size(); tileIndex++)
	{
		TileSelection newTileSelection = newTileSelectionData[tileIndex];
		IntVec2 newTileCoords = newTileSelection.m_tileCoords;
		if(level->IsTileCoordValid(newTileCoords) && !level->DoesTileIntersectAnySpawnInfo(newTileCoords))
		{
			Tile overwrittenTile = level->GetTile(newTileCoords);
			if (!overwrittenTile.IsEmpty())
			{
				TileSelection overwrittenTileSelection = TileSelection(overwrittenTile.m_definition, newTileCoords);
				m_overwrittenTiles.push_back(overwrittenTileSelection);
			}
			level->SetTileDefinitionAtCoords(newTileSelection.m_tileDef, newTileCoords);
			editor->SelectTile(newTileCoords);
		}
	}

	return true;
}


bool SelectionDragOperation::IsValid(Level* level)
{
	UNUSED(level);
	return true;
}


