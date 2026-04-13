#include "Game/SpawnInfo.hpp"


SpawnInfo::SpawnInfo(ActorFaction faction, char const* actorDefinitionName, Vec3 const& position, EulerAngles const& orient /*= EulerAngles(0.0f, 0.0f, 0.0f) */) :
	m_faction(faction),
	m_position(position),
	m_orientation(orient)
{
	m_definition = ActorDefinition::GetActorDefinition(actorDefinitionName);
}


SpawnInfo::SpawnInfo(XmlElement const& element)
{
	std::string actorType = ParseXmlAttribute(element, "actor", "");
	std::string faction = ParseXmlAttribute(element, "faction", "");
	m_position = ParseXmlAttribute(element, "position", m_position);
	m_orientation.m_yaw = ParseXmlAttribute(element, "facing", m_orientation.m_yaw);

	m_definition = ActorDefinition::GetActorDefinition(actorType);

	if (faction == "marines")
	{
		m_faction = ACTOR_FACTION_MARINES;
	}
	else if (faction == "demons")
	{
		m_faction = ACTOR_FACTION_DEMONS;
	}
	else if (faction == "demonSpawn")
	{
		m_faction = ACTOR_FACTION_DEMON_SPAWN;
	}
	else if (faction == "marineSpawn")
	{
		m_faction = ACTOR_FACTION_MARINE_SPAWN;
	}
}

