#include "Game/Chunk.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/World.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Math/MathUtils.hpp"

Chunk::Chunk()
{
}


bool Chunk::LoadFromFile()
{
	std::string fileName = "Saves/" + m_chunkName + ".chunk";
	std::vector<uint8_t> fileData;
	bool success = FileReadToBuffer(fileData, fileName);

	if (!success || (int) fileData.size() == 0)
		return false;

	int fileHeaderSize = 8;
	GUARANTEE_OR_DIE((int) fileData.size() >= fileHeaderSize, Stringf("File for chunk does not contain a header of the appropriate size. Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));

	unsigned char g = fileData[0];
	unsigned char c = fileData[1];
	unsigned char h = fileData[2];
	unsigned char k = fileData[3];
	unsigned char v = fileData[4];
	unsigned char x = fileData[5];
	unsigned char y = fileData[6];
	unsigned char z = fileData[7];
	unsigned char s = fileData[8];
	unsigned char seedValue = fileData[9];
	GUARANTEE_OR_DIE(g == 'G', Stringf("First character in chunk file is not G. Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(c == 'C', Stringf("Second character in chunk file is not C. Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(h == 'H', Stringf("Third character in chunk file is not H. Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(k == 'K', Stringf("Fourth character in chunk file is not K. Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(s == 'S', Stringf("Seed (8th) character in chunk file is not S. Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	
	if (x != CHUNK_BITS_X ||
		y != CHUNK_BITS_Y ||
		z != CHUNK_BITS_Z)
	{
		g_console->AddLine(DevConsole::WARNING_MESSAGE, Stringf("Chunk dimensions in file do not match the programmed chunk dims for %s", m_chunkName.c_str()));
		return false;
	}

	if ((int) v != FILE_FORMAT_VERSION)
	{
		g_console->AddLine(DevConsole::WARNING_MESSAGE, Stringf("File format version is incorrect for %s. Version in file: %i | Version in code: %i", m_chunkName.c_str(), (int) v, FILE_FORMAT_VERSION));
		return false;
	}

	if ((int) seedValue != WORLD_SEED)
	{
		g_console->AddLine(DevConsole::WARNING_MESSAGE, Stringf("File's world seed is incorrect for %s. Seed in file: %i | Seed in code: %i", m_chunkName.c_str(), (int) seedValue, WORLD_SEED));
		return false;
	}

	m_blocks = new Block[CHUNK_BLOCK_COUNT_TOTAL];
	int blockIndex = 0;
	for (int byteIndex = 10; byteIndex < (int) fileData.size() - 1; byteIndex += 2)
	{
		unsigned char blockId = fileData[byteIndex];
		int count = (int) fileData[byteIndex + 1];

		BlockDef const* blockDef = BlockDef::GetBlockDefById(blockId);
		GUARANTEE_OR_DIE(blockDef != nullptr, Stringf("Invalid block id %i while loading in Chunk (%i, %i)", (int) blockId, m_chunkCoords.x, m_chunkCoords.y))
		for (int i = 0; i < count; i++)
		{
			m_blocks[blockIndex++].m_blockDefId = blockId;
		}
	}

	GUARANTEE_OR_DIE(blockIndex == CHUNK_BLOCK_COUNT_TOTAL, "Insufficient block data in the chunk file for " + m_chunkName);
	return true;
}


bool Chunk::SaveToFile()
{
	bool success = false;
	std::vector<uint8_t> dataBuffer;

	dataBuffer.push_back('G');
	dataBuffer.push_back('C');
	dataBuffer.push_back('H');
	dataBuffer.push_back('K');
	dataBuffer.push_back((uint8_t) FILE_FORMAT_VERSION);
	dataBuffer.push_back((uint8_t) CHUNK_BITS_X);
	dataBuffer.push_back((uint8_t) CHUNK_BITS_Y);
	dataBuffer.push_back((uint8_t) CHUNK_BITS_Z);
	dataBuffer.push_back('S');
	dataBuffer.push_back((uint8_t) WORLD_SEED);
	
	int blockCount = 0;
	uint8_t blockId = m_blocks[0].m_blockDefId;
	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_COUNT_TOTAL; blockIndex++)
	{
		uint8_t blockDefId = m_blocks[blockIndex].m_blockDefId;
		if (blockDefId != blockId || blockCount == 255)
		{
			BlockDef const* blockDef = BlockDef::GetBlockDefById(blockDefId);
			GUARANTEE_OR_DIE(blockDef != nullptr, Stringf("Invalid block id %i while saving in %s", (int) blockId, m_chunkName.c_str()));
			dataBuffer.push_back(blockId);
			dataBuffer.push_back((uint8_t) blockCount);
			blockId = blockDefId;
			blockCount = 1;
		}
		else
		{
			blockCount++;
		}
	}
	dataBuffer.push_back(blockId);
	dataBuffer.push_back((uint8_t) blockCount);

	std::string fileName = Stringf("Saves/%s.chunk", m_chunkName.c_str());
	success = WriteBufferToFile(dataBuffer, fileName);
	return success;
}


void Chunk::GeneratePerlin()
{
	bool loadFromFile = LoadFromFile();
	if (!loadFromFile)
	{
		GenerateChunk();
	}
	m_isCPUMeshDirty = true;
}


void Chunk::Deactivate(bool shouldSave)
{
	if (m_needsSaving && shouldSave)
	{
		SaveToFile();
	}

	g_theRenderer->DestroyVertexBuffer(m_vbo);

	m_northChunk = nullptr;
	m_southChunk = nullptr;
	m_eastChunk = nullptr;
	m_westChunk = nullptr;

	if (m_blocks)
	{
		delete[] m_blocks;
		m_blocks = nullptr;
	}

	m_verts.clear();
}


void Chunk::Create(World* world, IntVec2 const& chunkCoords)
{
	m_chunkCoords = chunkCoords;
	m_world = world;

	Vec3 worldMins = Vec3((float) chunkCoords.x * CHUNK_DIMS_X, (float) chunkCoords.y * CHUNK_DIMS_Y, 0.0f);
	Vec3 worldMaxs;
	worldMaxs.x = worldMins.x + CHUNK_DIMS_X;
	worldMaxs.y = worldMins.y + CHUNK_DIMS_Y;
	worldMaxs.z = worldMins.z + CHUNK_DIMS_Z;
	m_worldBounds.m_mins = worldMins;
	m_worldBounds.m_maxs = worldMaxs;

	m_chunkName = Stringf("Chunk(%i,%i)", m_chunkCoords.x, m_chunkCoords.y);

	if (m_vbo == nullptr)
	{
		m_vbo = g_theRenderer->CreateDynamicVertexBuffer("Chunk_VBO Coords: " + Stringf("%i, %i", m_chunkCoords.x, m_chunkCoords.y));
	}

	m_chunkStatus = ChunkStatus::CHUNK_STATUS_CREATING;
	m_needsSaving = false;
	m_isCPUMeshDirty = true;
}


void Chunk::InitialiseLightExposures()
{
	//mark all non-opaque edge blocks to existing neighbors as dirty
	for (int x = 0; x < CHUNK_DIMS_X; x++)
	{
		for (int y = 0; y < CHUNK_DIMS_Y; y++)
		{
			for (int z = 0; z < CHUNK_DIMS_Z; z++)
			{
				if ((x == 0 && m_westChunk != nullptr) ||
					(x == CHUNK_DIMS_X - 1 && m_eastChunk != nullptr) ||
					(y == 0 && m_southChunk != nullptr) ||
					(y == CHUNK_DIMS_Y - 1 && m_northChunk != nullptr))
				{
					int blockIndex = GetBlockIndexFromCoords(x, y, z);
					Block block = GetBlockAtIndex(blockIndex);
					bool isOpaque = block.IsOpaque();
					if (!isOpaque)
					{
						BlockIterator blockIterator = BlockIterator(this, blockIndex);
						m_world->AddBlockIteratorToDirtyQueue(blockIterator);
					}
				}

			}
		}
	}

	//mark all non-opaque until opaque blocks sky
	for (int x = 0; x < CHUNK_DIMS_X; x++)
	{
		for (int y = 0; y < CHUNK_DIMS_Y; y++)
		{
			for (int z = CHUNK_DIMS_Z - 1; z >= 0; z--)
			{
				int blockIndex = GetBlockIndexFromCoords(x, y, z);
				Block block = m_blocks[blockIndex];
				if (!m_blocks[blockIndex].IsOpaque())
				{
					m_blocks[blockIndex].SetSkyFlag(true);
				}
				else
					break;
			}
		}
	}

	//mask all nesw non opaque and non sky blocks as dirty and set sky outdoor exposure to 15
	for (int x = 0; x < CHUNK_DIMS_X; x++)
	{
		for (int y = 0; y < CHUNK_DIMS_Y; y++)
		{
			for (int z = CHUNK_DIMS_Z - 1; z >= 0; z--)
			{
				int blockIndex = GetBlockIndexFromCoords(x, y, z);
				if (m_blocks[blockIndex].IsOpaque())
					break;
	
				if (m_blocks[blockIndex].IsSky())
				{
					m_blocks[blockIndex].SetOutdoorLightExposure(BLOCK_MAX_LIGHT_EXPOSURE);

					BlockIterator blockIter = BlockIterator(this, blockIndex);
					BlockIterator northBlockIter = blockIter.GetNorthNeighbor();
					if (northBlockIter.IsValid())
					{
						Block block = northBlockIter.GetBlock();
						if (!block.IsOpaque() && !block.IsSky())
						{
							m_world->AddBlockIteratorToDirtyQueue(northBlockIter);
						}
					}

					BlockIterator southBlockIter = blockIter.GetNorthNeighbor();
					if (southBlockIter.IsValid())
					{
						Block block = southBlockIter.GetBlock();
						if (!block.IsOpaque() && !block.IsSky())
						{
							m_world->AddBlockIteratorToDirtyQueue(southBlockIter);
						}
					}

					BlockIterator eastBlockIter = blockIter.GetSouthNeighbor();
					if (eastBlockIter.IsValid())
					{
						Block block = eastBlockIter.GetBlock();
						if (!block.IsOpaque() && !block.IsSky())
						{
							m_world->AddBlockIteratorToDirtyQueue(eastBlockIter);
						}
					}

					BlockIterator westBlockIter = blockIter.GetNorthNeighbor();
					if (westBlockIter.IsValid())
					{
						Block block = westBlockIter.GetBlock();
						if (!block.IsOpaque() && !block.IsSky())
						{
							m_world->AddBlockIteratorToDirtyQueue(westBlockIter);
						}
					}
				}
				
			}
		}
	}

	//mark all light emitting blocks dirty
	for (int i = 0; i < CHUNK_BLOCK_COUNT_TOTAL; i++)
	{
		if (m_blocks[i].EmitsLight())
		{
			BlockIterator blockIterator = BlockIterator(this, i);
			m_world->AddBlockIteratorToDirtyQueue(blockIterator);
		}
	}
}


void Chunk::GenerateChunk()
{
	Rgba8 infoColor = DevConsole::MAJOR_INFO;

	if (m_blocks)
	{
		delete[] m_blocks;
		m_blocks = nullptr;
	}

	uint8_t grassId		= BlockDef::GetBlockDefIdByName("grass");
	uint8_t airId		= BlockDef::GetBlockDefIdByName("air");
	uint8_t dirtId		= BlockDef::GetBlockDefIdByName("dirt");
	uint8_t stoneId		= BlockDef::GetBlockDefIdByName("stone");
	uint8_t coalId		= BlockDef::GetBlockDefIdByName("coal");
	uint8_t ironId		= BlockDef::GetBlockDefIdByName("iron");
	uint8_t goldId		= BlockDef::GetBlockDefIdByName("gold");
	uint8_t diamondId	= BlockDef::GetBlockDefIdByName("diamond");
	uint8_t waterId		= BlockDef::GetBlockDefIdByName("water");
	uint8_t snowId		= BlockDef::GetBlockDefIdByName("snow");
	uint8_t woodId		= BlockDef::GetBlockDefIdByName("wood");
	uint8_t leafId		= BlockDef::GetBlockDefIdByName("leaf");
	uint8_t sandId		= BlockDef::GetBlockDefIdByName("sand");
	uint8_t snowyDirtId = BlockDef::GetBlockDefIdByName("snowy dirt");
	uint8_t cactusBodyId = BlockDef::GetBlockDefIdByName("cactus body");
	uint8_t cactusHeadId = BlockDef::GetBlockDefIdByName("cactus head");
	uint8_t winterWoodId = BlockDef::GetBlockDefIdByName("winter wood");
	uint8_t winterLeafId = BlockDef::GetBlockDefIdByName("winter leaf");

	GUARANTEE_OR_DIE(grassId	!= -1, Stringf("Invalid block id for block name grass while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(airId		!= -1, Stringf("Invalid block id for block name air while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(dirtId		!= -1, Stringf("Invalid block id for block name dirt while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(stoneId	!= -1, Stringf("Invalid block id for block name stone while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(coalId		!= -1, Stringf("Invalid block id for block name coal while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(ironId		!= -1, Stringf("Invalid block id for block name iron while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(goldId		!= -1, Stringf("Invalid block id for block name gold while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(diamondId	!= -1, Stringf("Invalid block id for block name diamond while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(waterId	!= -1, Stringf("Invalid block id for block name water while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(snowId		!= -1, Stringf("Invalid block id for block name snow while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(woodId		!= -1, Stringf("Invalid block id for block name wood while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(leafId		!= -1, Stringf("Invalid block id for block name leaf while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(sandId		!= -1, Stringf("Invalid block id for block name sand while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(snowyDirtId != -1, Stringf("Invalid block id for block name snowy dirt while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(cactusBodyId != -1, Stringf("Invalid block id for block name cactus body while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(cactusHeadId != -1, Stringf("Invalid block id for block name cactus head while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(winterWoodId != -1, Stringf("Invalid block id for block name winter wood while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));
	GUARANTEE_OR_DIE(winterLeafId != -1, Stringf("Invalid block id for block name winter leaf while generating Chunk (%i, %i)", m_chunkCoords.x, m_chunkCoords.y));

	float temperatureNoise[CHUNK_BLOCK_COUNT_SINGLE_LAYER] = {};
	float humidityNoise[CHUNK_BLOCK_COUNT_SINGLE_LAYER] = {};
	float hillinessMultiplier[CHUNK_BLOCK_COUNT_SINGLE_LAYER] = {};
	for (int x = 0; x < CHUNK_DIMS_X; x++)
	{
		for (int y = 0; y < CHUNK_DIMS_Y; y++)
		{
			float worldX = m_worldBounds.m_mins.x + x;
			float worldY = m_worldBounds.m_mins.y + y;
			int columnIndex = x + y * CHUNK_DIMS_X;
			temperatureNoise[columnIndex]		= Get2dPerlinNoise(worldX, worldY, 100.0f, 2, 0.5f, 2.0f, true, WORLD_SEED) * 0.5f + 0.5f;
			humidityNoise[columnIndex]			= Get2dPerlinNoise(worldX, worldY, 500.0f, 8, 0.7f, 2.0f, true, WORLD_SEED + 2) * 0.5f + 0.5f;
			hillinessMultiplier[columnIndex]	= Get2dPerlinNoise(worldX, worldY, 200.0f, 3, 0.5f, 2.0f, true, WORLD_SEED + 4) * 0.5f + 0.5f;
			hillinessMultiplier[columnIndex]	= SmoothStep3(hillinessMultiplier[columnIndex]);
		}
	}

	float coalProbabilityCutoff = 5.0f;
	float ironProbabilityCutoff = 2.0f;
	float goldProbabilityCutoff = 0.5f;
	float diamondProbabilityCutoff = 0.1f;

	float reallyLowTempCutoff = 0.15f;
	float lowTempCutoff = 0.30f;
	float mediumTempCutoff = 0.45f;
	UNUSED(reallyLowTempCutoff);

	float lowHumidityCutoff = 0.2f;
	float mediumHumidityCutoff = 0.4f;

	int seaLevel = 64;
	m_blocks = new Block[CHUNK_BLOCK_COUNT_TOTAL];
	for (int x = 0; x < CHUNK_DIMS_X; x++)
	{
		for (int y = 0; y < CHUNK_DIMS_Y; y++)
		{
			float worldX = m_worldBounds.m_mins.x + x;
			float worldY = m_worldBounds.m_mins.y + y;
			int columnIndex = x + y * CHUNK_DIMS_X;
			float terrainPerlinMinusOneToOne = Get2dPerlinNoise(float(worldX), float(worldY), 200.f, 6, 0.5f, 2.0f, true, 1);
			float terrainPerlinZeroToOne = (float) fabs(terrainPerlinMinusOneToOne) * hillinessMultiplier[columnIndex];
			int terrainHeight = seaLevel + int(terrainPerlinZeroToOne * 50) - 2;
			int numDirtBlocks = g_rng->GetRandomIntInRange(3, 4);

			float humidity = humidityNoise[columnIndex];
			float temperature = temperatureNoise[columnIndex];
			for (int z = 0; z < CHUNK_DIMS_Z; z++)
			{
				int arrayIndex = GetBlockIndexFromCoords(x, y, z);
				Block& block = m_blocks[arrayIndex];

				if (z > terrainHeight)
				{
					if (z <= seaLevel)
					{
						block.m_blockDefId = waterId;
						if (temperature <= lowTempCutoff)
						{
							block.m_blockDefId = snowId;
						}
					}
					else
					{
						block.m_blockDefId = airId;
					}
					continue;
				}

				if (z == terrainHeight)
				{
					block.m_blockDefId = grassId;
					if (humidity <= mediumHumidityCutoff)
					{
						block.m_blockDefId = sandId;
					}
					continue;
				}

				if (z >= terrainHeight - numDirtBlocks)
				{
					block.m_blockDefId = dirtId;
					if (humidity <= lowHumidityCutoff)
					{
						block.m_blockDefId = sandId;
						continue;
					}
// 					if (temperatureNoise[columnIndex] <= reallyLowTempCutoff)
// 					{
// 						block.m_blockDefId = snowyDirtId;
// 						continue;
// 					}
					continue;
				}

				block.m_blockDefId = stoneId;
				float oreProbability = g_rng->GetRandomFloatInRange(0.0f, 100.0f);
				if (oreProbability < coalProbabilityCutoff)
				{
					block.m_blockDefId = coalId;
					continue;
				}
				oreProbability = g_rng->GetRandomFloatInRange(0.0f, 100.0f);
				if (oreProbability < ironProbabilityCutoff)
				{
					block.m_blockDefId = ironId;
					continue;
				}
				oreProbability = g_rng->GetRandomFloatInRange(0.0f, 100.0f);
				if (oreProbability < goldProbabilityCutoff)
				{
					block.m_blockDefId = goldId;
					continue;
				}
				oreProbability = g_rng->GetRandomFloatInRange(0.0f, 100.0f);
				if (oreProbability < diamondProbabilityCutoff)
				{
					block.m_blockDefId = diamondId;
					continue;
				}

			}

			const int treeNoiseCheckingRadiusX = 2;
			const int treeNoiseCheckingRadiusY = 2;
			bool isLocalMaxima = true;
			float currentTreeDensityNoise = Get2dPerlinNoise(worldX, worldY, 100.0f, 1, 0.5f, 2.0f, true, WORLD_SEED + 6) * 0.5f + 0.5f;
			float currentTreeNoise = Get2dPerlinNoise(worldX, worldY, 500.0f, 7, currentTreeDensityNoise, 2.0f, true, WORLD_SEED + 8) * 0.5f + 0.5f;
			for (int treeX = x - treeNoiseCheckingRadiusX; treeX <= x + treeNoiseCheckingRadiusX; treeX++)
			{
				for (int treeY = y - treeNoiseCheckingRadiusY; treeY <= y + treeNoiseCheckingRadiusY; treeY++)
				{
					float treeWorldX = m_worldBounds.m_mins.x + treeX;
					float treeWorldY = m_worldBounds.m_mins.y + treeY;

					float treeDensityNoise = Get2dPerlinNoise(treeWorldX, treeWorldY, 100.0f, 1, 0.5f, 2.0f, true, WORLD_SEED + 6) * 0.5f + 0.5f;
					float treeNoise = Get2dPerlinNoise(treeWorldX, treeWorldY, 500.0f, 7, treeDensityNoise, 2.0f, true, WORLD_SEED + 8) * 0.5f + 0.5f;

					if (treeNoise >= currentTreeNoise && (treeWorldX != worldX || treeWorldY != worldY))
					{
						isLocalMaxima = false;
						break;
					}
				}

				if (!isLocalMaxima)
					break;
			}

			if (isLocalMaxima && terrainHeight >= seaLevel)
			{
				int arrayIndex1 = GetBlockIndexFromCoords(x, y, terrainHeight + 1);
				int arrayIndex2 = GetBlockIndexFromCoords(x, y, terrainHeight + 2);
				int arrayIndex3 = GetBlockIndexFromCoords(x, y, terrainHeight + 3);
				int arrayIndex4 = GetBlockIndexFromCoords(x, y, terrainHeight + 4);
				m_blocks[arrayIndex1].m_blockDefId = woodId;
				m_blocks[arrayIndex2].m_blockDefId = woodId;
				m_blocks[arrayIndex3].m_blockDefId = woodId;
				m_blocks[arrayIndex4].m_blockDefId = leafId;

				if (temperature <= mediumTempCutoff)
				{
					m_blocks[arrayIndex1].m_blockDefId = winterWoodId;
					m_blocks[arrayIndex2].m_blockDefId = winterWoodId;
					m_blocks[arrayIndex3].m_blockDefId = winterWoodId;
					m_blocks[arrayIndex4].m_blockDefId = winterLeafId;
				}

				if (humidity <= mediumHumidityCutoff)
				{
					m_blocks[arrayIndex1].m_blockDefId = cactusBodyId;
					m_blocks[arrayIndex2].m_blockDefId = cactusBodyId;
					m_blocks[arrayIndex3].m_blockDefId = cactusBodyId;
					m_blocks[arrayIndex4].m_blockDefId = cactusHeadId;
				}
			}
		}
	}
}


bool Chunk::GenerateCPUMesh()
{
	if (g_gameConfigBlackboard.GetValue("debugGenerateMeshWithAllNeighbors", true))
	{
		if (m_northChunk == nullptr || m_southChunk == nullptr || m_eastChunk == nullptr || m_westChunk == nullptr)
			return false;
	}

// 	Rgba8 eastWestTint = Rgba8(235, 235, 235, 255);
// 	Rgba8 northSouthTint = Rgba8(215, 215, 215, 255);
// 	Rgba8 skyHellTint = Rgba8::WHITE;

	m_verts.reserve(15000);
	m_verts.clear();
	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_COUNT_TOTAL; blockIndex++)
	{
		Block block = m_blocks[blockIndex];
		BlockDef const* blockDef = BlockDef::GetBlockDefById(block.m_blockDefId);
		if (blockDef->m_isRendered)
		{
			IntVec3 blockCoords = GetBlockCoordsFromIndex(blockIndex);
			AABB3 blockAABB3 = GetBlockWorldBounds(blockCoords.x, blockCoords.y, blockCoords.z);

			float minX = blockAABB3.m_mins.x;
			float minY = blockAABB3.m_mins.y;
			float minZ = blockAABB3.m_mins.z;
			float maxX = blockAABB3.m_maxs.x;
			float maxY = blockAABB3.m_maxs.y;
			float maxZ = blockAABB3.m_maxs.z;

			IntVec3 southBlockCoords = blockCoords + IntVec3(0, -1, 0);
			AddVertsForBlockQuadInDirection(DIRECTION_SOUTH, blockCoords, m_verts, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), blockDef->m_sideUVs);

			IntVec3 northBlockCoords = blockCoords + IntVec3(0, 1, 0);
			AddVertsForBlockQuadInDirection(DIRECTION_NORTH, blockCoords, m_verts, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), blockDef->m_sideUVs);

			IntVec3 eastBlockCoords = blockCoords + IntVec3(1, 0, 0);
			AddVertsForBlockQuadInDirection(DIRECTION_EAST, blockCoords, m_verts, Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), blockDef->m_sideUVs);

			IntVec3 westBlockCoords = blockCoords + IntVec3(-1, 0, 0);
			AddVertsForBlockQuadInDirection(DIRECTION_WEST, blockCoords, m_verts, Vec3(minX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(minX, maxY, maxZ), Vec3(minX, minY, maxZ), blockDef->m_sideUVs);

			IntVec3 skyBlockCoords = blockCoords + IntVec3(0, 0, 1);
			AddVertsForBlockQuadInDirection(DIRECTION_UP, blockCoords, m_verts, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), blockDef->m_topUVs);

			IntVec3 hellBlockCoords = blockCoords + IntVec3(0, 0, -1);
			AddVertsForBlockQuadInDirection(DIRECTION_DOWN, blockCoords, m_verts, Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), blockDef->m_bottomUVs);
		}
	}

	m_isCPUMeshDirty = false;
	m_chunkStatus = ChunkStatus::CHUNK_STATUS_GPU_MESH_DIRTY;

	return true;
}


void Chunk::Render() const
{
	g_theRenderer->DrawVertexBuffer(m_vbo, (int) m_verts.size());
}


void Chunk::DebugRender() const
{
// 	std::vector<Vertex_PCU> debugVerts;
//  	AddVertsForAABBZ3DToVector(debugVerts, m_worldBounds);
//  	g_theRenderer->DrawVertexArray((int) debugVerts.size(), debugVerts.data());

	for (int i = 0; i < CHUNK_BLOCK_COUNT_TOTAL; i++)
	{
		bool isSky = m_blocks[i].IsSky();
		int indoor = m_blocks[i].GetIndoorLightExposure();
		int outdoor = m_blocks[i].GetOutdoorLightExposure();
		Vec3 blockCenter = GetWorldCenterForBlock(i);


		if (isSky)
		{
			//DebugAddWorldPoint(blockCenter, Rgba8::MAGENTA);
		}

		Mat44 textBasis;
		textBasis.SetTranslation3D(blockCenter);
		textBasis.AppendTranslation3D(Vec3(0.0f, 0.0f, 0.25f));
		DebugAddWorldText(Stringf("%i", indoor), 0.25f, Vec2(0.5f, 0.5f), Rgba8::GREEN, textBasis, 0.0f, DebugRenderMode::ALWAYS);
		textBasis.SetTranslation3D(blockCenter);
		textBasis.AppendTranslation3D(Vec3(0.0f, 0.0f, -0.25f));
		DebugAddWorldText(Stringf("%i", outdoor), 0.25f, Vec2(0.5f, 0.5f), Rgba8::RED, textBasis, 0.0f, DebugRenderMode::ALWAYS);
	}
}


void Chunk::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_chunkStatus == ChunkStatus::CHUNK_STATUS_GPU_MESH_DIRTY)
	{
		m_vbo->CopyVertexArray(m_verts.data(), (int) m_verts.size());
		m_chunkStatus = ChunkStatus::CHUNK_STATUS_ACTIVE;
	}
}


void Chunk::SetChunkStatus(ChunkStatus newStatus)
{
	m_chunkStatus = newStatus;
}


void Chunk::SetBlockDefinitionForBlockCoords(IntVec3 const& blockCoords, uint8_t newBlockDefId)
{
	int blockIndex = GetBlockIndexFromCoords(blockCoords);
	SetBlockDefinitionForBlockIndex(blockIndex, newBlockDefId);
}


void Chunk::SetBlockDefinitionForBlockIndex(int blockIndex, uint8_t newBlockDefId)
{
	if (blockIndex >= 0 && blockIndex < CHUNK_BLOCK_COUNT_TOTAL)
	{
		Block& block = m_blocks[blockIndex];
		block.m_blockDefId = newBlockDefId;
		BlockIterator blockIter = BlockIterator(this, blockIndex);
		m_world->AddBlockIteratorToDirtyQueue(blockIter);
		
		if (block.IsOpaque())
		{
			bool isSky = block.IsSky();
			if (isSky)
			{
				block.SetSkyFlag(false);
				BlockIterator downwardIter = blockIter.GetDownNeighbor();
				while (downwardIter.IsValid())
				{
					bool isBlockOpaque = downwardIter.GetBlock().IsOpaque();
					if(isBlockOpaque)
						break;

					m_world->AddBlockIteratorToDirtyQueue(downwardIter);
					downwardIter.m_chunk->SetSkyFlagForBlock(downwardIter.m_blockIndex, false);

					downwardIter = downwardIter.GetDownNeighbor();
				}
			}
		}
		else
		{
			BlockIterator upBlockIter = blockIter.GetUpNeighbor();
			if (upBlockIter.IsValid() && upBlockIter.GetBlock().IsSky())
			{
				block.SetSkyFlag(true);
				BlockIterator downwardIter = blockIter.GetDownNeighbor();
				while (downwardIter.IsValid())
				{
					bool isBlockOpaque = downwardIter.GetBlock().IsOpaque();
					if (isBlockOpaque)
						break;

					m_world->AddBlockIteratorToDirtyQueue(downwardIter);
					downwardIter.m_chunk->SetSkyFlagForBlock(downwardIter.m_blockIndex, true);

					downwardIter = downwardIter.GetDownNeighbor();
				}
			}
		}


		m_needsSaving = true;
		m_isCPUMeshDirty = true;
	}
}


Chunk* Chunk::GetNorthChunk()
{
	return m_northChunk;
}


Chunk* Chunk::GetSouthChunk()
{
	return m_southChunk;
}


Chunk* Chunk::GetEastChunk()
{
	return m_eastChunk;
}


Chunk* Chunk::GetWestChunk()
{
	return m_westChunk;
}


void Chunk::SetNorthChunk(Chunk* northChunk)
{
	m_northChunk = northChunk;
	if (g_gameConfigBlackboard.GetValue("newNeighborDirtiesMesh", false))
		MarkCPUMeshDirty();

	if (g_gameConfigBlackboard.GetValue("debugNewNeighborRebuildsLightExposure", false))
		InitialiseLightExposures();
}


void Chunk::SetSouthChunk(Chunk* southChunk)
{
	m_southChunk = southChunk;
	if (g_gameConfigBlackboard.GetValue("newNeighborDirtiesMesh", false))
		MarkCPUMeshDirty();

	if (g_gameConfigBlackboard.GetValue("debugNewNeighborRebuildsLightExposure", false))
		InitialiseLightExposures();
}


void Chunk::SetEastChunk(Chunk* eastChunk)
{
	m_eastChunk = eastChunk;
	if (g_gameConfigBlackboard.GetValue("newNeighborDirtiesMesh", false))
		MarkCPUMeshDirty();

	if (g_gameConfigBlackboard.GetValue("debugNewNeighborRebuildsLightExposure", false))
		InitialiseLightExposures();
}


void Chunk::SetWestChunk(Chunk* westChunk)
{
	m_westChunk = westChunk;
	if (g_gameConfigBlackboard.GetValue("newNeighborDirtiesMesh", false))
		MarkCPUMeshDirty();

	if (g_gameConfigBlackboard.GetValue("debugNewNeighborRebuildsLightExposure", false))
		InitialiseLightExposures();
}


void Chunk::MarkCPUMeshDirty()
{
	m_isCPUMeshDirty = true;
}


bool Chunk::SetSkyFlagForBlock(int blockIndex, bool isSky)
{
	if (blockIndex >= 0 && blockIndex < CHUNK_BLOCK_COUNT_TOTAL)
	{
		m_blocks[blockIndex].SetSkyFlag(isSky);
		return true;
	}

	return false;
}


bool Chunk::SetDirtyLightFlagForBlock(int blockIndex, bool isLightDirty)
{
	if (blockIndex >= 0 && blockIndex < CHUNK_BLOCK_COUNT_TOTAL)
	{
		m_blocks[blockIndex].SetDirtyLightFlag(isLightDirty);
		return true;
	}

	return false;
}


bool Chunk::SetIndoorLightExposureForBlock(int blockIndex, int indoorLightExposure)
{
	if (blockIndex >= 0 && blockIndex < CHUNK_BLOCK_COUNT_TOTAL)
	{
		m_blocks[blockIndex].SetIndoorLightExposure(indoorLightExposure);
		return true;
	}

	return false;
}


bool Chunk::SetOutoorLightExposureForBlock(int blockIndex, int outoorLightExposure)
{
	if (blockIndex >= 0 && blockIndex < CHUNK_BLOCK_COUNT_TOTAL)
	{
		m_blocks[blockIndex].SetOutdoorLightExposure(outoorLightExposure);
		return true;
	}

	return false;
}


Block Chunk::GetBlockAtIndex(int index)
{
	return m_blocks[index];
}


Block Chunk::GetBlockAtCoords(int x, int y, int z)
{
	int index = GetBlockIndexFromCoords(x, y, z);
	return GetBlockAtIndex(index);
}


int Chunk::GetBlockIndexFromCoords(int x, int y, int z) const
{
	int indexZ = z << (CHUNK_BITS_X + CHUNK_BITS_Y);
	int indexY = y << (CHUNK_BITS_X);
	int indexX = x;

	return indexZ | indexY | indexX;
}


int Chunk::GetBlockIndexFromCoords(IntVec3 const& blockCoords) const
{
	int indexZ = blockCoords.z << (CHUNK_BITS_X + CHUNK_BITS_Y);
	int indexY = blockCoords.y << (CHUNK_BITS_X);
	int indexX = blockCoords.x;

	return indexZ | indexY | indexX;
}


IntVec3 Chunk::GetBlockCoordsFromIndex(int arrayIndex) const
{
	IntVec3 blockCoords;
	blockCoords.x = arrayIndex & CHUNK_BITMASK_X;
	int indexY = arrayIndex & CHUNK_BITMASK_Y;
	blockCoords.y = indexY >> CHUNK_BITS_X;
	int indexZ = arrayIndex & CHUNK_BITMASK_Z;
	blockCoords.z = indexZ >> (CHUNK_BITS_X + CHUNK_BITS_Y);
	return blockCoords;
}


IntVec3 Chunk::GetLocalBlockCoordsForWorldPos(Vec3 const& worldPos) const
{
	int localXCoord = RoundDownToInt(worldPos.x - m_chunkCoords.x * CHUNK_DIMS_X);
	int localYCoord = RoundDownToInt(worldPos.y - m_chunkCoords.y * CHUNK_DIMS_Y);
	int localZCoord = RoundDownToInt(worldPos.z);
	localZCoord = Clamp(localZCoord, 0, CHUNK_DIMS_Z - 1);

	return IntVec3(localXCoord, localYCoord, localZCoord);
}


int Chunk::GetBlockIndexForWorldPos(Vec3 const& worldPos) const
{
	IntVec3 blockCoords = GetLocalBlockCoordsForWorldPos(worldPos);
	return GetBlockIndexFromCoords(blockCoords);
}


uint8_t Chunk::GetBlockDefIdForBlockCoords(IntVec3 const& blockCoords) const
{
	int blockIndex = GetBlockIndexFromCoords(blockCoords);
	GUARANTEE_OR_DIE(blockIndex >= 0 && blockIndex < CHUNK_BLOCK_COUNT_TOTAL, 
					 Stringf("Tried to access block def for invalid block coords: %i, %i, %i in %s", blockCoords.x, blockCoords.y, blockCoords.z, m_chunkName.c_str()));
	return m_blocks[blockIndex].m_blockDefId;
}


ChunkStatus Chunk::GetChunkStatus() const
{
	return m_chunkStatus;
}


std::string Chunk::GetChunkName() const
{
	return m_chunkName;
}


void Chunk::AddVertsForBlockQuadInDirection(Directions direction, IntVec3 const& blockCoords, std::vector<Vertex_PCU>& out_vector, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topLeft, Vec3 const& topRight, AABB2 const& UVs)
{
	int blockIndex = GetBlockIndexFromCoords(blockCoords);
	BlockIterator blockIterator = BlockIterator(this, blockIndex);
	BlockIterator neighboringBlockIterator = BlockIterator::INVALID;
	switch (direction)
	{
		case DIRECTION_NORTH:
			neighboringBlockIterator = blockIterator.GetNorthNeighbor();
			break;
		case DIRECTION_SOUTH:
			neighboringBlockIterator = blockIterator.GetSouthNeighbor();
			break;
		case DIRECTION_EAST:
			neighboringBlockIterator = blockIterator.GetEastNeighbor();
			break;
		case DIRECTION_WEST:
			neighboringBlockIterator = blockIterator.GetWestNeighbor();
			break;
		case DIRECTION_UP:
			neighboringBlockIterator = blockIterator.GetUpNeighbor();
			break;
		case DIRECTION_DOWN:
			neighboringBlockIterator = blockIterator.GetDownNeighbor();
			break;
		default:
			break;
	}

	if (!neighboringBlockIterator.IsValid() || neighboringBlockIterator.GetBlock().IsOpaque())
		return;

	unsigned char outdoorLightByte = 255;
	unsigned char indoorLightByte = 255;
	Block const& neighboringBlock = neighboringBlockIterator.GetBlock();

	int outdoorLightExposure = neighboringBlock.GetOutdoorLightExposure();
	float normalisedOutdoorLightExposure = (float) outdoorLightExposure / BLOCK_MAX_LIGHT_EXPOSURE;
	outdoorLightByte = FloatToByte(normalisedOutdoorLightExposure);

	int indoorLightExposure = neighboringBlock.GetIndoorLightExposure();
	float normalisedIndoorLightExposure = (float) indoorLightExposure / BLOCK_MAX_LIGHT_EXPOSURE;
	indoorLightByte = FloatToByte(normalisedIndoorLightExposure);

	Vec2 topRightUV = UVs.m_maxs;
	Vec2 topLeftUV = Vec2(UVs.m_mins.x, UVs.m_maxs.y);
	Vec2 bottomRightUV = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 bottomLeftUV = UVs.m_mins;

	Rgba8 vertexColor;
	vertexColor.r = outdoorLightByte;
	vertexColor.g = indoorLightByte;
	vertexColor.b = 255;
	vertexColor.a = 255;
	if (g_gameConfigBlackboard.GetValue("renderWithoutLighting", false))
		vertexColor = Rgba8::WHITE;
	out_vector.push_back(Vertex_PCU(bottomLeft, vertexColor, bottomLeftUV));
	out_vector.push_back(Vertex_PCU(bottomRight, vertexColor, bottomRightUV));
	out_vector.push_back(Vertex_PCU(topLeft, vertexColor, topLeftUV));

	out_vector.push_back(Vertex_PCU(bottomRight, vertexColor, bottomRightUV));
	out_vector.push_back(Vertex_PCU(topRight, vertexColor, topRightUV));
	out_vector.push_back(Vertex_PCU(topLeft, vertexColor, topLeftUV));
}


AABB3 Chunk::GetBlockWorldBounds(int x, int y, int z) const
{
	AABB3 blockAABB3;
	float xFloat = (float) x;
	float yFloat = (float) y;
	float zFloat = (float) z;
	blockAABB3.m_mins.x = m_worldBounds.m_mins.x + xFloat;
	blockAABB3.m_mins.y = m_worldBounds.m_mins.y + yFloat;
	blockAABB3.m_mins.z = m_worldBounds.m_mins.z + zFloat;
	blockAABB3.m_maxs.x = m_worldBounds.m_mins.x + xFloat + 1.0f;
	blockAABB3.m_maxs.y = m_worldBounds.m_mins.y + yFloat + 1.0f;
	blockAABB3.m_maxs.z = m_worldBounds.m_mins.z + zFloat + 1.0f;
	return blockAABB3;
}


AABB3 Chunk::GetBlockWorldBounds(int blockIndex)
{
	IntVec3 blockCoords = GetBlockCoordsFromIndex(blockIndex);
	return GetBlockWorldBounds(blockCoords.x, blockCoords.y, blockCoords.z);
}


bool Chunk::IsBlockOpaque(IntVec3 const& blockCoords) const
{
	int blockIndex = GetBlockIndexFromCoords(blockCoords);
	Block block = m_blocks[blockIndex];
	BlockDef const* blockDef = BlockDef::GetBlockDefById(block.m_blockDefId);
	GUARANTEE_OR_DIE(blockDef != nullptr, Stringf("Invalid block def id %i. Chunk(%i, %i)", block.m_blockDefId, m_chunkCoords.x, m_chunkCoords.y));
	return blockDef->m_isOpaque;
}


bool Chunk::IsMeshDirty() const
{
	return m_isCPUMeshDirty;
}


bool Chunk::NeedsSaving() const
{
	return m_needsSaving;
}


Vec3 Chunk::GetWorldCenter() const
{
	return m_worldBounds.GetCenter();
}


Vec3 Chunk::GetWorldCenterForBlock(int blockIndex) const
{
	IntVec3 blockCoords = GetBlockCoordsFromIndex(blockIndex);
	return m_worldBounds.m_mins + Vec3(blockCoords.x + 0.5f, blockCoords.y + 0.5f, blockCoords.z + 0.5f);
}


IntVec2 Chunk::GetChunkCoords() const
{
	return m_chunkCoords;
}
