#pragma once
#include "Game/Controller.hpp"

enum AIType
{
	AI_TYPE_INVALID = -1,
	AI_TYPE_FOLLOWER,
	AI_TYPE_COUNT
};


class AIController : public Controller
{

public:
	AIController(World* world, std::string const aiType);
	~AIController();
	virtual void Think(float deltaSeconds) override;

	bool HasLineOfSight(Actor* possessedActor, Actor* targetActor);
protected:
	void Think_Follower(float deltaSeconds, Actor* possessedActor);
	void Think_FollowerWander(Actor* possessedActor);

private:
	AIType m_type = AI_TYPE_INVALID;
	ActorUID m_targetActorUID = ActorUID::INVALID;
};
