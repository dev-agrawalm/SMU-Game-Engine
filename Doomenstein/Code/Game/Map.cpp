#include "Game/Map.hpp"
#include "Game/World.hpp"
#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/Prop.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/SpawnInfo.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Controller.hpp"

int Map::s_actorSalt = -1;

Map::Map(World* world)
{
	m_world = world;
}


Map::~Map()
{

}


void Map::Create(MapDefinition const* definition)
{
	m_definition = definition;
	GenerateGrid();

	SpawnActorsFromXml(m_definition->m_spawnInfoXmlPath);

	SoundID playMusic = g_audioSystem->CreateOrGetSound(m_definition->m_bgMusicPath);
	float playmodeMusicVolume = g_gameConfigBlackboard.GetValue("playmodeMusicVolume", 0.0f);
	m_bgMusicID = g_audioSystem->StartSound(playMusic, true, playmodeMusicVolume);
}


void Map::Destroy()
{
	for (int actorIndex = 0; actorIndex < (int) m_allActors.size(); actorIndex++)
	{
		Actor*& actor = m_allActors[actorIndex];
		if (actor)
		{
			Controller* aiController = actor->GetAIController();
			actor->GetActorUID().Invalidate();
			delete actor;
			actor = nullptr;
			if (aiController)
			{
				delete aiController;
				aiController = nullptr;
			}
		}
	}

	m_definition = nullptr;

	g_audioSystem->StopSound(m_bgMusicID);
}


void Map::Update(float deltaSeconds)
{
	Vec3 mapOrigin = m_definition->m_origin;
	Mat44 identity = Mat44::CreateTranslation3D(mapOrigin);
	DebugAddWorldBasis(identity);

	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor* const& actor = m_allActors[actorIndex];
		if (actor)
		{
			actor->Update(deltaSeconds);
		}
	}

	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor* const& actor = m_allActors[actorIndex];
		if (actor)
		{
			actor->LateUpdate(deltaSeconds);
		}
	}

	CheckCollisionsBetweenAllActors();
	CheckAndResolveCollisionsForFlyingActorsAlongZ();
	CheckCollisionsBetweenAllActorsAndGrid();

	CollectGarbage();
}


void Map::Render() const
{
	RenderGrid();

	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor* const& actor = m_allActors[actorIndex];
		if (actor && !actor->IsHidden())
		{
			actor->Render();
		}
	}
}


RaycastResult Map::RayCastAll(Vec3 const& rayStart, Vec3 const& rayDirection, float maxDistance, ActorUID raycastingActorUID, bool debugRaycasts /*= false*/)
{
	RaycastResult results[3] = {};
	results[0] = RaycastMapXY(rayStart, rayDirection, maxDistance);
	results[1] = RaycastMapZ(rayStart, rayDirection, maxDistance);
	results[2] = RaycastEntities(rayStart, rayDirection, maxDistance, raycastingActorUID);


	bool shouldDebug = g_game->ShouldDebugRaycasts() && debugRaycasts;
	if (shouldDebug)
	{
		Rgba8 rayLineColor = Rgba8::WHITE;
		Rgba8 rayImpactSphereColor = Rgba8(255, 255, 255, 150);

		RaycastResult wallResult = results[0];
		Vec3 wallRaycastStart = wallResult.m_start;
		Vec3 wallRaycastEnd = wallRaycastStart + wallResult.m_totalLength * wallResult.m_forwardNormal;
		DebugAddWorldLine(wallRaycastStart, wallRaycastEnd, rayLineColor, 0.1f, 10.0f, DebugRenderMode::XRAY);
		if (wallResult.m_didImpact)
		{
			Vec3 impactPoint = wallResult.m_impactPoint;
			DebugAddWorldSolidSphere(impactPoint, 0.125f, 10.0f, rayImpactSphereColor, DebugRenderMode::USE_DEPTH);
			
			Vec3 impactNormal = wallResult.m_impactNormal;
			DebugAddWorldArrow(impactPoint, impactPoint + impactNormal, 10.0f, Rgba8::BLUE, 0.1f, DebugRenderMode::XRAY);
		}

		RaycastResult ceilingResult = results[1];
		Vec3 ceilingRaycastStart = ceilingResult.m_start;
		Vec3 ceilingRaycastEnd = ceilingRaycastStart + ceilingResult.m_totalLength * ceilingResult.m_forwardNormal;
		DebugAddWorldLine(ceilingRaycastStart, ceilingRaycastEnd, rayLineColor, 0.1f, 10.0f, DebugRenderMode::XRAY);
		if (ceilingResult.m_didImpact)
		{
			Vec3 impactPoint = ceilingResult.m_impactPoint;
			DebugAddWorldSolidSphere(impactPoint, 0.125f, 10.0f, rayImpactSphereColor, DebugRenderMode::USE_DEPTH);

			Vec3 impactNormal = ceilingResult.m_impactNormal;
			DebugAddWorldArrow(impactPoint, impactPoint + impactNormal, 10.0f, Rgba8::GREEN, 0.1f, DebugRenderMode::XRAY);
		}

		RaycastResult entityResult = results[2];
		Vec3 entityRaycastStart = entityResult.m_start;
		Vec3 entityRaycastEnd = entityRaycastStart + entityResult.m_totalLength * entityResult.m_forwardNormal;
		DebugAddWorldLine(entityRaycastStart, entityRaycastEnd, rayLineColor, 0.1f, 10.0f, DebugRenderMode::XRAY);
		if (entityResult.m_didImpact)
		{
			Vec3 impactPoint = entityResult.m_impactPoint;
			DebugAddWorldSolidSphere(impactPoint, 0.125f, 10.0f, rayImpactSphereColor, DebugRenderMode::USE_DEPTH);

			Vec3 impactNormal = entityResult.m_impactNormal;
			DebugAddWorldArrow(impactPoint, impactPoint + impactNormal, 10.0f, Rgba8::RED, 0.1f, DebugRenderMode::XRAY);
		}
	}
	
	RaycastResult finalResult = results[0];
	for (int resultIndex = 1; resultIndex < 3; resultIndex++)
	{
		RaycastResult& result = results[resultIndex];
		if (result.m_didImpact)
		{
			if (!finalResult.m_didImpact)
			{
				finalResult = result;
				continue;
			}

			if (result.m_impactDistance < finalResult.m_impactDistance)
			{
				finalResult = result;
			}
		}
	}
	return finalResult;
}


RaycastResult Map::RaycastMapXY(Vec3 const& rayStart, Vec3 const& rayFwdNormal, float maxDistance)
{
	RaycastResult result;
	result.m_forwardNormal = rayFwdNormal;
	result.m_start = rayStart;
	result.m_totalLength = maxDistance;
	
	IntVec2 startTileCoords = GetGridCoords(rayStart);
	if (!AreTileCoordsValid(startTileCoords))
		return result;
	Tile startTile = GetTile(startTileCoords);
	if (IsTileSolid(startTile))
	{
		result.m_didImpact = true;
		result.m_impactDistance = 0.0f;
		result.m_impactPoint = rayStart;
		result.m_impactFraction = 0.0f;
		result.m_impactNormal = -rayFwdNormal;
		return result;
	}
	
	Vec3 forwardNormalXY = Vec3(rayFwdNormal.x, rayFwdNormal.y, 0.0f).GetNormalized();
	float maxDistanceXY = DotProduct3D(rayFwdNormal * maxDistance, forwardNormalXY);
	Vec3 rayDisplacementXY = forwardNormalXY * maxDistanceXY;

	float tileWidth = m_definition->m_tileDims.x;
	float tileHeight = m_definition->m_tileDims.y;

	float tPerUnitLengthX = 1.0f / abs(rayDisplacementXY.x);
	float tPerTileWidth = tPerUnitLengthX * tileWidth;
	int tileStepX = rayDisplacementXY.x >= 0.0f ? 1 : -1;
	int tileOffsetToNextTileX = (tileStepX + 1) / 2;
	float firstVerticalIntersectionX = static_cast<float>(startTileCoords.x + tileOffsetToNextTileX) * tileWidth;
	float tOfNextXCrossing = abs(firstVerticalIntersectionX - rayStart.x) * tPerUnitLengthX;

	float tPerUnitLengthY = 1.0f / abs(rayDisplacementXY.y);
	float tPerTileHeight = tPerUnitLengthY * tileHeight;
	int tileStepY = rayDisplacementXY.y >= 0.0f ? 1 : -1;
	int tileOffsetToNextTileY = (tileStepY + 1) / 2;
	float firstVerticalIntersectionY = static_cast<float>(startTileCoords.y + tileOffsetToNextTileY) * tileHeight;
	float tOfNextYCrossing = abs(firstVerticalIntersectionY - rayStart.y) * tPerUnitLengthY;

	IntVec2 tileCoords = startTileCoords;
	while (true)
	{
		if (tOfNextXCrossing <= tOfNextYCrossing)
		{
			if (tOfNextXCrossing > 1)
			{
				result.m_didImpact = false;
				return result;
			}

			tileCoords.x += tileStepX;
			Tile tile = GetTile(tileCoords);
			Vec3 tilePos = GetTileCenter(tileCoords);

			if (IsTileSolid(tile))
			{
				result.m_didImpact		= true;
				result.m_impactFraction = tOfNextXCrossing;
				float impactDistanceXY	= tOfNextXCrossing * maxDistanceXY;
				result.m_impactDistance = (maxDistance * impactDistanceXY) / maxDistanceXY;
				result.m_impactPoint	= rayStart + rayFwdNormal * result.m_impactDistance;
				result.m_impactNormal	= Vec3((float) -tileStepX, 0.0f, 0.0f);
				return result;
			}

			tOfNextXCrossing += tPerTileWidth;
		}
		else if (tOfNextYCrossing < tOfNextXCrossing)
		{
			if (tOfNextYCrossing > 1)
			{
				result.m_didImpact = false;
				return result;
			}

			tileCoords.y += tileStepY;
			Tile tile = GetTile(tileCoords);
			Vec3 tilePos = GetTileCenter(tileCoords);

			if (IsTileSolid(tile))
			{
				result.m_didImpact		= true;
				result.m_impactFraction = tOfNextYCrossing;
				float impactDistanceXY	= tOfNextYCrossing * maxDistanceXY;
				result.m_impactDistance = (maxDistance * impactDistanceXY) / maxDistanceXY;
				result.m_impactPoint	= rayStart + rayFwdNormal * result.m_impactDistance;
				result.m_impactNormal	= Vec3(0.0f, (float) -tileStepY, 0.0f);
				return result;
			}

			tOfNextYCrossing += tPerTileHeight;
		}
	}
}


RaycastResult Map::RaycastMapZ(Vec3 const& rayStart, Vec3 const& rayFwdNormal, float maxDistance)
{
	RaycastResult result;
	result.m_forwardNormal = rayFwdNormal;
	result.m_start = rayStart;
	result.m_totalLength = maxDistance;
	result.m_didImpact = false;

	float tileHeight = m_definition->m_tileDims.z;
	//going up
	if (rayFwdNormal.z > 0.0)
	{
		float heightOfRaycastFromStart = (rayFwdNormal * maxDistance).z;
		float heightOfCeilingFromStart = tileHeight - rayStart.z;

		float impactFraction = heightOfCeilingFromStart / heightOfRaycastFromStart;
		if (impactFraction >= 0.0f && impactFraction <= 1.0f)
		{
			result.m_didImpact = true;
			result.m_impactFraction = impactFraction;
			result.m_impactPoint = rayStart + rayFwdNormal * maxDistance * impactFraction;
			result.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
			result.m_impactDistance = maxDistance * impactFraction;
		}
	}

	//going down
	if (rayFwdNormal.z < 0.0f)
	{
		float heightOfRaycastFromStart = -(rayFwdNormal * maxDistance).z;
		float heightOfStartFromFloor = rayStart.z;

		float impactFraction = heightOfStartFromFloor / heightOfRaycastFromStart;
		if (impactFraction >= 0.0f && impactFraction <= 1.0f)
		{
			result.m_didImpact = true;
			result.m_impactFraction = impactFraction;
			result.m_impactPoint = rayStart + rayFwdNormal * maxDistance * impactFraction;
			result.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
			result.m_impactDistance = maxDistance * impactFraction;
		}
	}

	return result;
}


RaycastResult Map::RaycastEntities(Vec3 const& rayStart, Vec3 const& rayFwdNormal, float maxDistance, ActorUID raycastingActorUID)
{
	RaycastResult defaultResult = {};
	defaultResult.m_start = rayStart;
	defaultResult.m_forwardNormal = rayFwdNormal;
	defaultResult.m_totalLength = maxDistance;
	defaultResult.m_didImpact = false;

	if (m_allActors.size() <= 0)
	{
		return defaultResult;
	}

	std::vector<RaycastResult> actorHitResults;
	for (int actorIndex = 0; actorIndex < (int) m_allActors.size(); actorIndex++)
	{
		RaycastResult result;
		result.m_start = rayStart;
		result.m_forwardNormal = rayFwdNormal;
		result.m_totalLength = maxDistance;

		Actor*& actor = m_allActors[actorIndex];
		if (actor)
		{
			Vec3 actorPos = actor->GetPosition();
			float actorPhysicsRadius = actor->GetCollisionRadius();
			float actorPhysicsHeight = actor->GetCollisionHeight();

			BaseRaycastResult3D raycastResult3D = RaycastVsZCylinder3D(rayStart, rayFwdNormal, maxDistance, actorPos.GetVec2(), FloatRange(0.0f, actorPhysicsHeight), actorPhysicsRadius);
			if (!raycastResult3D.m_didHit || actor->GetActorUID() == raycastingActorUID)
			{
				continue;
			}

			result.m_didImpact = true;
			result.m_impactDistance = raycastResult3D.m_impactDistance;
			result.m_impactPoint = raycastResult3D.m_impactPoint;
			result.m_impactFraction = raycastResult3D.m_impactDistance / maxDistance;
			result.m_impactNormal = raycastResult3D.m_impactNormal;
			result.m_impactActorUID = actor->GetActorUID();
			actorHitResults.push_back(result);
		}
	}

	if ((int) actorHitResults.size() > 0)
	{
		RaycastResult finalResult = actorHitResults[0];
		for (int hitIndex = 1; hitIndex < (int) actorHitResults.size(); hitIndex++)
		{
			RaycastResult& result = actorHitResults[hitIndex];
			if (result.m_impactDistance < finalResult.m_impactDistance)
			{
				finalResult = result;
			}
		}

		return finalResult;
	}
	else
	{
		return defaultResult;
	}
}


ActorUID Map::GenerateActorUIDForIndex(int actorIndex) const
{
	int salt = ActorUID::GetNextSalt(s_actorSalt);
	ActorUID uid = ActorUID(actorIndex, salt);
	s_actorSalt = salt;
	return uid;
}


ActorUID Map::GetActorUIDForNewActor() const
{
	for (int actorIndex = 0; actorIndex < (int) m_allActors.size(); actorIndex++)
	{
		Actor* const& actor = m_allActors[actorIndex];
		if (actor == nullptr)
			return GenerateActorUIDForIndex(actorIndex);
	}

	return GenerateActorUIDForIndex((int) m_allActors.size());
}


ActorList Map::GetActorListByFaction(ActorFaction faction) const
{
	return m_actorsByFaction[faction];
}



Tile Map::GetRandomNonSolidTile() const
{
	int randomTileIndex = g_rng->GetRandomIntLessThan((int) m_grid.size());
	Tile randomTile = m_grid[randomTileIndex];
	while (randomTile.IsSolid())
	{
		randomTileIndex = g_rng->GetRandomIntLessThan((int) m_grid.size());
		randomTile = m_grid[randomTileIndex];
	}

	return randomTile;
}


Actor* Map::GetActorWithUID(ActorUID actorUID) const
{
	if (!actorUID.IsValid())
	{
		return nullptr;
	}

	int index = actorUID.GetIndex();
	if (index >= (int) m_allActors.size())
	{
		return nullptr;
	}

	return m_allActors[index];
}


bool Map::CheckAndResolveCollisionBetweenActors(Actor*& actorA, Actor*& actorB)
{
	if (actorA->m_owningActorUID.IsValid() || actorB->m_owningActorUID.IsValid())
	{
		if (actorB->GetActorUID() == actorA->m_owningActorUID)
			return false;
		if (actorA->GetActorUID() == actorB->m_owningActorUID)
			return false;
	}

	if (actorA->GetFaction() == ACTOR_FACTION_PROJECTILE && actorB->GetFaction() == ACTOR_FACTION_PROJECTILE)
		return false;

	if (!DoActorsCollide(actorA, actorB))
		return false;
	
	ApplyCollisionImpulse(actorA, actorB);
	ApplyCollisionDamageOnActors(actorA, actorB);
	
	if (actorA->DieOnCollision())
		actorA->Die();

	if (actorB->DieOnCollision())
		actorB->Die();
	
	if (!actorA->IsDead() && !actorB->IsDead())
		PushActorsOutOfEachOther(actorA, actorB);
	return true;
}


void Map::ApplyCollisionImpulse(Actor*& actorA, Actor*& actorB)
{
	Vec2 impactNormalXY = actorA->GetPosition().GetVec2() - actorB->GetPosition().GetVec2();
	impactNormalXY.Normalize();
	if (actorA->CreatesImpulseOnTouch())
	{
		float impulse = actorA->GetTouchImpulse();
		Vec3 impactNormal = -impactNormalXY.GetVec3();
		actorB->ApplyImpulse(impactNormal * impulse);
	}

	if (actorB->CreatesImpulseOnTouch())
	{
		float impulse = actorB->GetTouchImpulse();
		Vec3 impactNormal = impactNormalXY.GetVec3();
		actorA->ApplyImpulse(impactNormal * impulse);
	}
}


void Map::ApplyCollisionDamageOnActors(Actor*& actorA, Actor*& actorB)
{
	bool aDealsDamageOnCollision = actorA->DoesDamageOnTouch();
	if (actorA->m_definition == actorB->m_definition)
		return;

	if (aDealsDamageOnCollision)
	{
		FloatRange damageRange = actorA->GetCollisionDamage();
		float damage = g_rng->GetRandomFloatInRange(damageRange.m_min, damageRange.m_max);
		actorB->TakeDamage((int) damage);
	}

	bool bDealsDamageOnCollision = actorB->DoesDamageOnTouch();
	if (bDealsDamageOnCollision)
	{
		FloatRange damageRange = actorB->GetCollisionDamage();
		float damage = g_rng->GetRandomFloatInRange(damageRange.m_min, damageRange.m_max);
		actorA->TakeDamage((int) damage);
	}
}


void Map::PushActorsOutOfEachOther(Actor*& actorA, Actor*& actorB)
{
	bool isAMovable = !actorA->IsImmovable();
	bool isBMovable = !actorB->IsImmovable();

	bool aPushesB = isBMovable;
	bool bPushesA = isAMovable;

	Vec3 posA = actorA->GetPosition();
	float radiusA = actorA->GetCollisionRadius();
	Vec3 posB = actorB->GetPosition();
	float radiusB = actorB->GetCollisionRadius();

	if (aPushesB && bPushesA)
	{
		Vec2 newPosAXY = posA.GetVec2();
		Vec2 newPosBXY = posB.GetVec2();
		if (PushDisksOutOfEachOther2D(newPosAXY, radiusA, newPosBXY, radiusB))
		{
			actorA->SetPosition(newPosAXY.GetVec3());
			actorB->SetPosition(newPosBXY.GetVec3());
		}
	}
	else if (aPushesB && !bPushesA)
	{
		Vec2 newPosAXY = posA.GetVec2();
		Vec2 newPosBXY = posB.GetVec2();
		if (PushDiskOutOfDisk2D(newPosBXY, radiusB, newPosAXY, radiusA))
		{
			actorA->SetPosition(newPosAXY.GetVec3());
			actorB->SetPosition(newPosBXY.GetVec3());
		}
	}
	else if (!aPushesB && bPushesA)
	{
		Vec2 newPosAXY = posA.GetVec2();
		Vec2 newPosBXY = posB.GetVec2();
		if (PushDiskOutOfDisk2D(newPosAXY, radiusA, newPosBXY, radiusB))
		{
			actorA->SetPosition(newPosAXY.GetVec3());
			actorB->SetPosition(newPosBXY.GetVec3());
		}
	}
}


bool Map::DoActorsCollide(Actor* actorA, Actor* actorB)
{
	Vec2 posAXY = actorA->GetPosition().GetVec2();
	float radiusA = actorA->GetCollisionRadius();
	float baseA		= actorA->GetCollisionBase();
	float heightA	= actorA->GetCollisionHeight();

	Vec2 posBXY = actorB->GetPosition().GetVec2();
	float radiusB = actorB->GetCollisionRadius();
	float baseB		= actorB->GetCollisionBase();
	float heightB	= actorB->GetCollisionHeight();

	return DoZCylindersOverlap3D(posAXY, FloatRange(baseA, heightA), radiusA, posBXY, FloatRange(baseB, heightB), radiusB);
}


bool Map::CheckAndResolveCollisionBetweenActorAndTile(Actor*& actor, Tile const& tile)
{
	if (!tile.IsSolid())
		return false;

	Vec2 tileBoxMins2D = tile.m_worldBounds.m_mins.GetVec2();
	Vec2 tileBoxMax2D = tile.m_worldBounds.m_maxs.GetVec2();
	AABB2 tileBox2D = AABB2(tileBoxMins2D, tileBoxMax2D);

	Vec2 actorPosXY = actor->GetPosition().GetVec2();
	float actorRadius = actor->GetCollisionRadius();

	if (PushDiskOutOfAABB2D(actorPosXY, actorRadius, tileBox2D))
	{
		if (actor->DieOnCollision())
		{
			actor->Die();
			return true;
		}
		actor->SetPosition(actorPosXY.GetVec3());
		return true;
	}

	return false;
}


void Map::CheckAndResolveCollisionsForFlyingActorsAlongZ()
{
	for (int actorIndex = 0; actorIndex < m_flyingActors.size(); actorIndex++)
	{
		Actor*& flyingActor = m_flyingActors[actorIndex];
		if (flyingActor && !flyingActor->IsDead() && flyingActor->HasCollisionWithMap())
			CheckAndResolveCollisionForActorAlongZ(flyingActor);
	}
}


bool Map::CheckAndResolveCollisionForActorAlongZ(Actor*& actor)
{
	float actorHeight = actor->GetCollisionHeight();
	float actorBase = actor->GetCollisionBase();
	float mapHeight = m_definition->m_tileDims.z;
	bool dieOnCollision = actor->DieOnCollision();
	if (actorHeight >= mapHeight)
	{
		if (dieOnCollision)
		{
			actor->Die();
			return true;
		}

		Vec2 actorPosXY = actor->GetPosition().GetVec2();
		float newZ = mapHeight - actor->GetLocalCollisionHeight();
		actor->SetPosition(Vec3(actorPosXY.x, actorPosXY.y, newZ));
		return true;
	}

	if (actorBase <= 0.0f)
	{
		if (dieOnCollision)
		{
			actor->Die();
			return true;
		}

		Vec2 actorPosXY = actor->GetPosition().GetVec2();
		float newZ = 0.0f - actor->GetLocalCollisionBase();
		actor->SetPosition(Vec3(actorPosXY.x, actorPosXY.y, newZ));
		return true;
	}

	return false;
}


Actor* Map::SpawnActor(SpawnInfo const& spawnInfo)
{
	Actor* actor = new Actor(this, spawnInfo);
	return AddActorToMap(actor);
}


Actor* Map::AddActorToMap(Actor* actor)
{
	ActorUID actorUID = GetActorUIDForNewActor();
	actor->SetActorUID(actorUID);
	AddActorToList(actor, m_allActors);
	
	ActorFaction actorFaction = actor->GetFaction();
	if (actorFaction > ACTOR_FACTION_UNKNOWN && actorFaction < NUM_ACTOR_FACTION)
	{
		ActorList& actorFactionList = m_actorsByFaction[actorFaction];
		AddActorToList(actor, actorFactionList);
	}

	bool doesActorFly = actor->CanFly();
	if (doesActorFly)
	{
		AddActorToList(actor, m_flyingActors);
	}

	return actor;
}


Actor* Map::RemoveActorFromMap(Actor* actor)
{
	int actorIndex = actor->GetActorUID().GetIndex();
	if (actorIndex < (int) m_allActors.size())
	{
		m_allActors[actorIndex] = nullptr;
		return actor;
	}

	return nullptr;
}


Actor* Map::GetFirstUnpossesedActor() const
{
	for (int actorIndex = 0; actorIndex < (int) m_allActors.size(); actorIndex++)
	{
		Actor* const& actor = m_allActors[actorIndex];
		if (actor && actor->GetActorUID().IsValid())
		{
			if (actor->IsPossessable() && !actor->IsPossessed())
			{
				return actor;
			}
		}
	}

	return nullptr;
}


World* Map::GetWorld() const
{
	return m_world;
}


Tile Map::GetTile(Vec3 const& worldPoint) const
{
	IntVec2 tileCoords = GetGridCoords(worldPoint);
	int tileIndex = GetGridArrayIndex(tileCoords);
	return m_grid[tileIndex];
}


Tile Map::GetTile(IntVec2 const& tileCoords) const
{
	int tileIndex = GetGridArrayIndex(tileCoords);
	return m_grid[tileIndex];
}


Vec3 Map::GetTileCenter(IntVec2 const& tileCoords) const
{
	float tileWidth = m_definition->m_tileDims.x;
	float tileHeight = m_definition->m_tileDims.y;

	float x = tileCoords.x * tileWidth + tileWidth * 0.5f;
	float y = tileCoords.y * tileHeight + tileHeight * 0.5f;

	return Vec3(x, y, 0.0f);
}


IntVec2 Map::GetGridCoords(Vec3 const& worldPoint) const
{
	float tileWidth = m_definition->m_tileDims.x;
	float tileHeight = m_definition->m_tileDims.y;

	int x = RoundDownToInt(worldPoint.x / tileWidth);
	int y = RoundDownToInt(worldPoint.y / tileHeight);
	return IntVec2(x, y);
}


bool Map::IsTileSolid(Tile const& tile) const
{
	return tile.m_tileDef->m_isSolid;
}


bool Map::AreTileCoordsValid(IntVec2 const& tileCoords) const
{
	int tileArrayIndex = GetGridArrayIndex(tileCoords);
	return tileArrayIndex >= 0 && tileArrayIndex < (int) m_grid.size();
}


int Map::GetGridArrayIndex(IntVec2 const& gridCoords) const
{
	int x = gridCoords.x;
	int y = gridCoords.y;

	return x + y * m_gridDimensions.x;
}


int Map::GetGridArrayIndex(int x, int y) const
{
	return x + y * m_gridDimensions.x;
}


MapDefinition const* Map::GetMapDefinition() const
{
	return m_definition;
}


Actor* Map::AddActorToList(Actor* actor, ActorList& list)
{
	for (int actorIndex = 0; actorIndex < (int) list.size(); actorIndex++)
	{
		Actor*& a = list[actorIndex];
		if (a == nullptr)
		{
			list[actorIndex] = actor;
			return actor;
		}
	}

	list.push_back(actor);
	return actor;
}


void Map::GenerateGrid()
{
	Vec3 origin = m_definition->m_origin;
	Image const& sourceImage = m_definition->m_sourceImage;
	m_gridDimensions = m_definition->m_sourceImage.GetDimensions();
	int gridX = m_gridDimensions.x;
	int gridY = m_gridDimensions.y;

	std::vector<Rgba8> imageTextData;
	Vec3 tileDims = m_definition->m_tileDims;
	TileSet const* tileSet = m_definition->m_tileSet;

	int tileCount = gridX * gridY;
	m_grid.reserve(tileCount);

	for (int y = 0; y < gridY; y++)
	{
		for (int x = 0; x < gridX; x++)
		{
			Tile tile;
			tile.m_gridCoords = IntVec2(x, y);
			Rgba8 tileColor = sourceImage.GetTexelRgba8(x, y);
			TileDefinition const* tileDef = tileSet->GetTileDefinition(tileColor);
			ASSERT_OR_DIE(tileDef != nullptr, "Invalid tile definition in tile set (name): " + tileSet->m_name);

			tile.m_tileDef = tileDef;
			tile.m_hasFloor = tileDef->m_floorMaterial != nullptr;
			Vec3 boundMins = origin + Vec3(x * tileDims.x, y * tileDims.y, origin.z);
			Vec3 boundMaxs = boundMins + tileDims;
			tile.m_worldBounds.m_mins = boundMins;
			tile.m_worldBounds.m_maxs = boundMaxs;

			AddVertsForTileToMapVerts(tile);
			m_grid.push_back(tile);
		}
	}
}


void Map::RenderGrid() const
{
	auto materialIterator = m_gridVerts.begin();
	while (materialIterator != m_gridVerts.end())
	{
		TileMaterial const* material = materialIterator->first;
		MaterialVerts verts = materialIterator->second;
		
		if (material == TileMaterial::GetTileMaterialByName("WoodTile"))
		{
			g_theRenderer->SetCullMode(CullMode::FRONT);
		}
		else
		{
			g_theRenderer->SetCullMode(CullMode::NONE);
		}

		if (material->m_isVisible)
		{
			Shader const* shader;
			Texture const* diffuse;
			shader = material->m_shader;
			diffuse = material->m_diffuse;
			g_theRenderer->BindShader(shader);
			g_theRenderer->BindTexture(0, diffuse);
			g_theRenderer->DrawVertexArray((int) verts.size(), verts.data());
		}
		materialIterator++;
	}
}


void Map::AddVertsForTileToMapVerts(Tile const& tile)
{
	TileDefinition const* tileDef = tile.m_tileDef;
	AABB2 uvs;
	Rgba8 tint;
	Shader const* shader;
	Texture const* diffuse;

	TileMaterial const* ceilingMaterial = tileDef->m_ceilingMaterial;
	TileMaterial const* floorMaterial = tileDef->m_floorMaterial;
	TileMaterial const* wallMaterial = tileDef->m_wallMaterial;

	Vec3 tileMins = tile.m_worldBounds.m_mins;
	Vec3 tileMaxs = tile.m_worldBounds.m_maxs;

	if (ceilingMaterial && ceilingMaterial->m_isVisible)
	{
		std::vector<Vertex_PCU> ceilingVerts;
		
		uvs		= ceilingMaterial->m_uvs;
		tint	= ceilingMaterial->m_tint;
		shader	= ceilingMaterial->m_shader;
		diffuse = ceilingMaterial->m_diffuse;

		Vec3 bottomLeft		= Vec3(tileMins.x, tileMins.y, tileMaxs.z);
		Vec3 bottomRight	= Vec3(tileMaxs.x, tileMins.y, tileMaxs.z);
		Vec3 topLeft		= Vec3(tileMins.x, tileMaxs.y, tileMaxs.z);
		Vec3 topRight		= tileMaxs;

		AddVertsForQuad3DToVector(ceilingVerts, bottomLeft, bottomRight, topLeft, topRight, tint, uvs);

		auto materialIterator = m_gridVerts.find(ceilingMaterial);
		if (materialIterator != m_gridVerts.end())
		{
			MaterialVerts& verts = materialIterator->second;
			verts.insert(verts.end(), ceilingVerts.begin(), ceilingVerts.end());
		}
		else
		{
			m_gridVerts[ceilingMaterial] = ceilingVerts;
		}
	}

	if (floorMaterial && floorMaterial->m_isVisible)
	{
		std::vector<Vertex_PCU> floorVerts;

		uvs		= floorMaterial->m_uvs;
		tint	= floorMaterial->m_tint;
		shader	= floorMaterial->m_shader;
		diffuse = floorMaterial->m_diffuse;

		Vec3 bottomRight	= Vec3(tileMaxs.x, tileMaxs.y, tileMins.z);
		Vec3 bottomLeft		= Vec3(tileMins.x, tileMaxs.y, tileMins.z);
		Vec3 topRight		= Vec3(tileMaxs.x, tileMins.y, tileMins.z);
		Vec3 topLeft		= tileMins;

		AddVertsForQuad3DToVector(floorVerts, bottomLeft, bottomRight, topLeft, topRight, tint, uvs);

		auto materialIterator = m_gridVerts.find(floorMaterial);
		if (materialIterator != m_gridVerts.end())
		{
			MaterialVerts& verts = materialIterator->second;
			verts.insert(verts.end(), floorVerts.begin(), floorVerts.end());
		}
		else
		{
			m_gridVerts[floorMaterial] = floorVerts;
		}
	}

	if (wallMaterial && wallMaterial->m_isVisible)
	{
		std::vector<Vertex_PCU> wallVerts;

		uvs		= wallMaterial->m_uvs;
		tint	= wallMaterial->m_tint;
		shader	= wallMaterial->m_shader;
		diffuse = wallMaterial->m_diffuse;

		//south face
		Vec3 bottomLeft		= tileMins;
		Vec3 bottomRight	= Vec3(tileMaxs.x, tileMins.y, tileMins.z);
		Vec3 topLeft		= Vec3(tileMins.x, tileMins.y, tileMaxs.z);
		Vec3 topRight		= Vec3(tileMaxs.x, tileMins.y, tileMaxs.z);

		AddVertsForQuad3DToVector(wallVerts, bottomLeft, bottomRight, topLeft, topRight, tint, uvs);

		//north face
		bottomLeft		= Vec3(tileMaxs.x, tileMaxs.y, tileMins.z);
		bottomRight		= Vec3(tileMins.x, tileMaxs.y, tileMins.z);
		topLeft			= tileMaxs;
		topRight		= Vec3(tileMins.x, tileMaxs.y, tileMaxs.z);

		AddVertsForQuad3DToVector(wallVerts, bottomLeft, bottomRight, topLeft, topRight, tint, uvs);

		//east face
		bottomLeft		= Vec3(tileMaxs.x, tileMins.y, tileMins.z);
		bottomRight		= Vec3(tileMaxs.x, tileMaxs.y, tileMins.z);
		topLeft			= Vec3(tileMaxs.x, tileMins.y, tileMaxs.z);
		topRight		= tileMaxs;

		AddVertsForQuad3DToVector(wallVerts, bottomLeft, bottomRight, topLeft, topRight, tint, uvs);

		//west face
		bottomLeft		= Vec3(tileMins.x, tileMaxs.y, tileMins.z);
		bottomRight		= tileMins;
		topLeft			= Vec3(tileMins.x, tileMaxs.y, tileMaxs.z);
		topRight		= Vec3(tileMins.x, tileMins.y, tileMaxs.z);

		AddVertsForQuad3DToVector(wallVerts, bottomLeft, bottomRight, topLeft, topRight, tint, uvs);


		auto materialIterator = m_gridVerts.find(wallMaterial);
		if (materialIterator != m_gridVerts.end())
		{
			MaterialVerts& verts = materialIterator->second;
			verts.insert(verts.end(), wallVerts.begin(), wallVerts.end());
		}
		else
		{
			m_gridVerts[wallMaterial] = wallVerts;
		}
	}
}


void Map::CollectGarbage()
{
	for (int actorIndex = 0; actorIndex < (int) m_allActors.size(); actorIndex++)
	{
		Actor*& actor = m_allActors[actorIndex];
		if (actor && actor->IsGarbage())
		{
			Actor* a = RemoveActorFromMap(actor);
			if (a)
			{
				Controller* aiController = a->GetAIController();
				a->GetActorUID().Invalidate();
				delete a;
				a = nullptr;
				if (aiController)
				{
					delete aiController;
					aiController = nullptr;
				}
			}
		}
	}
}


void Map::CheckCollisionsBetweenAllActors()
{
	for (int actorAIndex = 0; actorAIndex < m_allActors.size(); actorAIndex++)
	{
		Actor*& actorA = m_allActors[actorAIndex];
		if (actorA && !actorA->IsDead())
		{
			for (int actorBIndex = 0; actorBIndex < m_allActors.size(); actorBIndex++)
			{
				if (actorAIndex != actorBIndex)
				{
					Actor*& actorB = m_allActors[actorBIndex];
					if (actorB && !actorB->IsDead())
					{
						bool doesACollide = actorA->HasCollisionWithActors();
						bool doesBCollide = actorB->HasCollisionWithActors();

						if (doesACollide && doesBCollide)
						{
							CheckAndResolveCollisionBetweenActors(actorA, actorB);
						}
					}
				}
			}
		}
	}
}


void Map::CheckCollisionsBetweenAllActorsAndGrid()
{
	for (int actorIndex = 0; actorIndex < m_allActors.size(); actorIndex++)
	{
		Actor*& actor = m_allActors[actorIndex];
		if (actor && !actor->IsDead() && actor->HasCollisionWithMap())
		{
			IntVec2 actorTileCoords = GetGridCoords(actor->GetPosition());

			IntVec2 northTileCoords = actorTileCoords + NORTH;
			if (AreTileCoordsValid(northTileCoords))
				CheckAndResolveCollisionBetweenActorAndTile(actor, GetTile(northTileCoords));

			IntVec2 southTileCoords = actorTileCoords + SOUTH;
			if (AreTileCoordsValid(southTileCoords))
				CheckAndResolveCollisionBetweenActorAndTile(actor, GetTile(southTileCoords));
			
			IntVec2 eastTileCoords = actorTileCoords + EAST;
			if (AreTileCoordsValid(eastTileCoords))
				CheckAndResolveCollisionBetweenActorAndTile(actor, GetTile(eastTileCoords));

			IntVec2 westTileCoords = actorTileCoords + WEST;
			if (AreTileCoordsValid(westTileCoords))
				CheckAndResolveCollisionBetweenActorAndTile(actor, GetTile(westTileCoords));

			IntVec2 northEastTileCoords = actorTileCoords + NORTH_EAST;
			if (AreTileCoordsValid(northEastTileCoords))
				CheckAndResolveCollisionBetweenActorAndTile(actor, GetTile(northEastTileCoords));

			IntVec2 southEastTileCoords = actorTileCoords + SOUTH_EAST;
			if (AreTileCoordsValid(southEastTileCoords))
				CheckAndResolveCollisionBetweenActorAndTile(actor, GetTile(southEastTileCoords));

			IntVec2 northWestTileCoords = actorTileCoords + NORTH_WEST;
			if (AreTileCoordsValid(northWestTileCoords))
				CheckAndResolveCollisionBetweenActorAndTile(actor, GetTile(northWestTileCoords));

			IntVec2 southWestTileCoords = actorTileCoords + SOUTH_WEST;
			if (AreTileCoordsValid(southWestTileCoords))
				CheckAndResolveCollisionBetweenActorAndTile(actor, GetTile(southWestTileCoords));
		}
	}
}


void Map::SpawnActorsFromXml(std::string const& xmlPath)
{
	XmlDocument xmlDoc;
	XmlError result = xmlDoc.LoadFile(xmlPath.c_str());
	GUARANTEE_OR_DIE(result == XmlError::XML_SUCCESS, "Unable to open spawn info path for map: " + xmlPath);

	XmlElement* rootElement = xmlDoc.RootElement();
	GUARANTEE_OR_DIE(rootElement && _stricmp(rootElement->Name(), "SpawnInfos") == 0, "Unable to find definitions element in spawn info xml");

	XmlElement const* spawnInfoElement = rootElement->FirstChildElement("SpawnInfo");
	while (spawnInfoElement)
	{
		SpawnInfo info = {};
		info = SpawnInfo(*spawnInfoElement);
		SpawnActor(info);

		spawnInfoElement = spawnInfoElement->NextSiblingElement("SpawnInfo");
	}
}

