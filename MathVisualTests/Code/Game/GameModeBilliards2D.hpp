#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include "Game/Shapes2D.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RaycastUtils.hpp"

enum GameDiskType
{
	GAME_DISK_TYPE_BILLIARD = 0,
	GAME_DISK_TYPE_BUMPER,
	GAME_DISK_TYPE_COUNT
};


struct GameDisk : public Disk2D
{
	Vec2 m_velocity;
	float m_elasticity = 0.0f;
	Rgba8 m_color = Rgba8::WHITE;
	Rgba8 m_tint = Rgba8::WHITE;
};


typedef std::vector<GameDisk> DiskList;

class GameModeBilliards2D
{
public:
	GameModeBilliards2D();
	~GameModeBilliards2D();
	void Init();
	void DeInit();
	void Update(float deltaSeconds);
	void Render() const;
	void CheckInput(float deltaSeconds);

	GameDisk GenerateRandomBumper();
	bool ShouldCorrectVelocity(GameDisk diskA, GameDisk diskB) const;
	bool ShouldCorrectPosition(GameDisk diskA, GameDisk diskB) const;

private:
	void CheckCollisionsBwBilliards();
	void CheckCollisionsBwBilliardsAndWalls();
	void CheckCollisionsBwBilliardsAndBumper();
	void UpdateVertArray();

	void CheckAndResolveCollisionBwBilliards(GameDisk& billiardA, GameDisk& billiardB);
	void CheckAndResolveCollisionBwBilliardAndBumper(GameDisk& billiard, GameDisk& bumper);
	void CheckAndResolveCollisionBwBilliardAndWall(GameDisk& billiard);

	void UpdateBilliards(float deltaSeconds);
	void GenerateRandomBumpers();
	void RaycastDisks();
private:
	std::vector<Vertex_PCU> m_verts;
	DiskList m_disksByType[GAME_DISK_TYPE_COUNT];

	Vec2 m_raycastStart;
	Vec2 m_raycastEnd;
	bool m_doesRaycastHit = false;
	BaseRaycastResult2D m_raycastResult = {};

	bool m_clampBilliardElasticity = true;

	float m_raycastSpeed = 0.0f;
	float m_billiardRadius = 0.0f;
	float m_wallElasticity = 0.0f;
	float m_billiardElasticity = 0.0f;
	float m_billiardElasticitySpeed = 0.0f;
	Rgba8 m_billiardColor;
	Rgba8 m_hitDiskTint;
};
