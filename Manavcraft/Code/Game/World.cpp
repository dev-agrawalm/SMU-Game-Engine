#include "Game/World.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "ThirdParty/SquirrelNoise/SmoothNoise.hpp"
#include "Game/GameCamera.hpp"
#include "Game/Game.hpp"
#include <deque>
#include "Engine/Core/DevConsole.hpp"
#include <mutex>
#include<filesystem>

struct GameShaderConstants
{
	Vec4 m_cameraPos;
	
	float m_fogColor[4];
	float m_indoorLightColor[4];
	float m_outdoorLightColor[4];

	float m_fogMinDistance;
	float m_fogMaxDistance;
	float m_fogMaxIntensity;
	float m_time;
};

ConstantBuffer* g_cbo = nullptr;

//physics variables
float g_gravity = 0.0f;
float g_horizontalFriction = 0.0f;

std::mutex g_perlinMutex;
std::vector<Chunk*> g_chunksQueuedForPerlinGeneration;

std::mutex g_fileIOMutex;
std::vector<Chunk*> g_chunksQueuedForFileIO;

void GenerateChunkPerlinWorkerThread(std::string const& threadId)
{
	UNUSED(threadId);
	while (g_isQuitting == 0)
	{
		float currentTime1 = (float) GetCurrentTimeSeconds();
		Chunk* chunkToGenerate = nullptr;
		g_perlinMutex.lock();
		for (int chunkIndex = 0; chunkIndex < g_chunksQueuedForPerlinGeneration.size(); chunkIndex++)
		{
			Chunk*& chunk = g_chunksQueuedForPerlinGeneration[chunkIndex];
			if (chunk->GetChunkStatus() == ChunkStatus::CHUNK_STATUS_GENERATING_PERLIN_QUEUED)
			{
				chunkToGenerate = chunk;
				chunkToGenerate->SetChunkStatus(ChunkStatus::CHUNK_STATUS_GENERATING_PERLIN);
				break;
			}
		}
		g_perlinMutex.unlock();

		if (chunkToGenerate != nullptr)
		{
			chunkToGenerate->GeneratePerlin();
			chunkToGenerate->SetChunkStatus(ChunkStatus::CHUNK_STATUS_CREATED);
		}
		else
		{
			std::this_thread::yield();
		}

		float currentTime2 = (float) GetCurrentTimeSeconds();
		//g_console->AddLine(DevConsole::MINOR_INFO, Stringf("%s:Time for one iter: %.5f", threadId.c_str(), currentTime2 - currentTime1));
	}
}


void ChunkLoadingSavingWorkerThread(std::string const& threadId)
{
	UNUSED(threadId);
	while (g_isQuitting == 0)
	{
		float currentTime1 = (float) GetCurrentTimeSeconds();
		Chunk* chunkToLoad = nullptr;
		g_fileIOMutex.lock();
		for (int chunkIndex = 0; chunkIndex < g_chunksQueuedForFileIO.size(); chunkIndex++)
		{
			Chunk*& chunk = g_chunksQueuedForFileIO[chunkIndex];
			if (chunk->GetChunkStatus() == ChunkStatus::CHUNK_STATUS_LOAD_FROM_FILE_QUEUED)
			{
				chunkToLoad = chunk;
				chunkToLoad->SetChunkStatus(ChunkStatus::CHUNK_STATUS_LOADING_FROM_FILE);
				break;
			}
		}
		g_fileIOMutex.unlock();

		bool wasChunkLoaded = false;
		if (chunkToLoad != nullptr)
		{
			wasChunkLoaded = chunkToLoad->LoadFromFile();
			if (wasChunkLoaded)
			{
				chunkToLoad->SetChunkStatus(ChunkStatus::CHUNK_STATUS_CREATED);
			}
			else
			{
				chunkToLoad->SetChunkStatus(ChunkStatus::CHUNK_STATUS_GENERATING_PERLIN_QUEUED);
			}
		}
		
		bool wasChunkSaved = false;
		if(!wasChunkLoaded)//if not load
		{
			Chunk* chunkToSave = nullptr;
			g_fileIOMutex.lock();
			for (int chunkIndex = 0; chunkIndex < g_chunksQueuedForFileIO.size(); chunkIndex++)
			{
				Chunk*& chunk = g_chunksQueuedForFileIO[chunkIndex];
				if (chunk->GetChunkStatus() == ChunkStatus::CHUNK_STATUS_SAVING_QUEUED)
				{
					chunkToSave = chunk;
					chunkToSave->SetChunkStatus(ChunkStatus::CHUNK_STATUS_SAVING);
					break;
				}
			}
			g_fileIOMutex.unlock();

			if (chunkToSave != nullptr)
			{
				wasChunkSaved = chunkToSave->SaveToFile();
				if (wasChunkSaved)
				{
					chunkToSave->SetChunkStatus(ChunkStatus::CHUNK_STATUS_SAVED);
				}
				else
				{
					g_console->AddLine(DevConsole::ERROR_MESSAGE, Stringf("Unable to save %s to disk", chunkToSave->GetChunkName().c_str()));
				}
			}
		}

		if (!wasChunkLoaded && !wasChunkSaved)
		{
			std::this_thread::yield();
		}

		float currentTime2 = (float) GetCurrentTimeSeconds();
		//g_console->AddLine(DevConsole::MINOR_INFO, Stringf("%s:Time for one iter: %.5f", threadId.c_str(), currentTime2 - currentTime1));
	}
}



World::World(Player* player)
	: m_player(player)
{

}


void World::Create()
{
	g_gravity = g_gameConfigBlackboard.GetValue("gravity", 10.0f);
	g_horizontalFriction = g_gameConfigBlackboard.GetValue("horizontalFriction", 4.0f);

	m_chunkActivationDistance = g_gameConfigBlackboard.GetValue("chunkActivationDistance", m_chunkActivationDistance);
	m_chunkDeactivationDistance = m_chunkActivationDistance + CHUNK_DIMS_X + CHUNK_DIMS_Y;

	int maxChunksRadiusX = 1 + (int) m_chunkActivationDistance / CHUNK_DIMS_X;
	int maxChunksRadiusY = 1 + (int) m_chunkActivationDistance / CHUNK_DIMS_Y;
	m_maxAllowedChunks = g_gameConfigBlackboard.GetValue("maxChunkCount", -1); 
	if (m_maxAllowedChunks <= 0)
	{
		m_maxAllowedChunks = (2 * maxChunksRadiusX) * (2 * maxChunksRadiusY);
	}
	m_numActiveChunk = 0;

	m_debugRenderChunks = g_gameConfigBlackboard.GetValue("debugRenderChunks", false);
	g_cbo = g_theRenderer->CreateConstantBuffer(sizeof(GameShaderConstants));

	g_threads[0] = new std::thread(ChunkLoadingSavingWorkerThread, "Chunk File I/O Thread #:1");
	g_threads[1] = new std::thread(GenerateChunkPerlinWorkerThread, "Perlin Generation Thread #:1");
	g_threads[2] = new std::thread(GenerateChunkPerlinWorkerThread, "Perlin Generation Thread #:2");
	g_threads[3] = new std::thread(GenerateChunkPerlinWorkerThread, "Perlin Generation Thread #:3");
	g_threads[4] = new std::thread(GenerateChunkPerlinWorkerThread, "Perlin Generation Thread #:4");
	g_threads[5] = new std::thread(GenerateChunkPerlinWorkerThread, "Perlin Generation Thread #:5");
	g_threads[6] = new std::thread(GenerateChunkPerlinWorkerThread, "Perlin Generation Thread #:6");
	g_threads[7] = new std::thread(GenerateChunkPerlinWorkerThread, "Perlin Generation Thread #:7");
}


void World::Destroy()
{
	g_theRenderer->DestroyConstantBuffer(g_cbo);

	auto it = m_activatedChunks.begin();
	while (it != m_activatedChunks.end())
	{
		Chunk*& chunk = it->second;
		if (chunk)
		{
			chunk->Deactivate(true);
			delete chunk;
			chunk = nullptr;
		}

		it++;
	}
}


void World::CheckInput()
{
	if (g_inputSystem->WasKeyJustPressed('U'))
	{
		auto it = m_activatedChunks.begin();
		while (it != m_activatedChunks.end())
		{
			Chunk*& chunk = it->second;
			if (chunk)
			{
				chunk->Deactivate();
				delete chunk;
				chunk = nullptr;
			}

			it++;
		}

		m_activatedChunks.clear();
		m_numActiveChunk = 0;
	}

	Vec3 playerPos = m_player->GetPosition();
	if (g_inputSystem->WasMouseButtonJustPressed(0)) // dig
	{
		if (m_raycastResult.m_didHit)
		{
			BlockIterator hitBlockIter = m_raycastResult.m_hitBlockIterator;
			Chunk* chunk = hitBlockIter.m_chunk;
			uint8_t airId = BlockDef::GetBlockDefIdByName("air");
			chunk->SetBlockDefinitionForBlockIndex(hitBlockIter.m_blockIndex, airId);
		}
	}

	if (g_inputSystem->WasMouseButtonJustPressed(1)) //place brick
	{
		if (m_raycastResult.m_didHit)
		{
			BlockIterator hitBlockIter = m_raycastResult.m_hitBlockIterator;
			uint8_t placementBlockDef = m_player->GetPlacementBlockId();
			switch (m_raycastResult.m_hitSurfaceDirection)
			{
				case DIRECTION_EAST:
				{
					BlockIterator neighboringBlockIter = hitBlockIter.GetEastNeighbor();
					if (neighboringBlockIter.IsValid())
					{
						Chunk* chunk = neighboringBlockIter.m_chunk;
						chunk->SetBlockDefinitionForBlockIndex(neighboringBlockIter.m_blockIndex, placementBlockDef);
					}
					break;
				}
				case DIRECTION_WEST:
				{
					BlockIterator neighboringBlockIter = hitBlockIter.GetWestNeighbor();
					if (neighboringBlockIter.IsValid())
					{
						Chunk* chunk = neighboringBlockIter.m_chunk;
						chunk->SetBlockDefinitionForBlockIndex(neighboringBlockIter.m_blockIndex, placementBlockDef);
					}
					break;
				}
				case DIRECTION_NORTH:
				{
					BlockIterator neighboringBlockIter = hitBlockIter.GetNorthNeighbor();
					if (neighboringBlockIter.IsValid())
					{
						Chunk* chunk = neighboringBlockIter.m_chunk;
						chunk->SetBlockDefinitionForBlockIndex(neighboringBlockIter.m_blockIndex, placementBlockDef);
					}
					break;
				}
				case DIRECTION_SOUTH:
				{
					BlockIterator neighboringBlockIter = hitBlockIter.GetSouthNeighbor();
					if (neighboringBlockIter.IsValid())
					{
						Chunk* chunk = neighboringBlockIter.m_chunk;
						chunk->SetBlockDefinitionForBlockIndex(neighboringBlockIter.m_blockIndex, placementBlockDef);
					}
					break;
				}
				case DIRECTION_UP:
				{
					BlockIterator neighboringBlockIter = hitBlockIter.GetUpNeighbor();
					if (neighboringBlockIter.IsValid())
					{
						Chunk* chunk = neighboringBlockIter.m_chunk;
						chunk->SetBlockDefinitionForBlockIndex(neighboringBlockIter.m_blockIndex, placementBlockDef);
					}
					break;
				}
				case DIRECTION_DOWN:
				{
					BlockIterator neighboringBlockIter = hitBlockIter.GetDownNeighbor();
					if (neighboringBlockIter.IsValid())
					{
						Chunk* chunk = neighboringBlockIter.m_chunk;
						chunk->SetBlockDefinitionForBlockIndex(neighboringBlockIter.m_blockIndex, placementBlockDef);
					}
					break;
				}
				default:
					break;
			}
		}
	}

	if (g_inputSystem->WasKeyJustPressed('R'))
	{
		m_isRaycastLocked = !m_isRaycastLocked;
	}
}


void World::FindSingleChunkToBeCreated()
{
	Vec3 playerPos = m_player->GetPosition();
	IntVec2 chunkCoordsRange;
	chunkCoordsRange.x = (int) m_chunkActivationDistance / CHUNK_DIMS_X;
	chunkCoordsRange.y = (int) m_chunkActivationDistance / CHUNK_DIMS_Y;
	IntVec2 centerChunkCoords = GetChunkCoordsForWorldPos(playerPos);
	IntVec2 minChunkCoords = centerChunkCoords - chunkCoordsRange;
	IntVec2 maxChunkCoords = centerChunkCoords + chunkCoordsRange;

	IntVec2 chunkCoordsToBeCreated;
	float distanceFromNearestChunk = 99999.0f;
	for (int chunkCoordX = minChunkCoords.x; chunkCoordX <= maxChunkCoords.x; chunkCoordX++)
	{
		for (int chunkCoordY = minChunkCoords.y; chunkCoordY <= maxChunkCoords.y; chunkCoordY++)
		{
			g_fileIOMutex.lock();
			if (g_chunksQueuedForFileIO.size() >= NUM_CHUNKS_TO_CREATE_PER_FRAME)
			{
				g_fileIOMutex.unlock();
				return;
			}
			g_fileIOMutex.unlock();

			AABB3 chunkWorldBounds = GetChunkWorldBounds(chunkCoordX, chunkCoordY);
			Vec3 nearestPointOnChunk = GetNearestPointOnAABB3(playerPos, chunkWorldBounds);
			float distanceFromChunkXY = (nearestPointOnChunk - playerPos).GetLengthXY();
			if (distanceFromChunkXY > m_chunkActivationDistance)
				continue;

			if (distanceFromChunkXY < distanceFromNearestChunk)
			{
				IntVec2 chunkCoords = IntVec2(chunkCoordX, chunkCoordY);
				if (!IsChunkActivated(chunkCoords) && !DoesChunkExistInPerlinQueue(chunkCoords) && !DoesChunkExistInFileIOQueue(chunkCoords))
				{
					distanceFromNearestChunk = distanceFromChunkXY;
					chunkCoordsToBeCreated = chunkCoords;
				}
			}
		}
	}

	if (distanceFromNearestChunk != 99999.0f)
	{
		Chunk* chunk = new Chunk();
		chunk->Create(this, chunkCoordsToBeCreated);
		g_fileIOMutex.lock();
		if (g_chunksQueuedForFileIO.size() < NUM_CHUNKS_TO_CREATE_PER_FRAME)
		{
			chunk->SetChunkStatus(ChunkStatus::CHUNK_STATUS_LOAD_FROM_FILE_QUEUED);
			g_chunksQueuedForFileIO.push_back(chunk);
		}
		g_fileIOMutex.unlock();
	}
}


void World::FindSingleChunkToBeDeactivated()
{
	Chunk* chunkToDeactivate = nullptr;
	Vec3 playerPos = m_player->GetPosition();

	IntVec2 chunkCoordsToBeDeactivated;
	float distanceFromFurthestChunk = 0.0f;
	auto it = m_activatedChunks.begin();
	while (it != m_activatedChunks.end())
	{
		Chunk*& chunk = it->second;
		if (chunk != nullptr)
		{
			Vec3 chunkCenter = chunk->GetWorldCenter();
			float distanceFromChunkCenterXY = (chunkCenter - playerPos).GetLengthXY();

			if (distanceFromChunkCenterXY >= m_chunkDeactivationDistance && distanceFromChunkCenterXY > distanceFromFurthestChunk)
			{
				distanceFromFurthestChunk = distanceFromChunkCenterXY;
				chunkToDeactivate = chunk;
				chunkCoordsToBeDeactivated = chunkToDeactivate->GetChunkCoords();
			}
		}

		it++;
	}

	if (chunkToDeactivate != nullptr && !chunkToDeactivate->NeedsSaving() && !DoesChunkExistInFileIOQueue(chunkCoordsToBeDeactivated))
	{
		DeactivateChunk(chunkToDeactivate);
	}
}


void World::FindSingleChunkToBeSaved()
{
	Chunk* chunkToSave = nullptr;
	Vec3 playerPos = m_player->GetPosition();

	IntVec2 chunkCoordsToBeSaved;
	float distanceFromFurthestChunk = 0.0f;
	auto it = m_activatedChunks.begin();
	while (it != m_activatedChunks.end())
	{
		Chunk*& chunk = it->second;
		if (chunk != nullptr)
		{
			Vec3 chunkCenter = chunk->GetWorldCenter();
			float distanceFromChunkCenterXY = (chunkCenter - playerPos).GetLengthXY();

			if (distanceFromChunkCenterXY >= m_chunkDeactivationDistance && distanceFromChunkCenterXY > distanceFromFurthestChunk)
			{
				distanceFromFurthestChunk = distanceFromChunkCenterXY;
				chunkToSave = chunk;
				chunkCoordsToBeSaved = chunkToSave->GetChunkCoords();
			}
		}

		it++;
	}

	if (chunkToSave != nullptr && chunkToSave->NeedsSaving() && !DoesChunkExistInFileIOQueue(chunkCoordsToBeSaved))
	{
		chunkToSave->SetChunkStatus(ChunkStatus::CHUNK_STATUS_SAVING_QUEUED);
		g_fileIOMutex.lock();
		g_chunksQueuedForFileIO.push_back(chunkToSave);
		g_fileIOMutex.unlock();
	}
}


void World::ClaimSingleCreatedChunkForActivation()
{
	Chunk* chunkToActivate = nullptr;

	g_fileIOMutex.lock();
	auto chunkIter = g_chunksQueuedForFileIO.begin();
	while (chunkIter != g_chunksQueuedForFileIO.end())
	{
		if ((*chunkIter)->GetChunkStatus() == ChunkStatus::CHUNK_STATUS_CREATED)
		{
			chunkToActivate = *chunkIter;
			g_chunksQueuedForFileIO.erase(chunkIter);
			break;
		}
		chunkIter++;
	}
	g_fileIOMutex.unlock();

	if (chunkToActivate == nullptr)
	{
		g_perlinMutex.lock();
		if (g_chunksQueuedForPerlinGeneration.size() > 0)
		{
			chunkIter = g_chunksQueuedForPerlinGeneration.begin();
			while (chunkIter != g_chunksQueuedForPerlinGeneration.end())
			{
				if ((*chunkIter)->GetChunkStatus() == ChunkStatus::CHUNK_STATUS_CREATED)
				{
					chunkToActivate = *chunkIter;
					g_chunksQueuedForPerlinGeneration.erase(chunkIter);
					break;
				}
				chunkIter++;
			}
		}
		g_perlinMutex.unlock();
	}

	if (chunkToActivate != nullptr)
	{
		ActivateChunk(chunkToActivate);
	}
}


void World::ActivateChunk(Chunk* chunk)
{
	if (chunk == nullptr)
		return;

	IntVec2 chunkCoords = chunk->GetChunkCoords();

	IntVec2 northChunkCoords = IntVec2(0, 1) + chunkCoords;
	if (IsChunkActivated(northChunkCoords))
	{
		Chunk*& northChunk = m_activatedChunks[northChunkCoords];
		northChunk->SetSouthChunk(chunk);
		chunk->SetNorthChunk(northChunk);
	}
	IntVec2 southChunkCoords = IntVec2(0, -1) + chunkCoords;
	if (IsChunkActivated(southChunkCoords))
	{
		Chunk*& southChunk = m_activatedChunks[southChunkCoords];
		southChunk->SetNorthChunk(chunk);
		chunk->SetSouthChunk(southChunk);
	}
	IntVec2 eastChunkCoords = IntVec2(1, 0) + chunkCoords;
	if (IsChunkActivated(eastChunkCoords))
	{
		Chunk*& eastChunk = m_activatedChunks[eastChunkCoords];
		eastChunk->SetWestChunk(chunk);
		chunk->SetEastChunk(eastChunk);
	}
	IntVec2 westChunkCoords = IntVec2(-1, 0) + chunkCoords;
	if (IsChunkActivated(westChunkCoords))
	{
		Chunk*& westChunk = m_activatedChunks[westChunkCoords];
		westChunk->SetEastChunk(chunk);
		chunk->SetWestChunk(westChunk);
	}

	if (g_gameConfigBlackboard.GetValue("debugInitialiseLightingOnChunkActivation", true))
		chunk->InitialiseLightExposures();

	m_activatedChunks[chunkCoords] = chunk;
	m_activatedChunks[chunkCoords]->SetChunkStatus(ChunkStatus::CHUNK_STATUS_ACTIVE);
	m_numActiveChunk++;
}


void World::DeactivateChunk(Chunk* chunkToDeactivate)
{
	IntVec2 chunkCoords = chunkToDeactivate->GetChunkCoords();
	IntVec2 northChunkCoords = IntVec2(0, 1) + chunkCoords;
	if (IsChunkActivated(northChunkCoords))
	{
		Chunk*& northChunk = m_activatedChunks[northChunkCoords];
		northChunk->SetSouthChunk(nullptr);
	}
	IntVec2 southChunkCoords = IntVec2(0, -1) + chunkCoords;
	if (IsChunkActivated(southChunkCoords))
	{
		Chunk*& southChunk = m_activatedChunks[southChunkCoords];
		southChunk->SetNorthChunk(nullptr);
	}
	IntVec2 eastChunkCoords = IntVec2(1, 0) + chunkCoords;
	if (IsChunkActivated(eastChunkCoords))
	{
		Chunk*& eastChunk = m_activatedChunks[eastChunkCoords];
		eastChunk->SetWestChunk(nullptr);
	}
	IntVec2 westChunkCoords = IntVec2(-1, 0) + chunkCoords;
	if (IsChunkActivated(westChunkCoords))
	{
		Chunk*& westChunk = m_activatedChunks[westChunkCoords];
		westChunk->SetEastChunk(nullptr);
	}

	UnDirtyAllBlocksForChunk(chunkCoords);

	chunkToDeactivate->Deactivate();
	chunkToDeactivate->SetChunkStatus(ChunkStatus::CHUNK_STATUS_DEACTIVATED);
	delete chunkToDeactivate;
	chunkToDeactivate = nullptr;
	auto chunkIter = m_activatedChunks.find(chunkCoords);
	m_activatedChunks.erase(chunkIter);
	m_numActiveChunk--;
}


void World::DirtyAllChunkMeshes()
{
	auto it = m_activatedChunks.begin();
	while (it != m_activatedChunks.end())
	{
		Chunk*& chunk = it->second;
		if (chunk)
		{
			chunk->MarkCPUMeshDirty();
		}

		it++;
	}
}


WorldRaycastResult World::RaycastWorld(Vec3 const& startWorldPosition, Vec3 const& forwardNormal, float distance, float raycastBackupOffset /* = 0.0f*/)
{
	WorldRaycastResult result = {};
	BlockIterator raycastBlockIter = {};
	Vec3 rayStartPos = startWorldPosition - (forwardNormal * raycastBackupOffset);
	float raycastDistance = distance + raycastBackupOffset;
	result.m_startPos = startWorldPosition;
	result.m_maxDistance = distance;
	result.m_forwardNormal = forwardNormal;

	raycastBlockIter = GetBlockIteratorForWorldPos(rayStartPos);
	if (raycastBackupOffset == 0.0f)
	{
		if (!raycastBlockIter.IsValid() || raycastBlockIter.GetBlock().IsSolid())
		{
			result.m_didHit = true;
			result.m_hitBlockIterator = raycastBlockIter;
			result.m_impactDistance = 0.0f;
			result.m_impactPoint = startWorldPosition;
			result.m_impactNormal = -forwardNormal;
			result.m_impactFraction = 0.0f;
			return result;
		}
	}

	Vec3 rayDisplacement = forwardNormal * raycastDistance;
	float distanceX = abs(rayDisplacement.x);
	float tOfNextIntersectionX = 9999.0f;
	bool isMovingEast = rayDisplacement.x > 0.0f;
	float tPerUnitX = 0.0f;
	if (distanceX > 0.0f)
	{
		tPerUnitX = 1.0f / distanceX;
		if (isMovingEast)
		{
			BlockIterator eastBlock = raycastBlockIter.GetEastNeighbor();
			if (eastBlock.IsValid())
			{
				Vec3 eastBlockCenter = eastBlock.GetWorldCenter();
				float distanceStartToCenterX = eastBlockCenter.x - rayStartPos.x;
				tOfNextIntersectionX = (distanceStartToCenterX - 0.5f) * tPerUnitX;
			}
		}
		else
		{
			BlockIterator westBlock = raycastBlockIter.GetWestNeighbor();
			if (westBlock.IsValid())
			{
				Vec3 startBlockCenter = raycastBlockIter.GetWorldCenter();
				float distanceStartToCenterX = startBlockCenter.x - rayStartPos.x;
				tOfNextIntersectionX = (0.5f - distanceStartToCenterX) * tPerUnitX;
			}
		}
	}

	float distanceY = abs(rayDisplacement.y);
	float tOfNextIntersectionY = 9999.0f;
	bool isMovingNorth = rayDisplacement.y > 0.0f;
	float tPerUnitY = 0.0f;
	if (distanceY > 0.0f)
	{
		tPerUnitY = 1.0f / distanceY;
		if (isMovingNorth)
		{
			BlockIterator northBlock = raycastBlockIter.GetNorthNeighbor();
			if (northBlock.IsValid())
			{
				Vec3 northBlockCenter = northBlock.GetWorldCenter();
				float distanceStartToCenterY = northBlockCenter.y - rayStartPos.y;
				tOfNextIntersectionY = (distanceStartToCenterY - 0.5f) * tPerUnitY;
			}
		}
		else
		{
			BlockIterator southBlock = raycastBlockIter.GetSouthNeighbor();
			if (southBlock.IsValid())
			{
				Vec3 currentBlockCenter = raycastBlockIter.GetWorldCenter();
				float distanceStartToCenterY = currentBlockCenter.y - rayStartPos.y;
				tOfNextIntersectionY = (0.5f - distanceStartToCenterY) * tPerUnitY;
			}
		}
	}

	float distanceZ = abs(rayDisplacement.z);
	float tOfNextIntersectionZ = 9999.0f;
	bool isMovingUp = rayDisplacement.z > 0.0f;
	float tPerUnitZ = 0.0f;
	if (distanceZ > 0.0f)
	{
		tPerUnitZ = 1.0f / distanceZ;

		if (isMovingUp)
		{
			BlockIterator upBlock = raycastBlockIter.GetUpNeighbor();
			if (upBlock.IsValid())
			{
				Vec3 upBlockCenter = upBlock.GetWorldCenter();
				float distanceStartToCenterZ = upBlockCenter.z - rayStartPos.z;
				tOfNextIntersectionZ = (distanceStartToCenterZ - 0.5f) * tPerUnitZ;
			}
		}
		else
		{
			BlockIterator southBlock = raycastBlockIter.GetSouthNeighbor();
			if (southBlock.IsValid())
			{
				Vec3 currentBlockCenter = raycastBlockIter.GetWorldCenter();
				float distanceStartToCenterZ = currentBlockCenter.z - rayStartPos.z;
				tOfNextIntersectionZ = (0.5f - distanceStartToCenterZ) * tPerUnitZ;
			}
		}
	}

	while (true)
	{
		float smallestTOfNextIntersection = GetMin(tOfNextIntersectionX, tOfNextIntersectionY, tOfNextIntersectionZ);
		if (smallestTOfNextIntersection > 1.0f)
			return result;

		if (smallestTOfNextIntersection == tOfNextIntersectionX)
		{
			if (isMovingEast) 
			{
				raycastBlockIter = raycastBlockIter.GetEastNeighbor();
				if (raycastBlockIter.IsValid() && raycastBlockIter.GetBlock().IsSolid())
				{
					result.m_didHit = true;
					result.m_hitBlockIterator = raycastBlockIter;
					result.m_impactNormal = Vec3(-1.0f, 0.0f, 0.0f);
					result.m_hitSurfaceDirection = DIRECTION_WEST;
					result.m_impactPoint = rayStartPos + rayDisplacement * smallestTOfNextIntersection;
					result.m_impactDistance = (raycastDistance * smallestTOfNextIntersection) - raycastBackupOffset;
					result.m_impactFraction = result.m_impactDistance / distance;
					return result;
				}
			}
			else
			{
				raycastBlockIter = raycastBlockIter.GetWestNeighbor();
				if (raycastBlockIter.IsValid() && raycastBlockIter.GetBlock().IsSolid())
				{
					result.m_didHit = true;
					result.m_hitBlockIterator = raycastBlockIter;
					result.m_impactNormal = Vec3(1.0f, 0.0f, 0.0f);
					result.m_hitSurfaceDirection = DIRECTION_EAST;
					result.m_impactPoint = rayStartPos + rayDisplacement * smallestTOfNextIntersection;
					result.m_impactDistance = (raycastDistance * smallestTOfNextIntersection) - raycastBackupOffset;
					result.m_impactFraction = result.m_impactDistance / distance;
					return result;
				}
			}
			tOfNextIntersectionX += tPerUnitX;
		}
		
		if (smallestTOfNextIntersection == tOfNextIntersectionY)
		{
			if (isMovingNorth)
			{
				raycastBlockIter = raycastBlockIter.GetNorthNeighbor();
				if (raycastBlockIter.IsValid() && raycastBlockIter.GetBlock().IsSolid())
				{
					result.m_didHit = true;
					result.m_hitBlockIterator = raycastBlockIter;
					result.m_impactNormal = Vec3(0.0f, -1.0f, 0.0f);
					result.m_hitSurfaceDirection = DIRECTION_SOUTH;
					result.m_impactPoint = rayStartPos + rayDisplacement * smallestTOfNextIntersection;
					result.m_impactDistance = (raycastDistance * smallestTOfNextIntersection) - raycastBackupOffset;
					result.m_impactFraction = result.m_impactDistance / distance;
					return result;
				}
			}
			else
			{
				raycastBlockIter = raycastBlockIter.GetSouthNeighbor();
				if(raycastBlockIter.IsValid() && raycastBlockIter.GetBlock().IsSolid())
				{
					result.m_didHit = true;
					result.m_hitBlockIterator = raycastBlockIter;
					result.m_impactNormal = Vec3(0.0f, 1.0f, 0.0f);
					result.m_hitSurfaceDirection = DIRECTION_NORTH;
					result.m_impactPoint = rayStartPos + rayDisplacement * smallestTOfNextIntersection;
					result.m_impactDistance = (raycastDistance * smallestTOfNextIntersection) - raycastBackupOffset;
					result.m_impactFraction = result.m_impactDistance / distance;
					return result;
				}
			}
			tOfNextIntersectionY += tPerUnitY;
		}

		if (smallestTOfNextIntersection == tOfNextIntersectionZ)
		{
			if (isMovingUp)
			{
				raycastBlockIter = raycastBlockIter.GetUpNeighbor();
				if (raycastBlockIter.IsValid() && raycastBlockIter.GetBlock().IsSolid())
				{
					result.m_didHit = true;
					result.m_hitBlockIterator = raycastBlockIter;
					result.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
					result.m_hitSurfaceDirection = DIRECTION_DOWN;
					result.m_impactPoint = rayStartPos + rayDisplacement * smallestTOfNextIntersection;
					result.m_impactDistance = (raycastDistance * smallestTOfNextIntersection) - raycastBackupOffset;
					result.m_impactFraction = result.m_impactDistance / distance;
					return result;
				}
			}
			else
			{
				raycastBlockIter = raycastBlockIter.GetDownNeighbor();
				if (raycastBlockIter.IsValid() && raycastBlockIter.GetBlock().IsSolid())
				{
					result.m_didHit = true;
					result.m_hitBlockIterator = raycastBlockIter;
					result.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
					result.m_hitSurfaceDirection = DIRECTION_UP;
					result.m_impactPoint = rayStartPos + rayDisplacement * smallestTOfNextIntersection;
					result.m_impactDistance = (raycastDistance * smallestTOfNextIntersection) - raycastBackupOffset;
					result.m_impactFraction = result.m_impactDistance / distance;
					return result;
				}
			}
			tOfNextIntersectionZ += tPerUnitZ;
		}
	}
}


void World::UnDirtyAllBlocksForChunk(IntVec2 const& chunkCoords)
{
	for (int dirtyBlockIndex = 0; dirtyBlockIndex < m_dirtyBlocksA.size(); dirtyBlockIndex++)
	{
		BlockIterator& dirtyBlockIter = m_dirtyBlocksA[dirtyBlockIndex];
		if (dirtyBlockIter.IsValid() && dirtyBlockIter.m_chunk->GetChunkCoords() == chunkCoords)
		{
			dirtyBlockIter = BlockIterator::INVALID;
		}
	}
}


void World::ProcessDirtyLighting()
{
	while (m_dirtyBlocksA.size() > 0 || m_dirtyBlocksB.size() > 0)
	{
		for (int dirtyBlockIndex = 0; dirtyBlockIndex < m_dirtyBlocksA.size(); dirtyBlockIndex++)
		{
			BlockIterator& blockIterator = m_dirtyBlocksA[dirtyBlockIndex];
			CheckAndUpdateLightingForBlock(blockIterator, m_dirtyBlocksB);
		}
		m_dirtyBlocksA.clear();

		for (int dirtyBlockIndex = 0; dirtyBlockIndex < m_dirtyBlocksB.size(); dirtyBlockIndex++)
		{
			BlockIterator& blockIterator = m_dirtyBlocksB[dirtyBlockIndex];
			CheckAndUpdateLightingForBlock(blockIterator, m_dirtyBlocksA);
		}
		m_dirtyBlocksB.clear();
	}
}


bool World::CheckAndUpdateLightingForBlock(BlockIterator const& blockIterator, std::vector<BlockIterator>& listForDirtyNeighbors)
{
	if (!blockIterator.IsValid())
		return false;

	int blockIndex = blockIterator.m_blockIndex;
	Chunk* chunk = blockIterator.m_chunk;

	BlockIterator northBlockIterator = blockIterator.GetNorthNeighbor();
	BlockIterator southBlockIterator = blockIterator.GetSouthNeighbor();
	BlockIterator eastBlockIterator = blockIterator.GetEastNeighbor();
	BlockIterator westBlockIterator = blockIterator.GetWestNeighbor();
	BlockIterator upwardBlockIterator = blockIterator.GetUpNeighbor();
	BlockIterator downwardBlockIterator = blockIterator.GetDownNeighbor();

	bool isNorthIteratorValid = northBlockIterator.IsValid();
	bool isSouthIteratorValid = southBlockIterator.IsValid();
	bool isEastIteratorValid = eastBlockIterator.IsValid();
	bool isWestIteratorValid = westBlockIterator.IsValid();
	bool isUpIteratorValid = upwardBlockIterator.IsValid();
	bool isDownIteratorValid = downwardBlockIterator.IsValid();

	Block block = blockIterator.GetBlock();
	chunk->SetDirtyLightFlagForBlock(blockIndex, false);

	int correctIndoorLightExposure = block.GetInternalIndoorLight();
	int correctOutdoorLightExposure = block.GetInternalOutdoorLight();

	int currentIndoorLightExposure = block.GetIndoorLightExposure();
	int currentOutdoorLightExposure = block.GetOutdoorLightExposure();

	bool isBlockOpaque = block.IsOpaque();
	if (!isBlockOpaque)
	{
		int brightestNeighboringIndoorLight = 0;
		int brightestNeighboringOutdoorLight = 0;

		if (isNorthIteratorValid)
		{
			Block northBlock = northBlockIterator.GetBlock();
			brightestNeighboringIndoorLight = GetMax(brightestNeighboringIndoorLight, northBlock.GetIndoorLightExposure());
			brightestNeighboringOutdoorLight = GetMax(brightestNeighboringOutdoorLight, northBlock.GetOutdoorLightExposure());
		}

		if (isSouthIteratorValid)
		{
			Block southBlock = southBlockIterator.GetBlock();
			brightestNeighboringIndoorLight = GetMax(brightestNeighboringIndoorLight, southBlock.GetIndoorLightExposure());
			brightestNeighboringOutdoorLight = GetMax(brightestNeighboringOutdoorLight, southBlock.GetOutdoorLightExposure());
		}

		if (isEastIteratorValid)
		{
			Block eastBlock = eastBlockIterator.GetBlock();
			brightestNeighboringIndoorLight = GetMax(brightestNeighboringIndoorLight, eastBlock.GetIndoorLightExposure());
			brightestNeighboringOutdoorLight = GetMax(brightestNeighboringOutdoorLight, eastBlock.GetOutdoorLightExposure());
		}

		if (isWestIteratorValid)
		{
			Block westBlock = westBlockIterator.GetBlock();
			brightestNeighboringIndoorLight = GetMax(brightestNeighboringIndoorLight, westBlock.GetIndoorLightExposure());
			brightestNeighboringOutdoorLight = GetMax(brightestNeighboringOutdoorLight, westBlock.GetOutdoorLightExposure());
		}
		
		if (isUpIteratorValid)
		{
			Block upwardBlock = upwardBlockIterator.GetBlock();
			brightestNeighboringIndoorLight = GetMax(brightestNeighboringIndoorLight, upwardBlock.GetIndoorLightExposure());
			brightestNeighboringOutdoorLight = GetMax(brightestNeighboringOutdoorLight, upwardBlock.GetOutdoorLightExposure());
		}
		
		if (isDownIteratorValid)
		{
			Block downwardBlock = downwardBlockIterator.GetBlock();
			brightestNeighboringIndoorLight = GetMax(brightestNeighboringIndoorLight, downwardBlock.GetIndoorLightExposure());
			brightestNeighboringOutdoorLight = GetMax(brightestNeighboringOutdoorLight, downwardBlock.GetOutdoorLightExposure());
		}

		correctIndoorLightExposure = GetMax(correctIndoorLightExposure, brightestNeighboringIndoorLight - 1);
		correctOutdoorLightExposure = GetMax(correctOutdoorLightExposure, brightestNeighboringOutdoorLight - 1);
	}

	bool isBlockSky = block.IsSky();
	if (isBlockSky)
	{
		correctOutdoorLightExposure = BLOCK_MAX_LIGHT_EXPOSURE;
	}

	if (currentIndoorLightExposure != correctIndoorLightExposure || currentOutdoorLightExposure != correctOutdoorLightExposure)
	{
		chunk->MarkCPUMeshDirty();
		chunk->SetIndoorLightExposureForBlock(blockIndex, correctIndoorLightExposure);
		chunk->SetOutoorLightExposureForBlock(blockIndex, correctOutdoorLightExposure);

		if (isNorthIteratorValid)
		{
			northBlockIterator.MarkChunkMeshDirty();
			if (!northBlockIterator.GetBlock().IsOpaque())
				AddBlockIteratorToDirtyQueue(listForDirtyNeighbors, northBlockIterator);

		}

		if (isSouthIteratorValid)
		{
			southBlockIterator.MarkChunkMeshDirty();
			if (!southBlockIterator.GetBlock().IsOpaque())
				AddBlockIteratorToDirtyQueue(listForDirtyNeighbors, southBlockIterator);

		}
		
		if (isEastIteratorValid)
		{
			eastBlockIterator.MarkChunkMeshDirty();
			if (!eastBlockIterator.GetBlock().IsOpaque())
				AddBlockIteratorToDirtyQueue(listForDirtyNeighbors, eastBlockIterator);

		}
		
		if (isWestIteratorValid)
		{
			westBlockIterator.MarkChunkMeshDirty();
			if (!westBlockIterator.GetBlock().IsOpaque())
				AddBlockIteratorToDirtyQueue(listForDirtyNeighbors, westBlockIterator);

		}
		
		if (isUpIteratorValid)
		{
			upwardBlockIterator.MarkChunkMeshDirty();
			if (!upwardBlockIterator.GetBlock().IsOpaque())
				AddBlockIteratorToDirtyQueue(listForDirtyNeighbors, upwardBlockIterator);

		}
		
		if (isDownIteratorValid)
		{
			downwardBlockIterator.MarkChunkMeshDirty();
			if (!downwardBlockIterator.GetBlock().IsOpaque())
				AddBlockIteratorToDirtyQueue(listForDirtyNeighbors, downwardBlockIterator);

		}

		return true;
	}

	return false;
}


BlockIterator World::GetBlockIteratorForWorldPos(Vec3 const& worldPos)
{
	BlockIterator blockIter = {};
	IntVec2 chunkCoords = GetChunkCoordsForWorldPos(worldPos);
	if (IsChunkActivated(chunkCoords))
	{
		blockIter.m_chunk = m_activatedChunks[chunkCoords];
		blockIter.m_blockIndex = blockIter.m_chunk->GetBlockIndexForWorldPos(worldPos);
	}
	return blockIter;
}


AABB3 World::GetChunkWorldBounds(IntVec2 const& chunkCoords) const
{
	AABB3 chunkBounds;
	chunkBounds.m_mins.x = (float) chunkCoords.x * CHUNK_DIMS_X;
	chunkBounds.m_mins.y = (float) chunkCoords.y * CHUNK_DIMS_Y;
	chunkBounds.m_mins.z = 0.0f;
	chunkBounds.m_maxs.x = chunkBounds.m_mins.x + CHUNK_DIMS_X;
	chunkBounds.m_maxs.y = chunkBounds.m_mins.y + CHUNK_DIMS_Y;
	chunkBounds.m_maxs.z = chunkBounds.m_mins.z + CHUNK_DIMS_Z;
	return chunkBounds;
}


AABB3 World::GetChunkWorldBounds(int x, int y) const
{
	return GetChunkWorldBounds(IntVec2(x, y));
}


IntVec2 World::GetChunkCoordsForWorldPos(Vec3 const& worldPos) const
{
	float chunkX = worldPos.x / (float) CHUNK_DIMS_X;
	float chunkY = worldPos.y / (float) CHUNK_DIMS_Y;
	int x = RoundDownToInt(chunkX);
	int y = RoundDownToInt(chunkY);
	return IntVec2(x, y);
}


bool World::IsChunkActivated(IntVec2 const& chunkCoords) const
{
	auto it = m_activatedChunks.find(chunkCoords);
	return it != m_activatedChunks.end() && it->second != nullptr;
}


void World::AddBlockIteratorToDirtyQueue(BlockIterator const& blockIterator)
{
	int blockIndex = blockIterator.m_blockIndex;
	Chunk* chunk = blockIterator.m_chunk;
	Block block = blockIterator.GetBlock();
	if (!block.IsLightDirty())
	{
		chunk->SetDirtyLightFlagForBlock(blockIndex, true);
		m_dirtyBlocksA.push_back(blockIterator);
	}
}


void World::AddBlockIteratorToDirtyQueue(std::vector<BlockIterator>& dirtyList, BlockIterator const& blockIterator)
{
	int blockIndex = blockIterator.m_blockIndex;
	Chunk* chunk = blockIterator.m_chunk;
	Block block = blockIterator.GetBlock();
	if (!block.IsLightDirty())
	{
		chunk->SetDirtyLightFlagForBlock(blockIndex, true);
		dirtyList.push_back(blockIterator);
	}
}


float World::GetWorldTimeHourMilitaryTime()
{
	int integerPartOfTime = RoundDownToInt(m_worldTime);
	float fractionalPartOfTime = m_worldTime - integerPartOfTime;
	return fractionalPartOfTime * 24.0f;
}


void World::UpdateConstantBuffer()
{
	GameShaderConstants shaderData = {};
	
	shaderData.m_cameraPos = m_player->GetModelMatrix().GetTranslation4D();
	
	m_indoorLightColor.GetColorAsFloats(shaderData.m_indoorLightColor);
	m_outdoorLightColor.GetColorAsFloats(shaderData.m_outdoorLightColor);

	m_skyColor.GetColorAsFloats(shaderData.m_fogColor);
	shaderData.m_fogMinDistance = g_gameConfigBlackboard.GetValue("fogMinDistance", 0.0f);
	shaderData.m_fogMaxDistance = g_gameConfigBlackboard.GetValue("fogMaxDistance", 200.0f);
	shaderData.m_fogMaxIntensity = g_gameConfigBlackboard.GetValue("fogMaxIntensity", 1.0f);
	
	shaderData.m_time = (float) GetCurrentTimeSeconds();
	
	g_cbo->SetData(shaderData);
	g_theRenderer->BindConstantBuffer(8, g_cbo);
}


void World::Update(float deltaSeconds)
{
	float worldTimeScale = g_gameConfigBlackboard.GetValue("worldTimeScale", 0.0f);
	float worldDeltaSeconds = deltaSeconds * worldTimeScale;
	m_worldTime += worldDeltaSeconds / (60.0f * 60.0f * 24.0f);

	CheckInput();

	if (m_maxAllowedChunks > 0)
	{
		if (m_activatedChunks.size() < m_maxAllowedChunks)
		{
			for (int chunkCreationIndex = 0; chunkCreationIndex < NUM_CHUNKS_TO_CREATE_PER_FRAME; chunkCreationIndex++)
			{
				FindSingleChunkToBeCreated();
			}

			for (int i = 0; i < 3; i++)
			{
				ClaimSingleChunkForPerlinGeneration();
			}

			for (int i = 0; i < NUM_CHUNKS_TO_ACTIVATE_PER_FRAME; i++)
			{
				ClaimSingleCreatedChunkForActivation();
			}
		}
	}
	else
	{
		for (int chunkCreationIndex = 0; chunkCreationIndex < NUM_CHUNKS_TO_CREATE_PER_FRAME; chunkCreationIndex++)
		{
			FindSingleChunkToBeCreated();
		}

		for (int i = 0; i < 3; i++)
		{
			ClaimSingleChunkForPerlinGeneration();
		}

		for (int i = 0; i < NUM_CHUNKS_TO_ACTIVATE_PER_FRAME; i++)
		{
			ClaimSingleCreatedChunkForActivation();
		}
	}

	for (int i = 0; i < NUM_CHUNKS_TO_DEACTIVATE_PER_FRAME; i++)
	{
		FindSingleChunkToBeDeactivated();
	}

	for (int i = 0; i < NUM_CHUNKS_TO_SAVE_PER_FRAME; i++)
	{
		FindSingleChunkToBeSaved();
	}

	for (int i = 0; i < NUM_SAVED_CHUNKS_TO_DEACTIVATE_PER_FRAME; i++)
	{
		ClaimSingleSavedChunkForDeactivation();
	}

	if (g_inputSystem->WasKeyJustPressed('M'))
	{
		DirtyAllChunkMeshes();
	}

	auto it = m_activatedChunks.begin();
	while (it != m_activatedChunks.end())
	{
		Chunk*& chunk = it->second;
		if (chunk->IsMeshDirty())
		{
			bool success = chunk->GenerateCPUMesh();
			UNUSED(success);
		}

		it++;
	}

	ProcessDirtyLighting();

	it = m_activatedChunks.begin();
	while (it != m_activatedChunks.end())
	{
		Chunk*& chunk = it->second;
		if (chunk)
		{
			chunk->Update(deltaSeconds);
		}

		it++;
	}

	if (m_player)
	{
		m_player->Update(deltaSeconds);
	}

	UpdateWorldColor();
	UpdateConstantBuffer();

	if (!m_isRaycastLocked && m_player != nullptr)
	{
		m_raycastStartPos = m_player->GetEyePos();
		m_raycastForwardNormal = m_player->GetForward();
	}
	m_raycastResult = RaycastWorld(m_raycastStartPos, m_raycastForwardNormal, m_raycastDistance);

	//print out debug info
	{
		std::string debugInfo = Stringf("Activated Chunk Count: %i | Max allowed chunks: %i", m_numActiveChunk, m_maxAllowedChunks);
		DebugAddScreenText(debugInfo, Vec2(0.0f, 0.0f), 0.0f, Vec2(0.0f, 0.0f), 16.0f);

		int day = RoundDownToInt(m_worldTime);
		float hour = GetWorldTimeHourMilitaryTime();
		std::string worldTimeText;
		if (hour >= 12.0f)
		{
			hour = hour >= 13.0f ? hour - 12.0f : hour;
			worldTimeText = Stringf("World day: %i | world hour: %.2f pm", day, hour);
		}
		else if (hour < 12.0f)
		{
			worldTimeText = Stringf("World day: %i | world hour: %.2f am", day, hour);
		}
		float uiX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
		DebugAddScreenText(worldTimeText, Vec2(uiX, 0.0f), 0.0f, Vec2(1.0f, 0.0f), 16.0f, Rgba8::WHITE);
	}
}


void World::UpdateWorldColor()
{
	float worldTimeHour = GetWorldTimeHourMilitaryTime();

	static Rgba8 skyNoonColor = g_gameConfigBlackboard.GetValue("skyNoonColor", Rgba8::MAGENTA);
	static Rgba8 skyNightColor = g_gameConfigBlackboard.GetValue("skyNightColor", Rgba8::YELLOW);
	if (worldTimeHour >= 6.0f && worldTimeHour < 12.0f)
	{
		float t = (worldTimeHour - 6.0f) / 6.0f;
		m_skyColor = Lerp(skyNightColor, skyNoonColor, t);
	}
	else if (worldTimeHour >= 12.0f && worldTimeHour < 18.0f)
	{
		float t = (worldTimeHour - 12.0f) / 6.0f;
		m_skyColor = Lerp(skyNoonColor, skyNightColor, t);
	}
	else
	{
		m_skyColor = skyNightColor;
	}

	float uiX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
	//DebugAddScreenText(Stringf("sky color: %i, %i, %i", m_skyColor.r, m_skyColor.g, m_skyColor.b), Vec2(uiX, 0.0f), 0.0f, Vec2(1.0f, 0.0f), 16.0f);

	static Rgba8 outdoorNoonLightColor = g_gameConfigBlackboard.GetValue("outdoorNoonLightColor", Rgba8::MAGENTA);
	static Rgba8 outdoorNightLightColor = g_gameConfigBlackboard.GetValue("outdoorNightLightColor", Rgba8::MAGENTA);
	if (worldTimeHour < 12.0f)
	{
		float t = worldTimeHour / 12.0f;
		m_outdoorLightColor = Lerp(outdoorNightLightColor, outdoorNoonLightColor, t);
	}
	else
	{
		float t = (worldTimeHour - 12.0f) / 12.0f;
		m_outdoorLightColor = Lerp(outdoorNoonLightColor, outdoorNightLightColor, t);
	}
	if (!g_gameConfigBlackboard.GetValue("debugVariableOutdoorLight", false))
		m_outdoorLightColor = outdoorNoonLightColor;


	float lightningPerlin = Compute1dPerlinNoise(m_worldTime, 0.03f, 9);
	float lightningStrength = RangeMapClamped(lightningPerlin, 0.6f, 0.9f, 0.0f, 1.0f);
	m_outdoorLightColor = Lerp(m_outdoorLightColor, Rgba8::WHITE, lightningStrength);
	m_skyColor = Lerp(m_skyColor, Rgba8::WHITE, lightningStrength);

	static Rgba8 indoorLightColor = g_gameConfigBlackboard.GetValue("indoorLightColor", Rgba8::MAGENTA);
	float glowPerlin = Compute1dPerlinNoise(m_worldTime, 0.003f, 9);
	float glowStrength = RangeMapClamped(glowPerlin, -1.0f, 1.0f, 0.8f, 1.0f);
	unsigned char glowStrengthByte = FloatToByte(glowStrength);
	m_indoorLightColor = indoorLightColor * Rgba8(glowStrengthByte, glowStrengthByte, glowStrengthByte, 255);
}


void World::ClaimSingleSavedChunkForDeactivation()
{
	Chunk* chunkToDeactivate = nullptr;

	g_fileIOMutex.lock();
	auto chunkIter = g_chunksQueuedForFileIO.begin();
	while (chunkIter != g_chunksQueuedForFileIO.end())
	{
		if ((*chunkIter)->GetChunkStatus() == ChunkStatus::CHUNK_STATUS_SAVED)
		{
			chunkToDeactivate = *chunkIter;
			g_chunksQueuedForFileIO.erase(chunkIter);
			break;
		}
		chunkIter++;
	}
	g_fileIOMutex.unlock();

	if (chunkToDeactivate != nullptr)
	{
		DeactivateChunk(chunkToDeactivate);
	}
}


void World::ClaimSingleChunkForPerlinGeneration()
{
	Chunk* chunkForPerlinGeneration = nullptr;

	g_fileIOMutex.lock();
	auto chunkIter = g_chunksQueuedForFileIO.begin();
	while (chunkIter != g_chunksQueuedForFileIO.end())
	{
		if ((*chunkIter)->GetChunkStatus() == ChunkStatus::CHUNK_STATUS_GENERATING_PERLIN_QUEUED)
		{
			chunkForPerlinGeneration = *chunkIter;
			g_chunksQueuedForFileIO.erase(chunkIter);
			break;
		}
		chunkIter++;
	}
	g_fileIOMutex.unlock();

	if (chunkForPerlinGeneration != nullptr)
	{
		g_perlinMutex.lock();
		g_chunksQueuedForPerlinGeneration.push_back(chunkForPerlinGeneration);
		g_perlinMutex.unlock();
	}
}


bool World::DoesChunkExistInPerlinQueue(IntVec2 const& chunkCoords)
{
	g_perlinMutex.lock();
	for (int chunkIndex = 0; chunkIndex < g_chunksQueuedForPerlinGeneration.size(); chunkIndex++)
	{
		Chunk*& chunk = g_chunksQueuedForPerlinGeneration[chunkIndex];
		if (chunk->GetChunkCoords() == chunkCoords)
		{
			g_perlinMutex.unlock();
			return true;
		}
	}
	g_perlinMutex.unlock();
	return false;
}


bool World::DoesChunkExistInFileIOQueue(IntVec2 const& chunkCoords)
{
	g_fileIOMutex.lock();
	for (int chunkIndex = 0; chunkIndex < g_chunksQueuedForFileIO.size(); chunkIndex++)
	{
		Chunk*& chunk = g_chunksQueuedForFileIO[chunkIndex];
		if (chunk->GetChunkCoords() == chunkCoords)
		{
			g_fileIOMutex.unlock();
			return true;
		}
	}
	g_fileIOMutex.unlock();
	return false;
}


void World::Render() const
{
	g_theRenderer->ClearScreen(m_skyColor);
	Texture* defaultDepthStencil = g_theRenderer->GetDefaultDepthStencil();
	g_theRenderer->ClearDepth(defaultDepthStencil, 1.0f, 0);

	g_theRenderer->SetCullMode(CullMode::BACK);
	g_theRenderer->SetFillMode(FillMode::SOLID);

	Shader* worldShader = g_theRenderer->CreateOrGetShader("Data/Shaders/World");
	g_theRenderer->BindShader(worldShader);
	Texture const& blocksTexture = BlockDef::GetTexture();
	g_theRenderer->BindTexture(0, &blocksTexture);
	auto it = m_activatedChunks.begin();
	while (it != m_activatedChunks.end())
	{
		Chunk* const& chunk = it->second;
		if (chunk)
		{
			chunk->Render();
		}

		it++;
	}

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(0, nullptr);
	if (m_debugRenderChunks)
	{
		it = m_activatedChunks.begin();
		while (it != m_activatedChunks.end())
		{
			Chunk* const& chunk = it->second;
			if (chunk)
			{
				chunk->DebugRender();
			}

			it++;
		}
	}

	if (m_player)
	{
		m_player->Render();
		
		GameCamera* playerCam = m_player->GetCamera();
		if (playerCam == nullptr || playerCam->GetCameraMode() != GAME_CAMERA_MODE_FIRST_PERSON)
		{
			VisualiseRaycast(m_raycastResult);
		}
	}

	if (m_raycastResult.m_hitBlockIterator.IsValid())
	{
		Vec3 bottomLeft, bottomRight, topLeft, topRight;
		m_raycastResult.m_hitBlockIterator.GetBlockFaceForDirection(m_raycastResult.m_hitSurfaceDirection, topLeft, topRight, bottomLeft, bottomRight);
		RenderMaterial renderMaterial = {};
		renderMaterial.m_color = Rgba8::MAGENTA;
		std::vector<Vertex_PCU> faceOutlineVerts;
		AddVertsForWireQuad3DToVector(faceOutlineVerts, bottomLeft, bottomRight, topLeft, topRight, 0.05f, Rgba8::MAGENTA);
		g_theRenderer->DrawVertexArray((int) faceOutlineVerts.size(), faceOutlineVerts.data());
	}
}


BlockIterator BlockIterator::INVALID = BlockIterator(nullptr, -1);


BlockIterator::BlockIterator(Chunk* chunk, int blockIndex)
	: m_chunk(chunk)
	, m_blockIndex(blockIndex)
{

}


BlockIterator::BlockIterator()
{

}


BlockIterator BlockIterator::GetEastNeighbor() const
{
	if (m_chunk == nullptr)
		return INVALID;

	Chunk* neighboringChunk = nullptr;
	int neighborBlockIndex = m_blockIndex + 1;
	neighboringChunk = m_chunk;
	if (neighborBlockIndex % CHUNK_DIMS_X == 0)
	{
		neighboringChunk = m_chunk->GetEastChunk();
		if (neighboringChunk == nullptr)
			return INVALID;
		
		neighborBlockIndex -= CHUNK_DIMS_X;
	}

	BlockIterator eastNeighbor = BlockIterator(neighboringChunk, neighborBlockIndex);
	return eastNeighbor;
}


BlockIterator BlockIterator::GetNorthNeighbor() const
{
	if (m_chunk == nullptr)
		return INVALID;

	Chunk* neighboringChunk = nullptr;
	int neighborBlockIndex = m_blockIndex + CHUNK_DIMS_X;
	neighboringChunk = m_chunk;
	if ((neighborBlockIndex & CHUNK_BITMASK_Z) != (m_blockIndex & CHUNK_BITMASK_Z))
	{
		neighboringChunk = m_chunk->GetNorthChunk();
		if (neighboringChunk == nullptr)
			return INVALID;

		neighborBlockIndex -= CHUNK_BLOCK_COUNT_SINGLE_LAYER;
	}

	BlockIterator northNeighbor = BlockIterator(neighboringChunk, neighborBlockIndex);
	return northNeighbor;
}


BlockIterator BlockIterator::GetSouthNeighbor() const
{
	if (m_chunk == nullptr)
		return INVALID;

	Chunk* neighboringChunk = nullptr;
	int neighborBlockIndex = m_blockIndex - CHUNK_DIMS_X;
	neighboringChunk = m_chunk;
	if (neighborBlockIndex < 0 || (neighborBlockIndex & CHUNK_BITMASK_Z) != (m_blockIndex & CHUNK_BITMASK_Z))
	{
		neighboringChunk = m_chunk->GetSouthChunk();
		if (neighboringChunk == nullptr)
			return INVALID;

		neighborBlockIndex += CHUNK_BLOCK_COUNT_SINGLE_LAYER;
	}

	BlockIterator southNeighbor = BlockIterator(neighboringChunk, neighborBlockIndex);
	return southNeighbor;
}


BlockIterator BlockIterator::GetWestNeighbor() const
{
	if (m_chunk == nullptr)
		return INVALID;

	Chunk* neighboringChunk = nullptr;
	int neighborBlockIndex = m_blockIndex - 1;
	neighboringChunk = m_chunk;
	if ((neighborBlockIndex + 1) % CHUNK_DIMS_X == 0 || neighborBlockIndex < 0)
	{
		neighboringChunk = m_chunk->GetWestChunk();
		if (neighboringChunk == nullptr)
			return INVALID;
		
		neighborBlockIndex += CHUNK_DIMS_X;
	}

	BlockIterator westNeighbor = BlockIterator(neighboringChunk, neighborBlockIndex);
	return westNeighbor;
}


BlockIterator BlockIterator::GetUpNeighbor() const
{
	if (m_chunk == nullptr)
		return INVALID;

	Chunk* neighboringChunk = nullptr;
	int neighborBlockIndex = m_blockIndex + CHUNK_BLOCK_COUNT_SINGLE_LAYER;
	neighboringChunk = m_chunk;
	if (neighborBlockIndex >= CHUNK_BLOCK_COUNT_TOTAL)
		return INVALID;

	BlockIterator upNeighbor = BlockIterator(neighboringChunk, neighborBlockIndex);
	return upNeighbor;
}


BlockIterator BlockIterator::GetDownNeighbor() const
{
	if (m_chunk == nullptr)
		return INVALID;

	Chunk* neighboringChunk = nullptr;
	int neighborBlockIndex = m_blockIndex - CHUNK_BLOCK_COUNT_SINGLE_LAYER;
	neighboringChunk = m_chunk;
	if (neighborBlockIndex < 0)
		return INVALID;

	BlockIterator downNeighbor = BlockIterator(neighboringChunk, neighborBlockIndex);
	return downNeighbor;
}


void BlockIterator::MarkChunkMeshDirty()
{
	if (m_chunk)
	{
		m_chunk->MarkCPUMeshDirty();
	}
}


void BlockIterator::GetBlockFaceForDirection(Directions direction, Vec3& out_topLeft, Vec3& out_topRight, Vec3& out_bottomLeft, Vec3& out_bottomRight) const
{
	AABB3 blockAABB3 = m_chunk->GetBlockWorldBounds(m_blockIndex);

	float minX = blockAABB3.m_mins.x;
	float minY = blockAABB3.m_mins.y;
	float minZ = blockAABB3.m_mins.z;
	float maxX = blockAABB3.m_maxs.x;
	float maxY = blockAABB3.m_maxs.y;
	float maxZ = blockAABB3.m_maxs.z;

	switch (direction)
	{
		case DIRECTION_NORTH:
		{
			out_bottomLeft = Vec3(maxX, maxY, minZ);
			out_bottomRight = Vec3(minX, maxY, minZ);
			out_topLeft = Vec3(maxX, maxY, maxZ);
			out_topRight = Vec3(minX, maxY, maxZ);
			break;
		}
		case DIRECTION_SOUTH:
		{
			out_bottomLeft = Vec3(minX, minY, minZ);
			out_bottomRight = Vec3(maxX, minY, minZ);
			out_topLeft = Vec3(minX, minY, maxZ);
			out_topRight = Vec3(maxX, minY, maxZ);
			break;
		}
		case DIRECTION_EAST:
		{
			out_bottomLeft = Vec3(maxX, minY, minZ);
			out_bottomRight = Vec3(maxX, maxY, minZ);
			out_topLeft = Vec3(maxX, minY, maxZ);
			out_topRight = Vec3(maxX, maxY, maxZ);
			break;
		}
		case DIRECTION_WEST:
		{
			out_bottomLeft = Vec3(minX, maxY, minZ);
			out_bottomRight = Vec3(minX, minY, minZ);
			out_topLeft = Vec3(minX, maxY, maxZ);
			out_topRight = Vec3(minX, minY, maxZ);
			break;
		}
		case DIRECTION_UP:
		{
			out_bottomLeft = Vec3(minX, minY, maxZ);
			out_bottomRight = Vec3(maxX, minY, maxZ);
			out_topLeft = Vec3(minX, maxY, maxZ);
			out_topRight = Vec3(maxX, maxY, maxZ);
			break;
		}
		case DIRECTION_DOWN:
		{
			out_bottomLeft = Vec3(minX, maxY, minZ);
			out_bottomRight = Vec3(maxX, maxY, minZ);
			out_topLeft = Vec3(minX, minY, minZ);
			out_topRight = Vec3(maxX, minY, minZ);
			break;
		}
		default:
			break;
	}
}


Vec3 BlockIterator::GetWorldCenter() const
{
	if (m_chunk)
	{
		return m_chunk->GetWorldCenterForBlock(m_blockIndex);
	}

	return Vec3(0.0f, 0.0f, 0.0f);
}


Block BlockIterator::GetBlock() const
{
	return m_chunk->GetBlockAtIndex(m_blockIndex);
}


bool BlockIterator::IsValid() const
{
	return m_chunk != nullptr && m_blockIndex >= 0 && m_blockIndex < CHUNK_BLOCK_COUNT_TOTAL;
}

