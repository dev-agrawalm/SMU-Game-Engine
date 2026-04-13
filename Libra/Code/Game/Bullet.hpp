#pragma once
#include "Game/Entity.hpp"
#include<string>

struct Vec2;
struct AABB2;

class Bullet : public Entity
{
public:
	Bullet();
	~Bullet();
	Bullet(Vec2 const& position, float orientationDegrees, EntityFaction faction, EntityType type, Map* map);

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;
	virtual void TakeDamage(int damage) override;
	
	void SetTargetEntity(Entity* target);
protected:
	void UpdateBullet(float deltaSeconds);
	void UpdateGuidedMissle(float deltaSeconds);
	void CheckAndCorrectBulletIfSpawnedInSolidObject();
	void CheckCollisionsWithNonBulletEntitiesOfTargetFaction();
	void CheckCollisionsWithSolidObjects(float deltaSeconds);  //includes solid tiles and the aries shield
	void CheckAndCorrectCollisionsWithTiles(Vec2 const& newPosition, Vec2 const& forwardNormal, float distanceToNewPosition);
	void CheckAndCorrectCollisionsWithAries(Vec2 const& newPosition, Vec2 const& forwardNormal);

private:
	int m_damage = 0;
	std::string m_texturePath = "";
	Entity* m_targetEntity = nullptr;
};