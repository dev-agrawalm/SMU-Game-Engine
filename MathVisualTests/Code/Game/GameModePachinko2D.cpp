#include "Game/GameModePachinko2D.hpp"	
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Game/Game.hpp"

GameModePachinko2D::GameModePachinko2D()
{
	m_raycastSpeed = g_gameConfigBlackboard.GetValue("biiliards2DRaycastSpeed", 10.0f);
	m_ballRadius = g_gameConfigBlackboard.GetValue("biiliards2DBilliardRadius", 1.0f);
	m_hitDiskTint = g_gameConfigBlackboard.GetValue("billiards2DHitBumperTint", Rgba8(200, 200, 200, 255));
	m_wallElasticity = 0.9f;
	m_ballElasticity = 0.9f;
	m_billiardElasticitySpeed = 2.0f;
	m_ballColor = Rgba8(34, 65, 115, 255);

	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);
	m_raycastStart = Vec2(worldX * 0.25f, worldY * 0.5f);
	m_raycastEnd = Vec2(worldX * 0.75f, worldY * 0.5f);

	m_gravity = g_gameConfigBlackboard.GetValue("pachinkoGravity", 9.8f);

	m_physicsTimeStep = g_gameConfigBlackboard.GetValue("pachikoPhysicsTimeStep", 0.05f);

	m_verts.resize(100000);
}


GameModePachinko2D::~GameModePachinko2D()
{
	DeInit();
}


void GameModePachinko2D::Init()
{
	GenerateRandomBumpers();
	m_physicsTimer = Stopwatch(g_game->GetClock(), m_physicsTimeStep);
}


void GameModePachinko2D::DeInit()
{

}


void GameModePachinko2D::Update(float deltaSeconds)
{
	CheckInput(deltaSeconds);
	if (m_clampBilliardElasticity)
	{
		m_ballElasticity = ClampZeroToOne(m_ballElasticity);
	}

	while (m_physicsTimer.CheckAndDecrement() && m_physicsTimer.GetDuration() > 0.0)
	{
		PhysicsUpdate(m_physicsTimeStep);
	}

	UpdateVertArray();
}


void GameModePachinko2D::PhysicsUpdate(float deltaSeconds)
{
	UpdateBalls(deltaSeconds);
	CheckCollisionsBwBilliards();
	CheckCollisionsBwBilliardsAndBumper();
	CheckCollisionsBwBilliardsAndWalls();
}


void GameModePachinko2D::Render() const
{
	DrawRing(m_raycastStart, m_ballRadius, 0.2f, m_ballColor);
	g_theRenderer->DrawVertexArray((int) m_verts.size(), m_verts.data());

	if (m_physicsControl)
	{
		float uiX = g_gameConfigBlackboard.GetValue("uiCamOrthoSizeX", 0.0f);
		std::string isClamped = m_clampBilliardElasticity ? "(Clamped)" : "(Free)";
		DebugAddScreenText(Stringf("Billiard Elasticity(Q/E): %f ", m_ballElasticity) + isClamped, Vec2(0.0f, 0.0f), 0.0f, Vec2(0.0f, 0.0f), 12.0f, Rgba8::YELLOW);
		DebugAddScreenText(Stringf("F2: reset to 0.9 | F3: clamp/unclamp"), Vec2(0.0f, 13.0f), 0.0f, Vec2(0.0f, 0.0f), 12.0f, Rgba8::YELLOW);
		DebugAddScreenText(Stringf("Ball count: %i", (int) m_balls.size()), Vec2(0.0f, 26.0f), 0.0f, Vec2(0.0f, 0.0f), 12.0f);
		DebugAddScreenText(Stringf("Physics Time Step(Z/X): %.5f", m_physicsTimeStep), Vec2(uiX, 0.0f), 0.0f, Vec2(1.0f, 0.0f), 12.0f, Rgba8::YELLOW);
		DebugAddScreenText(Stringf("Gravity(N/M): %.2f", m_gravity), Vec2(uiX, 13.0f), 0.0f, Vec2(1.0f, 0.0f), 12.0f, Rgba8::YELLOW);
		float frameTime = (float) g_game->GetClock().GetFrameDeltaSeconds();
		DebugAddScreenText(Stringf("Frame time: %.4f | FPS: %.2f", frameTime, (1.0f / frameTime)), Vec2(uiX, 26.0f), 0.0f, Vec2(1.0f, 0.0f), 12.0f);
	}
}


void GameModePachinko2D::CheckInput(float deltaSeconds)
{
	if (g_inputSystem->IsKeyPressed('W'))
	{
		m_raycastStart += Vec2(0.0f, 1.0f) * m_raycastSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('A'))
	{
		m_raycastStart += Vec2(-1.0f, 0.0f) * m_raycastSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('S'))
	{
		m_raycastStart += Vec2(0.0f, -1.0f) * m_raycastSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('D'))
	{
		m_raycastStart += Vec2(1.0f, 0.0f) * m_raycastSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('I'))
	{
		m_raycastEnd += Vec2(0.0f, 1.0f) * m_raycastSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('J'))
	{
		m_raycastEnd += Vec2(-1.0f, 0.0f) * m_raycastSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('K'))
	{
		m_raycastEnd += Vec2(0.0f, -1.0f) * m_raycastSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('L'))
	{
		m_raycastEnd += Vec2(1.0f, 0.0f) * m_raycastSpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('E') && m_physicsControl)
	{
		m_ballElasticity += m_billiardElasticitySpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('Q') && m_physicsControl)
	{
		m_ballElasticity -= m_billiardElasticitySpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('X') && m_physicsControl)
	{
		m_physicsTimeStep *= 1.01f/*+= 0.075f * deltaSeconds*/;
		m_physicsTimer.SetDuration(m_physicsTimeStep);
	}

	if (g_inputSystem->IsKeyPressed('Z') && m_physicsControl)
	{
		m_physicsTimeStep *= 0.99f/*-= 0.075f * deltaSeconds*/;
		m_physicsTimeStep = ClampZeroToOne(m_physicsTimeStep);
		m_physicsTimer.SetDuration(m_physicsTimeStep);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		m_clampBilliardElasticity = !m_clampBilliardElasticity;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		m_ballElasticity = 0.9f;
	}

	if (g_inputSystem->IsKeyPressed('M') && m_physicsControl)
	{
		m_gravity += 10.0f * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('N') && m_physicsControl)
	{
		m_gravity -= 10.0f * deltaSeconds;
	}

	if (g_inputSystem->WasKeyJustPressed('F'))
	{
		m_isFloor = !m_isFloor;
	}

	if (g_inputSystem->IsKeyPressed(KEYCODE_SPACE_BAR))
	{
		Vec2 center = m_raycastStart;
		Vec2 velocity = (m_raycastEnd - m_raycastStart) * 0.75f;
		Pachinko2DBall ball = {};
		ball.m_center = center;
		ball.m_radius = m_ballRadius;
		ball.m_velocity = velocity;
		ball.m_elasticity = m_ballElasticity;
		float randomT = g_rng->GetRandomFloatZeroToOne();
		ball.m_color = Lerp(m_ballColor, Rgba8::WHITE, randomT);
		m_balls.push_back(ball);
	}
	
// 	if (g_inputSystem->WasKeyJustPressed('G'))
// 	{
// 		for (int i = 0; i < 50; i++)
// 		{
// 			Vec2 center = m_raycastStart;
// 			Vec2 velocity = (m_raycastEnd - m_raycastStart) * 0.75f;
// 			Pachinko2DBall ball = {};
// 			ball.m_center = center;
// 			ball.m_radius = m_ballRadius;
// 			ball.m_velocity = velocity;
// 			ball.m_elasticity = m_ballElasticity;
// 			float randomT = g_rng->GetRandomFloatZeroToOne();
// 			ball.m_color = Lerp(m_ballColor, Rgba8::WHITE, randomT);
// 			m_balls.push_back(ball);
// 		}
// 	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
	{
		m_balls.clear();
		GenerateRandomBumpers();
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F4))
	{
		m_physicsControl = !m_physicsControl;
	}
}


void GameModePachinko2D::CheckCollisionsBwBilliards()
{
	for (int billiardAIndex = 0; billiardAIndex < m_balls.size(); billiardAIndex++)
	{
		for (int billiardBIndex = 0; billiardBIndex <  m_balls.size(); billiardBIndex++)
		{
			if (billiardAIndex != billiardBIndex)
			{
				Pachinko2DBall& ballA = m_balls[billiardAIndex];
				Pachinko2DBall& ballB = m_balls[billiardBIndex];
				CheckAndResolveCollisionBwPachinkoBalls(ballA, ballB);
			}
		}
	}
}


void GameModePachinko2D::CheckCollisionsBwBilliardsAndWalls()
{
	for (int ballIndex = 0; ballIndex < m_balls.size(); ballIndex++)
	{
		Pachinko2DBall& ball = m_balls[ballIndex];
		CheckAndResolveCollisionBwBallAndWall(ball);
	}
}


void GameModePachinko2D::CheckCollisionsBwBilliardsAndBumper()
{
	for (int ballIndex = 0; ballIndex < m_balls.size(); ballIndex++)
	{
		for (int bumperIndex = 0; bumperIndex < m_bumpers.size(); bumperIndex++)
		{
			Pachinko2DBall& billiard = m_balls[ballIndex];
			Pachinko2DBumper& bumper = m_bumpers[bumperIndex];
			CheckAndResolveCollisionBwBilliardAndBumper(billiard, bumper);
		}
	}
}


void GameModePachinko2D::UpdateVertArray()
{
	m_verts.clear();
	float lineWidth = 0.3f;

	AddVertsForArrow2DToVector(m_verts, m_raycastStart, m_raycastEnd, Rgba8::WHITE, lineWidth, 0.05f);

	for (int bumperIndex = 0; bumperIndex < m_bumpers.size(); bumperIndex++)
	{
		Pachinko2DBumper bumper = m_bumpers[bumperIndex];
		bumper.m_shape->AddVertsForShapeToVector(m_verts, bumper.m_color);
	}	

	for (int bumperIndex = 0; bumperIndex < m_balls.size(); bumperIndex++)
	{
		Pachinko2DBall ball = m_balls[bumperIndex];
		AddVertsForDisk2DToVector(m_verts, ball.m_center, ball.m_radius, ball.m_color, 16);
	}
}


void GameModePachinko2D::CheckAndResolveCollisionBwPachinkoBalls(Pachinko2DBall& ballA, Pachinko2DBall& ballB)
{
	if (PushDisksOutOfEachOther2D(ballA.m_center, ballA.m_radius, ballB.m_center, ballB.m_radius))
	{
		Vec2 centerA = ballA.m_center;
		Vec2 centerB = ballB.m_center;
		Vec2 impactNormalA = (centerA - centerB).GetNormalized();
		float aSpeedAlongNormal = DotProduct2D(ballA.m_velocity, impactNormalA);
		float bSpeedAlongNormal = DotProduct2D(ballB.m_velocity, impactNormalA);
		float velocityBRelativeToA = bSpeedAlongNormal - aSpeedAlongNormal;
		if (velocityBRelativeToA > 0.0f)
		{
			Vec2& velocityA = ballA.m_velocity;
			Vec2 aVelocityAlongNormal = GetProjectedOnto2D(velocityA, impactNormalA);
			Vec2 aVelocityAlongTangent = velocityA - aVelocityAlongNormal;

			Vec2& velocityB = ballB.m_velocity;
			Vec2 impactNormalB = -impactNormalA;
			Vec2 bVelocityAlongNormal = GetProjectedOnto2D(velocityB, impactNormalB);
			Vec2 bVelocityAlongTangent = velocityB - bVelocityAlongNormal;

			float elasticityCoefficient = ballA.m_elasticity * ballB.m_elasticity;
			velocityA = aVelocityAlongTangent + bVelocityAlongNormal * elasticityCoefficient;
			velocityB = bVelocityAlongTangent + aVelocityAlongNormal * elasticityCoefficient;
		}
	}
}


void GameModePachinko2D::CheckAndResolveCollisionBwBilliardAndBumper(Pachinko2DBall& ball, Pachinko2DBumper& bumper)
{
	if (bumper.IsBallOverlappingBounds(ball))
	{
		bumper.BounceBallOutOfBumper(ball);
	}
}


void GameModePachinko2D::CheckAndResolveCollisionBwBallAndWall(Pachinko2DBall& ball)
{
	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);
	Vec2& center = ball.m_center;
	float radius = ball.m_radius;
	float elasticityCoefficient = ball.m_elasticity * m_wallElasticity;
	if (center.x < radius)
	{
		center.x = radius;
		Vec2& velocity = ball.m_velocity;
		Vec2 impactNormal = Vec2(1.0f, 0.0f);
		Vec2 velocityAlongNormal = GetProjectedOnto2D(velocity, impactNormal);
		Vec2 velocityAlongTangent = velocity - velocityAlongNormal;

		velocity = velocityAlongTangent - velocityAlongNormal * elasticityCoefficient;
	}

	if (center.y < radius && m_isFloor)
	{
			center.y = radius;
			Vec2& velocity = ball.m_velocity;
			Vec2 impactNormal = Vec2(0.0f, 1.0f);
			Vec2 velocityAlongNormal = GetProjectedOnto2D(velocity, impactNormal);
			Vec2 velocityAlongTangent = velocity - velocityAlongNormal;
			velocity = velocityAlongTangent - velocityAlongNormal * elasticityCoefficient;
	}

	if (center.y < -radius && !m_isFloor)
	{
		center = Vec2(center.x, worldY + radius * 1.5f);
	}

	if (worldX - center.x < radius)
	{
		center.x = worldX - radius;
		Vec2& velocity = ball.m_velocity;
		Vec2 impactNormal = Vec2(-1.0f, 0.0f);
		Vec2 velocityAlongNormal = GetProjectedOnto2D(velocity, impactNormal);
		Vec2 velocityAlongTangent = velocity - velocityAlongNormal;

		velocity = velocityAlongTangent - velocityAlongNormal * elasticityCoefficient;
	}

// 	if (worldY - center.y < radius)
// 	{
// 		center.y = worldY - radius;
// 		Vec2& velocity = ball.m_velocity;
// 		Vec2 impactNormal = Vec2(0.0f, -1.0f);
// 		Vec2 velocityAlongNormal = GetProjectedOnto2D(velocity, impactNormal);
// 		Vec2 velocityAlongTangent = velocity - velocityAlongNormal;
// 
// 		velocity = velocityAlongTangent - velocityAlongNormal * elasticityCoefficient;
// 	}
}


void GameModePachinko2D::GenerateRandomBumpers()
{
	m_bumpers.clear();
	//int minBumperCount = 15;
	//int maxBumperCount = 20;
	int bumperCount = 30; // g_rng->GetRandomIntInRange(minBumperCount, maxBumperCount);
	for (int bumperIndex = 0; bumperIndex < bumperCount; bumperIndex++)
	{
		int token = bumperIndex % 3;
		Pachinko2DBumper bumper = GenerateRandomBumper(token);
		m_bumpers.push_back(bumper);
	}
}


Pachinko2DBumper GameModePachinko2D::GenerateRandomShape(Shape2DName shapeName)
{
	Pachinko2DBumper bumper;
	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);

	Rgba8 elasticColor = Rgba8::GREEN;
	Rgba8 inElasticColor = Rgba8::RED;

	Shape2D* shape = nullptr;
	switch (shapeName)
	{
		case SHAPE_CAPSULE_2D:
		{
			Capsule2 capsule;
			capsule.m_radius = g_rng->GetRandomFloatInRange(0.5f, 3.0f);
			Vec2 startMins = Vec2(capsule.m_radius, worldY * 0.2f);
			Vec2 startMaxs = Vec2(worldX - capsule.m_radius, worldY - capsule.m_radius);
			Vec2 start = g_rng->GetRandomPositionInAABB2(startMins, startMaxs);
			LineSegment2 lineSegment;
			g_game->RandomizeLineSegment2D(lineSegment);
			float length = g_rng->GetRandomFloatInRange(1.0f, 5.0f);
			Vec2 end = start + lineSegment.GetDirection() * length;
			capsule.m_bone = LineSegment2(start, end);
			shape = new Capsule2D(capsule);
			bumper.m_boundingRadius = capsule.m_bone.GetDistance() * 0.5f + capsule.m_radius;
			break;
		}
		case SHAPE_OBB_2D:
		{
			OBB2 obb;
			LineSegment2 lineSegment;
			g_game->RandomizeLineSegment2D(lineSegment);
			obb.m_iBasisNormal = lineSegment.GetDirection();
			obb.m_halfDims.x = g_rng->GetRandomFloatInRange(0.5f, 5.0f);
			obb.m_halfDims.y = g_rng->GetRandomFloatInRange(0.5f, 5.0f);
			obb.m_center = g_rng->GetRandomPositionInAABB2(Vec2(obb.m_halfDims.x, worldY * 0.2f), Vec2(worldX - obb.m_halfDims.x, worldY - obb.m_halfDims.y));
			shape = new OBB2D(obb);
			float obbDiagSqrd = Square(obb.m_halfDims.x) + Square(obb.m_halfDims.y);
			bumper.m_boundingRadius = SqrtFloat(obbDiagSqrd);
			break;
		}
		case SHAPE_DISK_2D:
		{
			float randomRadius = g_rng->GetRandomFloatInRange(2.0f, 5.0f);
			Vec2 randomCenter = g_rng->GetRandomPositionInAABB2(Vec2(randomRadius, worldY * 0.3f), Vec2(worldX - randomRadius, worldY - randomRadius));
			shape = new Disk2D(randomRadius, randomCenter);
			bumper.m_boundingRadius = randomRadius;
			break;
		}
		default:
			break;
	}
	bumper.m_shape = shape;
	bumper.m_elasticity = g_rng->GetRandomFloatInRange(0.01f, 0.99f);
	bumper.m_color = Lerp(inElasticColor, elasticColor, bumper.m_elasticity);
	return bumper;
}


Pachinko2DBumper GameModePachinko2D::GenerateRandomBumper(int bumperToken)
{
	Pachinko2DBumper bumper;
	
	//int shapeToken = g_rng->GetRandomIntInRange(0, 2);
	switch (bumperToken)
	{
		case 0:
			bumper = GenerateRandomShape(SHAPE_DISK_2D);
			break;
		case 1:
			bumper = GenerateRandomShape(SHAPE_CAPSULE_2D);
			break;
		case 2:
			bumper = GenerateRandomShape(SHAPE_OBB_2D);
			break;
		default:
			break;
	}

	return bumper;
}


void GameModePachinko2D::UpdateBalls(float deltaSeconds)
{
	for (int ballIndex = 0; ballIndex < m_balls.size(); ballIndex++)
	{
		Pachinko2DBall& billiard = m_balls[ballIndex];
		Vec2& billiardCenter = billiard.m_center;
		Vec2& billiardVelocity = billiard.m_velocity;
		billiardVelocity += Vec2(0.0f, -1.0f) * m_gravity * deltaSeconds;
		billiardCenter += billiardVelocity * deltaSeconds;
	}
}


bool Pachinko2DBumper::BounceBallOutOfBumper(Pachinko2DBall& ball)
{
	if (m_shape->PushDiskOutOfShape(ball.m_center, ball.m_radius))
	{
		Vec2 centerA = ball.m_center;
		Vec2 centerB = m_shape->GetNearestPoint(centerA);
		Vec2 impactNormalA = (centerA - centerB).GetNormalized();
		float aSpeedAlongNormal = DotProduct2D(ball.m_velocity, impactNormalA);
		float bSpeedAlongNormal = DotProduct2D(Vec2(0.0f, 0.0f), impactNormalA);
		float velocityBRelativeToA = bSpeedAlongNormal - aSpeedAlongNormal;
		if (velocityBRelativeToA > 0.0f)
		{
			Vec2& velocityA = ball.m_velocity;
			Vec2 aVelocityAlongNormal = GetProjectedOnto2D(velocityA, impactNormalA);
			Vec2 aVelocityAlongTangent = velocityA - aVelocityAlongNormal;

			float elasticityCoefficient = ball.m_elasticity * m_elasticity;
			velocityA = aVelocityAlongTangent - aVelocityAlongNormal * elasticityCoefficient;
			return true;
		}
	}
	return false;
}


bool Pachinko2DBumper::IsBallOverlappingBounds(Pachinko2DBall const& ball)
{
	Vec2 ballCenter = ball.m_center;
	float ballRadius = ball.m_radius;
	Vec2 boundCenter = m_shape->GetCenter();
	float boundRadius = m_boundingRadius;
	return DoDiscsOverlap2D(boundCenter, boundRadius, ballCenter, ballRadius);
}

