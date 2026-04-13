#include "Game/GameMode.hpp"
#include "Game/World.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Game/Map.hpp"
#include "Game/SpawnInfo.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerController.hpp"

GameMode::GameMode(World* world)
	: m_world(world)
{
}


void GameMode::Init()
{
	m_modeDuration		= g_gameConfigBlackboard.GetValue("gameModeDuration", 10.0f);
	m_playerRespawnTime = g_gameConfigBlackboard.GetValue("playerRespawnTime", 1.0f);
	m_maxDemonCount		= g_gameConfigBlackboard.GetValue("maxDemonCount", 100);
	m_demonSpawnTime	= g_gameConfigBlackboard.GetValue("demonSpawnTime", 5.0f);

	m_modeTimer.Start(m_modeDuration);
	m_modeTimer.Pause();
	m_demonSpawnTimer.Start(m_demonSpawnTime);
	m_demonSpawnTimer.Pause();
}


void GameMode::Update()
{
	Map* map = m_world->GetCurrentMap();
	ActorList demons = map->GetActorListByFaction(ACTOR_FACTION_DEMONS);
	if ((int) demons.size() > m_maxDemonCount)
	{
		m_world->OnGameLost();
		return;
	}

	if (m_modeTimer.HasElapsed() && demons.size() <= m_maxDemonCount)
	{
		m_world->OnGameWon();
		return;
	}

	if (m_demonSpawnTimer.CheckAndDecrement())
	{
		ActorList demonSpawnPoints = map->GetActorListByFaction(ACTOR_FACTION_DEMON_SPAWN);
		int randomSpawnPointIndex = g_rng->GetRandomIntLessThan((int) demonSpawnPoints.size());
		Actor* spawnPoint = demonSpawnPoints[randomSpawnPointIndex];
		SpawnInfo info = {};
		info.m_faction = ACTOR_FACTION_DEMONS;
		info.m_definition = ActorDefinition::GetActorDefinition("Pinky");
		info.m_orientation = spawnPoint->GetOrientation();
		info.m_position = spawnPoint->GetPosition();
		map->SpawnActor(info);
	}
}


void GameMode::SpawnActorForPlayer(PlayerController* player)
{
	static Vec3 s_previouslySpawnedPosition;

	Map* map = m_world->GetCurrentMap();
	ActorList marineSpawnPoints = map->GetActorListByFaction(ACTOR_FACTION_MARINE_SPAWN);
	int randomMarinePointIndex = g_rng->GetRandomIntLessThan((int) marineSpawnPoints.size());
	Actor* spawnPoint = marineSpawnPoints[randomMarinePointIndex];

	while (spawnPoint->GetPosition() == s_previouslySpawnedPosition && marineSpawnPoints.size() > 1)
	{
		randomMarinePointIndex = g_rng->GetRandomIntLessThan((int) marineSpawnPoints.size());
		spawnPoint = marineSpawnPoints[randomMarinePointIndex];
	}

	SpawnInfo info = {};
	info.m_faction = ACTOR_FACTION_MARINES;
	info.m_definition = ActorDefinition::GetActorDefinition("Marine");
	info.m_orientation = spawnPoint->GetOrientation();
	info.m_position = spawnPoint->GetPosition();
	Actor* spawnedActor = map->SpawnActor(info);

	s_previouslySpawnedPosition = info.m_position;
	player->PossessActor(spawnedActor);
	player->SetRespawnTime(m_playerRespawnTime);
}


bool GameMode::IsTimerPaused()
{
	return m_modeTimer.IsPaused();
}


void GameMode::ResumeTimer()
{
	m_modeTimer.Restart();
	m_demonSpawnTimer.Restart();
}
