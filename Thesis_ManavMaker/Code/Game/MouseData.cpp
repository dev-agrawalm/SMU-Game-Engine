#include "Game/MouseData.hpp"
#include "Game/GameCommon.hpp"

void MouseData::SetSelectionBounds(AABB2 const& selectionBounds)
{
	m_selectionData.m_selectionBounds = selectionBounds;
}


void MouseData::AddTileSelectionToSelection(TileSelection const& tileSelection)
{
	if (tileSelection.m_tileDef == nullptr)
		return;

	if (IsTileAlreadySelected(tileSelection))
		return;

	m_selectionData.m_tileSelections.push_back(tileSelection);
}


void MouseData::RemoveTileFromSelection(IntVec2 const& tileCoords)
{
	for (int i = 0; i < (int) m_selectionData.m_tileSelections.size(); i++)
	{
		TileSelection& tileSelection = m_selectionData.m_tileSelections[i];
		if (tileSelection.m_tileCoords == tileCoords)
		{
			tileSelection = m_selectionData.m_tileSelections.back();
			m_selectionData.m_tileSelections.pop_back();
			return;
		}
	}
}


void MouseData::AddEntitySpawnToSelection(EntitySpawnInfo const& entitySpawn)
{
	if (entitySpawn.IsInvalid())
		return;

	if (IsEntitySpawnAlreadySelected(entitySpawn))
		return;

	m_selectionData.m_entitySpawnSelections.push_back(entitySpawn);
}


void MouseData::RemoveEntitySpawnFromSelection(EntitySpawnInfo const& entitySpawn)
{
	for (int i = 0; i < (int) m_selectionData.m_entitySpawnSelections.size(); i++)
	{
		EntitySpawnInfo& spawnInfo = m_selectionData.m_entitySpawnSelections[i];
		if (spawnInfo == entitySpawn)
		{
			spawnInfo = m_selectionData.m_entitySpawnSelections.back();
			m_selectionData.m_entitySpawnSelections.pop_back();
			return;
		}
	}
}


void MouseData::EmptySelectionData()
{
	m_selectionData.m_entitySpawnSelections.clear();
	m_selectionData.m_tileSelections.clear();
}


bool MouseData::IsSelectionDataEmpty() const
{
	return m_selectionData.m_entitySpawnSelections.size() <= 0 && m_selectionData.m_tileSelections.size() <= 0;
}


bool MouseData::IsEntitySpawnAlreadySelected(EntitySpawnInfo const& entitySpawn) const
{
	if (entitySpawn.IsInvalid())
		return false;

	for (int spawnInfoIndex = 0; spawnInfoIndex < (int) m_selectionData.m_entitySpawnSelections.size(); spawnInfoIndex++)
	{
		EntitySpawnInfo const& info = m_selectionData.m_entitySpawnSelections[spawnInfoIndex];
		if (info == entitySpawn)
			return true;
	}

	return false;
}


bool MouseData::IsTileAlreadySelected(TileSelection const& tileSelection) const
{
	if (tileSelection.m_tileDef == nullptr)
		return false;

	for (int tileSelectionIndex = 0; tileSelectionIndex < (int) m_selectionData.m_tileSelections.size(); tileSelectionIndex++)
	{
		TileSelection const& tile = m_selectionData.m_tileSelections[tileSelectionIndex];
		if (tile.m_tileCoords.GetVec2() == tileSelection.m_tileCoords.GetVec2() && tile.m_tileDef == tileSelection.m_tileDef)
			return true;
	}

	return false;
}


AABB2 MouseData::GetSelectionBounds() const
{
	return m_selectionData.m_selectionBounds;
}


MouseSelectionData const& MouseData::GetSelectionData() const
{
	return m_selectionData;
}


TileSelection::TileSelection(TileDefinition const* tileDef, IntVec2 const& tileCoords)
	: m_tileDef(tileDef)
	, m_tileCoords(tileCoords)
{

}

