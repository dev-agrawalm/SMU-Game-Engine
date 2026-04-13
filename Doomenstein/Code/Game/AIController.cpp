#include "Game/AIController.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

float g_followerMovementAngleDegress = 10.0f;
bool g_shouldAIMove = true;
float g_wanderDuration = 2.0f;
static int s_eventRegistrationflag = 0;

bool ToggleAIMovement(EventArgs& args)
{
	UNUSED(args);
	g_shouldAIMove = !g_shouldAIMove;
	return false;
}


AIController::AIController(World* world, std::string const aiType) : Controller(world)
{
	if (_stricmp(aiType.c_str(), "follower") == 0)
	{
		m_type = AI_TYPE_FOLLOWER;
	}

	if (s_eventRegistrationflag == 0)
	{
		g_eventSystem->SubscribeEventCallbackFunction("ToggleAIMovement", ToggleAIMovement);
		s_eventRegistrationflag++;
	}
}


AIController::~AIController()
{
	if (s_eventRegistrationflag == 1)
	{
		g_eventSystem->UnsubscribeEventCallbackFunction("ToggleAIMovement", ToggleAIMovement);
	}
}


void AIController::Think(float deltaSeconds)
{
	if (!m_possessedActorUID.IsValid())
		return;

	Actor* possessedActor = m_currentMap->GetActorWithUID(m_possessedActorUID);
	switch (m_type)
	{
		case AI_TYPE_FOLLOWER:
			Think_Follower(deltaSeconds, possessedActor);
			break;
		case AI_TYPE_INVALID:
		case AI_TYPE_COUNT:
		default:
			ERROR_RECOVERABLE("Invalid AI type for actor: " + possessedActor->m_definition->m_name);
			break;
	}
}


bool AIController::HasLineOfSight(Actor* possessedActor, Actor* targetActor)
{
	float sightRadius = possessedActor->GetDefinition()->m_sightRadius;
	float sightAngle = possessedActor->GetDefinition()->m_sightAngle;
	Vec3 actorToTarget = targetActor->GetPosition() - possessedActor->GetPosition();
	float distanceFromTarget = actorToTarget.GetLength();
	if (distanceFromTarget > sightRadius)
		return false;

	Vec3 actorFwd = possessedActor->GetForwardVector();
	Vec2 actorToEnemyXY = actorToTarget.GetVec2();
	Vec2 actorFwdXY = actorFwd.GetVec2();
	float cosYawDelta = DotProduct2D(actorFwdXY.GetNormalized(), actorToEnemyXY.GetNormalized());
	if (cosYawDelta < CosDegrees(sightAngle * 0.5f))
		return false;

	RaycastResult raycastResult = m_currentMap->RayCastAll(possessedActor->GetEyePosition(), actorToTarget.GetNormalized(), distanceFromTarget, m_possessedActorUID, true);
	if (raycastResult.m_didImpact && raycastResult.m_impactActorUID.IsValid() && raycastResult.m_impactActorUID == targetActor->GetActorUID())
		return true;

	return false;
}


void AIController::Think_Follower(float deltaSeconds, Actor* possessedActor)
{
	UNUSED(deltaSeconds);
	if (possessedActor->IsDead())
		return;

	ActorFaction enemyFaction = possessedActor->m_faction == ACTOR_FACTION_DEMONS ? ACTOR_FACTION_MARINES : ACTOR_FACTION_DEMONS;
	ActorList enemyActors = m_currentMap->GetActorListByFaction(enemyFaction);
	for (int enemyIndex = 0; enemyIndex < (int) enemyActors.size(); enemyIndex++)
	{
		Actor*& enemy = enemyActors[enemyIndex];
		if (enemy && !enemy->IsDead())
		{
			ActorUID enemyUID = enemy->GetActorUID();
			if ((m_targetActorUID.IsValid() && m_targetActorUID == enemyUID) || !m_targetActorUID.IsValid())
			{
				if (HasLineOfSight(possessedActor, enemy))
				{
					m_targetActorUID = enemyUID;
					Vec3 enemyPos = enemy->GetPosition();
					Vec3 possessedActorPos = possessedActor->GetPosition();
					Vec3 actorFwd = possessedActor->GetForwardVector();
					Vec3 actorToEnemy = enemyPos - possessedActorPos;
					Vec2 actorToEnemyXY = actorToEnemy.GetVec2();
					Vec2 actorFwdXY = actorFwd.GetVec2();

					float yawDelta = GetAngleDegreesBetweenVectors2D(actorFwdXY, actorToEnemyXY);
					float dotWithLeft = DotProduct2D(actorToEnemyXY, possessedActor->GetLeftVector().GetVec2());
					if (dotWithLeft < 0)
						yawDelta *= -1;
					possessedActor->TurnByOrientation(EulerAngles(yawDelta, 0.0f, 0.0f));
					float distanceToEnemySqrd = actorToEnemyXY.GetLength();
					float stoppingDistanceSqrd = Square(enemy->GetCollisionRadius() + possessedActor->GetCollisionRadius());
					if (yawDelta <= g_followerMovementAngleDegress && distanceToEnemySqrd > stoppingDistanceSqrd)
					{
						if (g_shouldAIMove)
							possessedActor->MoveInDirection(actorToEnemy, 1.0f, 0.7f);
						possessedActor->SetAnimationToPlay("Walk");
						possessedActor->SetAnimationSpeed("Walk", 1.3f);
					}
					else if (distanceToEnemySqrd <= stoppingDistanceSqrd)
					{
						possessedActor->SetAnimationToPlay("Attack");
					}
					return;
				}
			}
		}
	}

	m_targetActorUID.Invalidate();
	Think_FollowerWander(possessedActor);
}


void AIController::Think_FollowerWander(Actor* possessedActor)
{
	static Stopwatch wanderStopwatch(g_game->GetGameStateClock(), g_wanderDuration);
	static Vec2 randomDirection(g_rng->GetRandomFloatInRange(-10.0f, 10.0f), g_rng->GetRandomFloatInRange(-10.0f, 10.0f));

	if (!wanderStopwatch.CheckAndDecrement())
	{
		randomDirection.GetNormalized();
		Vec3 actorFwd = possessedActor->GetForwardVector();
		Vec2 actorFwdXY = actorFwd.GetVec2();
		float yawDelta = GetAngleDegreesBetweenVectors2D(actorFwdXY, randomDirection);
		float dotWithLeft = DotProduct2D(randomDirection, possessedActor->GetLeftVector().GetVec2());
		if (dotWithLeft < 0)
			yawDelta *= -1;
		possessedActor->TurnByOrientation(EulerAngles(yawDelta, 0.0f, 0.0f));
		if (yawDelta <= g_followerMovementAngleDegress)
		{
			if (g_shouldAIMove)
			{
				possessedActor->MoveInDirection(randomDirection.GetVec3(), 1.0f, 0.2f);
			}
			possessedActor->SetAnimationSpeed("Walk", 0.7f);
			possessedActor->SetAnimationToPlay("Walk");
		}
	}
	else
	{
		randomDirection = Vec2(g_rng->GetRandomFloatInRange(-10.0f, 10.0f), g_rng->GetRandomFloatInRange(-10.0f, 10.0f)).GetNormalized();
	}
}
