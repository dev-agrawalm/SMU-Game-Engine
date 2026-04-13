#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Math/IntVec2.hpp"

TileHeatMap::TileHeatMap()
{

}


TileHeatMap::TileHeatMap(IntVec2 const& dimensions)
{
	int arraySize = dimensions.x * dimensions.y;
	m_values.assign(arraySize, 0.0f);
}


void TileHeatMap::SetValueAtTile(int tileIndex, float valueToSet)
{
	m_values[tileIndex] = valueToSet;
}


void TileHeatMap::SetAllValues(float valueToSet)
{
	for (int tileIndex = 0; tileIndex < (int) m_values.size(); tileIndex++)
	{
		m_values[tileIndex] = valueToSet;
	}
}


void TileHeatMap::AddValueAtTile(int tileIndex, float valueToAdd)
{
	m_values[tileIndex] += valueToAdd;
}


float TileHeatMap::GetValueAtTile(int tileIndex) const
{
	return m_values[tileIndex];
}


TileHeatMap::~TileHeatMap()
{

}
