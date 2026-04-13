#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Actor.hpp"
#include "Engine/Core/XmlUtils.hpp"


//------------------------------------------------------------------------------------------------
struct SpawnInfo 
{
	SpawnInfo() {}
	SpawnInfo( ActorFaction faction, char const* actorDefinitionName, Vec3 const& position, EulerAngles const& orient = EulerAngles(0.0f, 0.0f, 0.0f) ); 
	SpawnInfo(XmlElement const& element);
	// other convenience constructors...

	ActorDefinition const* m_definition		= nullptr; 
	
	Vec3 m_initialVelocity;
	Vec3 m_position; 
	EulerAngles m_orientation; 
	ActorFaction m_faction = ActorFaction::ACTOR_FACTION_UNKNOWN;
	ActorFlags m_flags = ActorFlagBit::ACTOR_FLAG_IS_ALIVE;
	ActorUID m_owningActor = ActorUID::INVALID;
}; 
