#pragma once
#include "Engine/Core/Stopwatch.hpp"

class World;
class PlayerController;

class GameMode
{
public:
	GameMode(World* world);
	void Init();
	void Update();
	void SpawnActorForPlayer(PlayerController* player);
	
	bool IsTimerPaused();
	void ResumeTimer();
private:
	World* m_world = nullptr;

	int m_maxDemonCount = 0;
	float m_modeDuration = 0.0f;
	float m_playerRespawnTime = 0.0f;
	float m_demonSpawnTime = 0.0f;
	
	Stopwatch m_modeTimer;
	Stopwatch m_demonSpawnTimer;
};