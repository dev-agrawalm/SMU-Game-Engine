#include "Game/Tile.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"

void TileDefinition::InitialiseDefinitions()
{
	uint32_t options = TILE_OPTIONS_NONE;
	AddDefinition("Empty", Rgba8::WHITE, options);

	bool useStone = g_gameConfigBlackboard.GetValue("useStoneAsGround", false);

	SpriteSheet* tileSpriteSheet = g_game->CreateOrGetSpriteSheet(IntVec2(128, 128));

	if (!useStone)
	{
		IntVec2 groundSpriteCoords[16] = {
			IntVec2(0,0),
			IntVec2(1,0),
			IntVec2(2,0),
			IntVec2(3,0),
			IntVec2(0,1),
			IntVec2(1,1),
			IntVec2(2,1),
			IntVec2(3,1),
			IntVec2(0,2),
			IntVec2(1,2),
			IntVec2(2,2),
			IntVec2(3,2),
			IntVec2(0,3),
			IntVec2(1,3),
			IntVec2(2,3),
			IntVec2(3,3)
		};
	
		options = TILE_OPTIONS_COMPLETELY_SOLID;
		AddDefinition("Ground", Rgba8::YELLOW, options, tileSpriteSheet, groundSpriteCoords, 16);
	}

	if (useStone)
	{
		IntVec2 stoneSpriteCoords[48] = {
			IntVec2(0	,16),
			IntVec2(1	,16),
			IntVec2(2	,16),
			IntVec2(3	,16),
			IntVec2(4	,16),
			IntVec2(5	,16),
			IntVec2(6	,16),
			IntVec2(7	,16),
			IntVec2(8	,16),
			IntVec2(9	,16),
			IntVec2(10	,16),
			IntVec2(11	,16),
			IntVec2(0	,17),
			IntVec2(1	,17),
			IntVec2(2	,17),
			IntVec2(3	,17),
			IntVec2(4	,17),
			IntVec2(5	,17),
			IntVec2(6	,17),
			IntVec2(7	,17),
			IntVec2(8	,17),
			IntVec2(9	,17),
			IntVec2(10	,17),
			IntVec2(11	,17),
			IntVec2(0	,18),
			IntVec2(1	,18),
			IntVec2(2	,18),
			IntVec2(3	,18),
			IntVec2(4	,18),
			IntVec2(5	,18),
			IntVec2(6	,18),
			IntVec2(7	,18),
			IntVec2(8	,18),
			IntVec2(9	,18),
			IntVec2(10	,18),
			IntVec2(11	,18),
			IntVec2(0	,19),
			IntVec2(1	,19),
			IntVec2(2	,19),
			IntVec2(3	,19),
			IntVec2(4	,19),
			IntVec2(5	,19),
			IntVec2(6	,19),
			IntVec2(7	,19),
			IntVec2(8	,19),
			IntVec2(9	,19),
			IntVec2(10	,19),
			IntVec2(11	,19)
		};

		options = TILE_OPTIONS_COMPLETELY_SOLID;
		AddDefinition("Ground", Rgba8::YELLOW, options, tileSpriteSheet, stoneSpriteCoords, 48);
	}

	IntVec2 spriteCoord = IntVec2(6, 3);
	options = TILE_OPTIONS_COMPLETELY_SOLID | TILE_OPTIONS_DOES_DAMAGE;
	AddDefinition("Spikes", Rgba8::MAGENTA, options, tileSpriteSheet, spriteCoord, 1);

	spriteCoord = IntVec2(4, 3);
	options = TILE_OPTIONS_SEMI_SOLID;
	AddDefinition("Bridge", Rgba8::MAGENTA, options, tileSpriteSheet, spriteCoord);
}


void TileDefinition::AddDefinition(std::string name, Rgba8 const& color, uint32_t flags/* = TILE_OPTIONS_NONE*/)
{
	TileDefinition definition = {};
	definition.m_name = name;
	definition.m_color = color;
	definition.m_options = flags;
	s_definitions.push_back(definition);
}


void TileDefinition::AddDefinition(std::string name, Rgba8 const& color, uint32_t flags, SpriteSheet* spriteSheet, IntVec2 const spriteCoord)
{
	TileDefinition definition = {};
	definition.m_name = name;
	definition.m_color = color;
	definition.m_options = flags;

	Tileset& tileSet = definition.m_tileSet;
	tileSet.m_type = TILE_SET_SINGLE_SPRITE;
	tileSet.m_spriteSheet = spriteSheet;
	tileSet.m_spriteUVs = new AABB2();
	*tileSet.m_spriteUVs = spriteSheet->GetSpriteUVs(spriteCoord);
	s_definitions.push_back(definition);
}


void TileDefinition::AddDefinition(std::string name, Rgba8 const& color, uint32_t flags, SpriteSheet* spriteSheet, IntVec2 const* spriteCoords, int numSprites)
{
	TileDefinition definition = {};
	definition.m_name = name;
	definition.m_color = color;
	definition.m_options = flags;
	Tileset& tileSet = definition.m_tileSet;

	if (numSprites == 16)
	{
		tileSet.m_type = TILE_SET_16_SPRITES;
		tileSet.m_spriteSheet = spriteSheet;
		tileSet.m_spriteUVs = new AABB2[16];
		tileSet.m_spriteUVs[0] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[0]);
		tileSet.m_spriteUVs[1] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[1]);
		tileSet.m_spriteUVs[2] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[2]);
		tileSet.m_spriteUVs[3] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[3]);
		tileSet.m_spriteUVs[4] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[4]);
		tileSet.m_spriteUVs[5] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[5]);
		tileSet.m_spriteUVs[6] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[6]);
		tileSet.m_spriteUVs[7] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[7]);
		tileSet.m_spriteUVs[8] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[8]);
		tileSet.m_spriteUVs[9] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[9]);
		tileSet.m_spriteUVs[10] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[10]);
		tileSet.m_spriteUVs[11] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[11]);
		tileSet.m_spriteUVs[12] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[12]);
		tileSet.m_spriteUVs[13] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[13]);
		tileSet.m_spriteUVs[14] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[14]);
		tileSet.m_spriteUVs[15] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[15]);
	}

	if (numSprites == 48)
	{
		tileSet.m_type = TILE_SET_48_SPRITES;
		tileSet.m_spriteSheet = spriteSheet;
		tileSet.m_spriteUVs = new AABB2[48];
		tileSet.m_spriteUVs[0] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[0]);
		tileSet.m_spriteUVs[1] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[1]);
		tileSet.m_spriteUVs[2] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[2]);
		tileSet.m_spriteUVs[3] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[3]);
		tileSet.m_spriteUVs[4] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[4]);
		tileSet.m_spriteUVs[5] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[5]);
		tileSet.m_spriteUVs[6] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[6]);
		tileSet.m_spriteUVs[7] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[7]);
		tileSet.m_spriteUVs[8] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[8]);
		tileSet.m_spriteUVs[9] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[9]);
		tileSet.m_spriteUVs[10] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[10]);
		tileSet.m_spriteUVs[11] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[11]);
		tileSet.m_spriteUVs[12] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[12]);
		tileSet.m_spriteUVs[13] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[13]);
		tileSet.m_spriteUVs[14] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[14]);
		tileSet.m_spriteUVs[15] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[15]);
		tileSet.m_spriteUVs[16] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[16]);
		tileSet.m_spriteUVs[17] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[17]);
		tileSet.m_spriteUVs[18] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[18]);
		tileSet.m_spriteUVs[19] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[19]);
		tileSet.m_spriteUVs[20] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[20]);
		tileSet.m_spriteUVs[21] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[21]);
		tileSet.m_spriteUVs[22] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[22]);
		tileSet.m_spriteUVs[23] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[23]);
		tileSet.m_spriteUVs[24] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[24]);
		tileSet.m_spriteUVs[25] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[25]);
		tileSet.m_spriteUVs[26] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[26]);
		tileSet.m_spriteUVs[27] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[27]);
		tileSet.m_spriteUVs[28] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[28]);
		tileSet.m_spriteUVs[29] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[29]);
		tileSet.m_spriteUVs[30] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[30]);
		tileSet.m_spriteUVs[31] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[31]);
		tileSet.m_spriteUVs[32] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[32]);
		tileSet.m_spriteUVs[33] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[33]);
		tileSet.m_spriteUVs[34] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[34]);
		tileSet.m_spriteUVs[35] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[35]);
		tileSet.m_spriteUVs[36] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[36]);
		tileSet.m_spriteUVs[37] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[37]);
		tileSet.m_spriteUVs[38] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[38]);
		tileSet.m_spriteUVs[39] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[39]);
		tileSet.m_spriteUVs[40] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[40]);
		tileSet.m_spriteUVs[41] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[41]);
		tileSet.m_spriteUVs[42] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[42]);
		tileSet.m_spriteUVs[43] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[43]);
		tileSet.m_spriteUVs[44] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[44]);
		tileSet.m_spriteUVs[45] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[45]);
		tileSet.m_spriteUVs[46] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[46]);
		tileSet.m_spriteUVs[47] = tileSet.m_spriteSheet->GetSpriteUVs(spriteCoords[47]);
	}
	s_definitions.push_back(definition);
}


void TileDefinition::AddDefinition(std::string name, Rgba8 const& color, uint32_t flags, SpriteSheet* spriteSheet, IntVec2 const spriteCoord, int damage)
{
	TileDefinition definition = {};
	definition.m_name = name;
	definition.m_color = color;
	definition.m_options = flags;
	definition.m_damage = damage;

	Tileset& tileSet = definition.m_tileSet;
	tileSet.m_type = TILE_SET_SINGLE_SPRITE;
	tileSet.m_spriteSheet = spriteSheet;
	tileSet.m_spriteUVs = new AABB2();
	*tileSet.m_spriteUVs = spriteSheet->GetSpriteUVs(spriteCoord);

	s_definitions.push_back(definition);
}


TileDefinition* TileDefinition::GetDefinitionByName(std::string name)
{
	for (int i = 0; i < s_definitions.size(); i++)
	{
		TileDefinition& definition = s_definitions[i];
		std::string definitionName = definition.m_name;
		if (_stricmp(definitionName.c_str(), name.c_str()) == 0)
		{
			return &definition;
		}
	}

	return nullptr;
}


AABB2 TileDefinition::GetUVs() const
{
	if (m_tileSet.m_spriteUVs)
		return *m_tileSet.m_spriteUVs;

	return AABB2();
}


SpriteSheet* TileDefinition::GetSpriteSheet() const
{
	return m_tileSet.m_spriteSheet;
}


SpriteDefinition const* TileDefinition::GetSpriteDefinition() const
{
	if (m_tileSet.m_spriteUVs)
	{
		SpriteDefinition* spriteDef = new SpriteDefinition(*m_tileSet.m_spriteSheet, -1, (*m_tileSet.m_spriteUVs).m_mins, (*m_tileSet.m_spriteUVs).m_maxs);
		return spriteDef;
	}

	return nullptr;
}


std::vector<TileDefinition> TileDefinition::s_definitions;


bool Tile::IsSelected() const
{
	if (m_definition == nullptr)
		return false;

	return m_isSelected;
}


bool Tile::IsTagged() const
{
	if (m_definition == nullptr)
		return false;

	return m_isTagged;
}


bool Tile::IsSolid(bool treatSemiSolidAsSolid) const
{
	if (m_definition == nullptr)
		return false;

	uint32_t options = m_definition->m_options;
	bool isSolid = (options & TILE_OPTIONS_COMPLETELY_SOLID) == TILE_OPTIONS_COMPLETELY_SOLID;
	if (treatSemiSolidAsSolid)
	{
		isSolid |= (options & TILE_OPTIONS_SEMI_SOLID) == TILE_OPTIONS_SEMI_SOLID;
	}

	return isSolid;
}


bool Tile::IsSemiSolid() const
{
	if (m_definition == nullptr)
		return false;

	uint32_t options = m_definition->m_options;
	return (options & TILE_OPTIONS_SEMI_SOLID) == TILE_OPTIONS_SEMI_SOLID;
}


bool Tile::IsEmpty() const
{
	if (m_definition == nullptr)
		return true;

	return m_definition->m_options == TILE_OPTIONS_NONE;
}


bool Tile::DoesDamage() const
{
	if (m_definition == nullptr)
		return false;

	uint32_t options = m_definition->m_options;
	return (options & TILE_OPTIONS_DOES_DAMAGE) == TILE_OPTIONS_DOES_DAMAGE;
}


int Tile::GetDamage() const
{
	if (m_definition == nullptr)
		return 0;

	return m_definition->m_damage;
}


bool Tile::OnStep(EventArgs& args)
{
	int index = -1;
	args.GetProperty("tileIndex", index, -1);
	if (index == m_index)
	{
		//SoundID sfxID = g_audioSystem->CreateOrGetSound(m_testSfxName);
		//g_audioSystem->StartSound(sfxID);
		g_eventSystem->UnsubscribeEventCallbackObjectMethod("tile:OnStep", *this, &Tile::OnStep);
	}
	return true;
}


void Tile::SelectTile()
{
	m_isSelected = true;;
}


void Tile::DeselectTile()
{
	m_isSelected = false;
}


void Tile::TagTile(Rgba8 const& color)
{
	m_isTagged = true;
	m_taggedColor = color;
}


void Tile::UnTagTile()
{
	m_isTagged = false;
}
