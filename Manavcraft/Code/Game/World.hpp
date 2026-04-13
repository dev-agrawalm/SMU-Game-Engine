#pragma once
#include "Game/Chunk.hpp"
#include <map>
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/GameCommon.hpp"
#include <mutex>

class Player;
struct Block;
class Entity;

struct BlockIterator
{
public:
	static BlockIterator INVALID;
public:
	BlockIterator();
	BlockIterator(Chunk* chunk, int blockIndex);
	
	BlockIterator GetEastNeighbor()		const;
	BlockIterator GetNorthNeighbor()	const;
	BlockIterator GetSouthNeighbor()	const;
	BlockIterator GetWestNeighbor()		const;
	BlockIterator GetUpNeighbor()		const;
	BlockIterator GetDownNeighbor()		const;

	void MarkChunkMeshDirty();
	
	void GetBlockFaceForDirection(Directions direction, Vec3& out_topLeft, Vec3& out_topRight, Vec3& out_bottomLeft, Vec3& out_bottomRight) const;
	Vec3 GetWorldCenter() const;
	Block GetBlock() const;
	bool IsValid() const;

public:
	Chunk* m_chunk = nullptr;
	int m_blockIndex = -1;
};


struct WorldRaycastResult : public BaseRaycastResult3D
{
public:
	BlockIterator m_hitBlockIterator = BlockIterator::INVALID;
	Directions m_hitSurfaceDirection = DIRECTION_EAST;
};


class World
{
public:
	World(Player* player);
	void Create();
	void Destroy();
	void Update(float deltaSeconds);

	void Render() const;
	void CheckInput();

	void FindSingleChunkToBeCreated();
	void FindSingleChunkToBeDeactivated();
	void FindSingleChunkToBeSaved();
	void ClaimSingleCreatedChunkForActivation();
	void ClaimSingleChunkForPerlinGeneration();
	void ClaimSingleSavedChunkForDeactivation();

	void ActivateChunk(Chunk* chunkToActivate);
	void DeactivateChunk(Chunk* chunkToDeactivate);
	void DirtyAllChunkMeshes();
	
	WorldRaycastResult RaycastWorld(Vec3 const& startWorldPosition, Vec3 const& forwardNormal, float distance, float raycastBackupOffset = 0.0f);

	void UnDirtyAllBlocksForChunk(IntVec2 const& chunkCoords);
	void ProcessDirtyLighting();
	bool CheckAndUpdateLightingForBlock(BlockIterator const& blockIterator, std::vector<BlockIterator>& listForDirtyNeighbors);
	
	BlockIterator GetBlockIteratorForWorldPos(Vec3 const& worldPos);
	AABB3 GetChunkWorldBounds(IntVec2 const& chunkCoords) const;
	AABB3 GetChunkWorldBounds(int x, int y) const;
	IntVec2 GetChunkCoordsForWorldPos(Vec3 const& worldPos) const;
	bool IsChunkActivated(IntVec2 const& chunkCoords) const;
	bool DoesChunkExistInPerlinQueue(IntVec2 const& chunkCoords);
	bool DoesChunkExistInFileIOQueue(IntVec2 const& chunkCoords);

	void AddBlockIteratorToDirtyQueue(BlockIterator const& blockIterator);
	void AddBlockIteratorToDirtyQueue(std::vector<BlockIterator>& dirtyList, BlockIterator const& blockIterator);

	float GetWorldTimeHourMilitaryTime();

private:
	void UpdateConstantBuffer();
	void UpdateWorldColor();

private:
	Player* m_player = nullptr;

	float m_chunkActivationDistance = 0.0f;
	float m_chunkDeactivationDistance = 0.0f;
	int m_maxAllowedChunks = 0;
	int m_numActiveChunk = 0;
	std::map<IntVec2, Chunk*> m_activatedChunks;

	std::vector<std::string> m_existingChunkSaveFileNames;

	bool m_debugRenderChunks = false;

	bool m_isRaycastLocked = false;
	float m_raycastDistance = 8.0;
	Vec3 m_raycastStartPos;
	Vec3 m_raycastForwardNormal;
	WorldRaycastResult m_raycastResult;

	//need both lists to implement queue
	std::vector<BlockIterator> m_dirtyBlocksA;
	std::vector<BlockIterator> m_dirtyBlocksB;

	float m_worldTime = 0.3f;
	Rgba8 m_skyColor;
	Rgba8 m_outdoorLightColor;
	Rgba8 m_indoorLightColor;
};
