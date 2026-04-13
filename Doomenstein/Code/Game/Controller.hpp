#pragma once
#include "Game/ActorUID.hpp"

#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"

class Actor; 
class World; 
class Map;

//------------------------------------------------------------------------------------------------
class Controller
{
friend class Actor; 

public:	
	Controller( World* world ); 
	virtual ~Controller(); 

	virtual void OnEnterWorld(); 
	virtual void OnExitWorld(); 

	virtual void OnEnterMap(Map* map);							// player is added to a map
	virtual void OnExitMap( Map* map);							// player is removed from a amp
	
	virtual void PreMapUpdate(float deltaSeconds);							// called before update - resets per frame data
	virtual void PostMapUpdate(float deltaSeconds);							 // called after update - set the camera (player)

	virtual void OnActorPossessed( Actor* actor ); 	
	virtual void OnActorUnpossessed( Actor* oldActor ); 
	virtual void OnPossessedActorDamaged();
	virtual void OnPossessedActorKilled();

	virtual void Think( float deltaSeconds ); 

	void SetCurrentMap( Map* map ); 
	Map* GetCurrentMap() const				{ return m_currentMap; }

	void PossessActor( Actor* actor ); 
	Actor* GetPossessedActor() const; 
	Actor* PossessRandomActor();

protected:
	World* m_world				= nullptr;  // world who created this player
	Map* m_currentMap			= nullptr;	// map this player is currently in
	ActorUID m_possessedActorUID = ActorUID::INVALID; 
}; 
