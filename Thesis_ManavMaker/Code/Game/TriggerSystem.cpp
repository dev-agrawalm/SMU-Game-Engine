#include "Game/TriggerSystem.hpp"
#include "Game/Level.hpp"

TriggerCommand::TriggerCommand(Level* level)
	: m_level(level)
{

}


EntitySpawnTriggerCommand::EntitySpawnTriggerCommand(Level* level, EntitySpawnInfo const& spawnInfo)
	: TriggerCommand(level)
	, m_spawnInfo(spawnInfo)
{

}


EntitySpawnTriggerCommand::~EntitySpawnTriggerCommand()
{

}


bool EntitySpawnTriggerCommand::Execute()
{
	m_triggeredCount++;
	Entity* spawnedEntity = m_level->SpawnEntity(m_spawnInfo);
	if (spawnedEntity)
		return true;
	else
		return false;
}


bool TriggerResponse::Execute()
{
	if (m_triggerResponseCommand)
		return m_triggerResponseCommand->Execute();

	return false;
}
