#pragma once
#include "Game/Block.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include <atomic>

struct IntVec3;
class World;

enum class ChunkStatus
{
	CHUNK_STATUS_DEACTIVATED,
	
	CHUNK_STATUS_CREATING,
	CHUNK_STATUS_GENERATING_PERLIN_QUEUED,
	CHUNK_STATUS_GENERATING_PERLIN,
	CHUNK_STATUS_LOAD_FROM_FILE_QUEUED,
	CHUNK_STATUS_LOADING_FROM_FILE,
	CHUNK_STATUS_CREATED,

	CHUNK_STATUS_SAVING_QUEUED,
	CHUNK_STATUS_SAVING,
	CHUNK_STATUS_SAVED,

	CHUNK_STATUS_GPU_MESH_DIRTY,
	CHUNK_STATUS_ACTIVE,
};

enum Directions : int;

class Chunk
{
public:
	Chunk();
	bool LoadFromFile();
	bool SaveToFile();
	void GeneratePerlin();
	void Deactivate(bool shouldSave = false);

	void Create(World* world, IntVec2 const& chunkCoords);
	void InitialiseLightExposures();
	void GenerateChunk();
	bool GenerateCPUMesh();
	void Render() const;
	void DebugRender() const;
	void Update(float deltaSeconds);

	void		SetChunkStatus(ChunkStatus newStatus);
	void		SetBlockDefinitionForBlockCoords(IntVec3 const& blockCoords, uint8_t newBlockDefId);
	void		SetBlockDefinitionForBlockIndex(int blockIndex, uint8_t newBlockDefId);

	Chunk*		GetNorthChunk();
	Chunk*		GetSouthChunk();
	Chunk*		GetEastChunk();
	Chunk*		GetWestChunk();

	void		SetNorthChunk(Chunk* northChunk);
	void		SetSouthChunk(Chunk* southChunk);
	void		SetEastChunk(Chunk* eastChunk);
	void		SetWestChunk(Chunk* westChunk);

	void		MarkCPUMeshDirty();
	bool		SetSkyFlagForBlock(int blockIndex, bool isSky);
	bool		SetDirtyLightFlagForBlock(int blockIndex, bool isLightDirty);
	bool		SetIndoorLightExposureForBlock(int blockIndex, int indoorLightExposure);
	bool		SetOutoorLightExposureForBlock(int blockIndex, int outoorLightExposure);

	Block		GetBlockAtIndex(int index);
	Block		GetBlockAtCoords(int x, int y, int z);
	int			GetBlockIndexFromCoords(int x, int y, int z) const;
	int			GetBlockIndexFromCoords(IntVec3 const& blockCoords) const;
	bool		IsBlockOpaque(IntVec3 const& blockCoords) const;
	bool		IsMeshDirty() const;
	bool		NeedsSaving() const;
	Vec3		GetWorldCenter() const;
	Vec3		GetWorldCenterForBlock(int blockIndex) const;
	AABB3		GetBlockWorldBounds(int x, int y, int z) const;
	AABB3		GetBlockWorldBounds(int blockIndex);
	IntVec2		GetChunkCoords() const;
	IntVec3		GetBlockCoordsFromIndex(int arrayIndex) const;
	IntVec3		GetLocalBlockCoordsForWorldPos(Vec3 const& worldPos) const;
	int			GetBlockIndexForWorldPos(Vec3 const& worldPos) const;
	uint8_t		GetBlockDefIdForBlockCoords(IntVec3 const& blockCoords) const; 
	ChunkStatus GetChunkStatus() const;
	std::string GetChunkName() const;
private:
	void	AddVertsForBlockQuadInDirection(Directions direction, IntVec3 const& blockCoords, std::vector<Vertex_PCU>& out_vector,
									  Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topLeft, Vec3 const& topRight, AABB2 const& UVs);
private:
	AABB3 m_worldBounds = AABB3::ZERO_TO_ONE;
	IntVec2 m_chunkCoords;
	World* m_world = nullptr;
	
	bool m_needsSaving = false;
	bool m_isCPUMeshDirty = false;

	std::string m_chunkName;
	std::atomic<ChunkStatus> m_chunkStatus = ChunkStatus::CHUNK_STATUS_DEACTIVATED;
	Block* m_blocks = nullptr;

	std::vector<Vertex_PCU> m_verts;
	VertexBuffer* m_vbo = nullptr;

	Chunk* m_westChunk = nullptr;
	Chunk* m_eastChunk = nullptr;
	Chunk* m_southChunk = nullptr;
	Chunk* m_northChunk = nullptr;
};
