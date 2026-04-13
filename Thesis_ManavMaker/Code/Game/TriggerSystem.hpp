#pragma once
#include "Game/EntitySpawnInfo.hpp"

class Level;

class TriggerCommand
{
public:
	TriggerCommand(Level* level);
	virtual ~TriggerCommand() { m_level = nullptr; m_triggeredCount = 0; }
	virtual bool Execute() = 0;

protected:
	int m_triggeredCount = 0;
	Level* m_level = nullptr;
};


class EntitySpawnTriggerCommand : public TriggerCommand
{
public:
	EntitySpawnTriggerCommand(Level* level, EntitySpawnInfo const& spawnInfo);
	~EntitySpawnTriggerCommand();
	virtual bool Execute() override;

public:
	EntitySpawnInfo m_spawnInfo = EntitySpawnInfo::s_INVALID_SPAWN_INFO;
};


struct TriggerResponse
{
public:
	bool Execute();

public:
	TriggerType m_triggerType = TRIGGER_TYPE_NONE;
	TriggerCommand* m_triggerResponseCommand = nullptr;
};
