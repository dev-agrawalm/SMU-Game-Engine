#include "Game/Controller.hpp"
#include "Game/World.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"

Controller::Controller(World* world)
{
	m_world = world;
	m_currentMap = m_world->GetCurrentMap();
}


Controller::~Controller()
{

}


void Controller::OnEnterWorld()
{

}


void Controller::OnExitWorld()
{

}


void Controller::OnEnterMap(Map* map)
{
	SetCurrentMap(map);
}


void Controller::OnExitMap(Map* map)
{
	UNUSED(map);
	SetCurrentMap(nullptr);
}


void Controller::PreMapUpdate(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Controller::PostMapUpdate(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Controller::OnActorPossessed(Actor* actor)
{
	if (actor != nullptr)
	{
		m_possessedActorUID = actor->GetActorUID();
		actor->OnPossessed(this);
	}
	else
	{
		m_possessedActorUID.Invalidate();
	}
}


void Controller::OnActorUnpossessed(Actor* oldActor)
{
	m_possessedActorUID.Invalidate();
	oldActor->OnUnpossessed(this);
}


void Controller::OnPossessedActorDamaged()
{

}


void Controller::OnPossessedActorKilled()
{

}


void Controller::Think(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}


void Controller::SetCurrentMap(Map* map)
{
	m_currentMap = map;
}


void Controller::PossessActor(Actor* actor)
{
	if (actor && !actor->IsPossessable())
		return;
	
	Actor* possesedActor = GetPossessedActor();
	if (possesedActor != nullptr)
	{
		if (possesedActor == actor)
			return;
		
		OnActorUnpossessed(possesedActor);
	}

	OnActorPossessed(actor);
}


Actor* Controller::GetPossessedActor() const
{
	if (!m_possessedActorUID.IsValid())
		return nullptr;

	Actor* possesedActor = m_currentMap->GetActorWithUID(m_possessedActorUID);
	return possesedActor;
}


Actor* Controller::PossessRandomActor()
{
	Actor* actor = m_currentMap->GetFirstUnpossesedActor();
	if (actor)
	{
		PossessActor(actor);
		return actor;
	}

	return nullptr;
}