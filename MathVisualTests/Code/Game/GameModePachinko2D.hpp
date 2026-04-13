#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
#include "Game/Shapes2D.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Core/Stopwatch.hpp"

struct Pachinko2DBall : public Disk2D
{
	Vec2 m_velocity;
	float m_elasticity = 0.0f;
	Rgba8 m_color;
};


struct Pachinko2DBumper
{
public:
	bool BounceBallOutOfBumper(Pachinko2DBall& ball);
	bool IsBallOverlappingBounds(Pachinko2DBall const& ball);

public:
	float m_boundingRadius = 0.0f;
	Shape2D* m_shape = nullptr;
	Rgba8 m_color;
	float m_elasticity;
};



class GameModePachinko2D
{
public:
	GameModePachinko2D();
	~GameModePachinko2D();
	void Init();
	void DeInit();
	void Update(float deltaSeconds);
	void PhysicsUpdate(float deltaSeconds);
	void Render() const;
	void CheckInput(float deltaSeconds);

	Pachinko2DBumper GenerateRandomBumper(int bumperToken);

private:
	void CheckCollisionsBwBilliards();
	void CheckCollisionsBwBilliardsAndWalls();
	void CheckCollisionsBwBilliardsAndBumper();
	void UpdateVertArray();

	void CheckAndResolveCollisionBwPachinkoBalls(Pachinko2DBall& ballA, Pachinko2DBall& ballB);
	void CheckAndResolveCollisionBwBilliardAndBumper(Pachinko2DBall& ball, Pachinko2DBumper& bumper);
	void CheckAndResolveCollisionBwBallAndWall(Pachinko2DBall& ball);

	void UpdateBalls(float deltaSeconds);
	void GenerateRandomBumpers();
	Pachinko2DBumper GenerateRandomShape(Shape2DName shapeName);
private:
	std::vector<Pachinko2DBall> m_balls;
	std::vector<Pachinko2DBumper> m_bumpers;
	std::vector<Vertex_PCU> m_verts;

	Vec2 m_raycastStart;
	Vec2 m_raycastEnd;

	bool m_clampBilliardElasticity = true;

	float m_raycastSpeed = 0.0f;
	float m_ballRadius = 0.0f;
	float m_wallElasticity = 0.0f;
	float m_ballElasticity = 0.0f;
	float m_billiardElasticitySpeed = 0.0f;
	Rgba8 m_ballColor;
	Rgba8 m_hitDiskTint;

	float m_gravity = 0.0f;

	float m_physicsTimeStep = 0.0f;
	Stopwatch m_physicsTimer;
	bool m_isFloor = true;

	bool m_physicsControl = true;
};
