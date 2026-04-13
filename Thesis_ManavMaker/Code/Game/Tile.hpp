#pragma once
#include "Engine/Core/Rgba8.hpp"
#include<vector>
#include <string>
#include "Game/TileSet.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EventSystem.hpp"

struct IntVec2;
class SpriteSheet;
class SpriteDefinition;

enum TileOptions : uint32_t
{
	TILE_OPTIONS_NONE = 0,
	TILE_OPTIONS_COMPLETELY_SOLID = 1 << 0,
	TILE_OPTIONS_DOES_DAMAGE = 1 << 1,
	TILE_OPTIONS_SEMI_SOLID = 1 <<2,
};

enum TileFlags : uint32_t
{
	TILE_FLAG_NONE = 0,
};


struct TileDefinition
{
public:
	static void InitialiseDefinitions();
	static void AddDefinition(std::string name, Rgba8 const& color, uint32_t flags = 0);
	static void AddDefinition(std::string name, Rgba8 const& color, uint32_t flags, SpriteSheet* spriteSheet, IntVec2 const* spriteCoords, int numSprites);
	static void AddDefinition(std::string name, Rgba8 const& color, uint32_t flags, SpriteSheet* spriteSheet, IntVec2 const spriteCoord);
	static void AddDefinition(std::string name, Rgba8 const& color, uint32_t flags, SpriteSheet* spriteSheet, IntVec2 const spriteCoord, int damage);
	static TileDefinition* GetDefinitionByName(std::string name);

public:
	AABB2 GetUVs() const;
	SpriteSheet* GetSpriteSheet() const;
	SpriteDefinition const* GetSpriteDefinition() const;

public:
	std::string m_name;
	Rgba8 m_color = Rgba8::WHITE;
	uint32_t m_options = TILE_OPTIONS_NONE;
	int m_damage = 0;
	Tileset m_tileSet = {};

private:
	static std::vector<TileDefinition> s_definitions;
};


struct Tile
{
public:
	bool IsSelected() const;
	bool IsTagged() const;

	bool IsSolid(bool treatSemiSolidAsSolid = false) const;
	bool IsSemiSolid() const;
	bool IsEmpty() const;
	bool DoesDamage() const;
	int GetDamage() const;

	bool OnStep(EventArgs& args);

	void SelectTile();
	void DeselectTile();
	void TagTile(Rgba8 const& color = Rgba8::WHITE);
	void UnTagTile();
public:
	TileDefinition const* m_definition = nullptr;
	uint32_t m_flags = 0;
	Rgba8 m_taggedColor = Rgba8::WHITE;
	bool m_isTagged = false;
	bool m_isSelected = false;
	//std::string m_testSfxName = "Data/Audio/Click.mp3";
	int m_index;
};
