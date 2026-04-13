#pragma once
#include <vector>
#include <map>
#include "Game/Entity.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/Actor.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

struct MapDefinition;
struct Tile;
struct TileMaterial;
struct SpawnInfo;

class Controller;
class World;

typedef std::vector<Actor*> ActorList;

struct RaycastResult
{
	Vec3 m_start;
	Vec3 m_forwardNormal;
	float m_totalLength;
	bool m_didImpact;
	Vec3 m_impactPoint;
	Vec3 m_impactNormal;
	float m_impactDistance = 0.0f;
	float m_impactFraction = 0.0f;
	ActorUID m_impactActorUID = ActorUID::INVALID;
};


class Map
{
	typedef std::vector<Vertex_PCU> MaterialVerts;

public:
	Map(World* world);
	~Map();
	void Create(MapDefinition const* definition);
	void Destroy();
	void Update(float deltaSeconds);
	void Render() const;

	RaycastResult RayCastAll(Vec3 const& rayStart, Vec3 const& rayDirection, float maxDistance, ActorUID raycastingActorUID, bool debugRaycasts = false);
	RaycastResult RaycastMapXY(Vec3 const& rayStart, Vec3 const& rayFwdNormal, float maxDistance);
	RaycastResult RaycastMapZ(Vec3 const& rayStart, Vec3 const&rayFwdNormal, float maxDistance);
	RaycastResult RaycastEntities(Vec3 const& rayStart, Vec3 const& rayFwdNormal, float maxDistance, ActorUID raycastingActorUID);

	Actor*	SpawnActor(SpawnInfo const& spawnInfo);
	Actor*	AddActorToMap(Actor* actor);
	Actor*	RemoveActorFromMap(Actor* actor);
	Actor*	GetFirstUnpossesedActor() const;
	Actor*	GetActorWithUID(ActorUID actorUID) const;
	ActorUID GenerateActorUIDForIndex(int actorIndex) const;
	ActorUID GetActorUIDForNewActor() const;
	ActorList GetActorListByFaction(ActorFaction faction) const;
	Tile GetRandomNonSolidTile() const;

	bool CheckAndResolveCollisionBetweenActors(Actor*& actorA, Actor*& actorB);
	void ApplyCollisionImpulse(Actor*& actorA, Actor*& actorB);
	void ApplyCollisionDamageOnActors(Actor*& actorA, Actor*& actorB);
	void PushActorsOutOfEachOther(Actor*& actorA, Actor*& actorB);
	bool DoActorsCollide(Actor* actorA, Actor* actorB);
	bool CheckAndResolveCollisionBetweenActorAndTile(Actor*& actor, Tile const& tile);
	void CheckAndResolveCollisionsForFlyingActorsAlongZ();
	bool CheckAndResolveCollisionForActorAlongZ(Actor*& actor);

	World*	GetWorld() const;
	Tile	GetTile(IntVec2 const& tileCoords) const;
	Tile	GetTile(Vec3 const& worldPoint) const;
	Vec3	GetTileCenter(IntVec2 const& tileCoords) const;
	IntVec2 GetGridCoords(Vec3 const& worldPoint) const;
	int		GetGridArrayIndex(IntVec2 const& gridCoords) const;
	int		GetGridArrayIndex(int x, int y) const;
	bool	IsTileSolid(Tile const& tile) const;
	bool	AreTileCoordsValid(IntVec2 const& tileCoords) const;
	MapDefinition const* GetMapDefinition() const;
private:
	Actor* AddActorToList(Actor* actor, ActorList& list);
	void GenerateGrid();
	void RenderGrid() const;
	void AddVertsForTileToMapVerts(Tile const& tile);
	void CollectGarbage();
	void CheckCollisionsBetweenAllActors();
	void CheckCollisionsBetweenAllActorsAndGrid();
	void SpawnActorsFromXml(std::string const& xmlPath);

private:
	World* m_world;

	MapDefinition const* m_definition = nullptr;
	std::vector<Tile> m_grid;
	std::map<TileMaterial const*, MaterialVerts> m_gridVerts;
	IntVec2 m_gridDimensions;

	ActorList m_allActors;
	ActorList m_actorsByFaction[ActorFaction::NUM_ACTOR_FACTION];
	ActorList m_flyingActors;

	SoundID m_bgMusicID = 0;
private:
	static int s_actorSalt;

private: //dev console commands
// 	static bool DeleteActor(EventArgs& args);
// 	static bool SpawnActor(EventArgs& args);
// 	static bool DisplayActorIndex(EventArgs& args);
};
