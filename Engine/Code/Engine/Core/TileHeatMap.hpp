#pragma once
#include<vector>

struct IntVec2;

class TileHeatMap
{
public:
	TileHeatMap();
	~TileHeatMap();
	TileHeatMap(IntVec2 const& dimensions);

	//mutators
	void SetValueAtTile(int tileIndex, float valueToSet);
	void SetAllValues(float valueToSet);
	void AddValueAtTile(int tileIndex, float valueToAdd);

	//getters
	float GetValueAtTile(int tileIndex) const;

private:
	std::vector<float> m_values;
};