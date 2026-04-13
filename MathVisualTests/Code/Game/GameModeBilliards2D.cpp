#include "Game/GameModeBilliards2D.hpp"	
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DebugRender.hpp"


GameModeBilliards2D::GameModeBilliards2D()
{
	Init();
	m_raycastSpeed = g_gameConfigBlackboard.GetValue("biiliards2DRaycastSpeed", 10.0f);
	m_billiardRadius = g_gameConfigBlackboard.GetValue("biiliards2DBilliardRadius", 1.0f);
	m_hitDiskTint = g_gameConfigBlackboard.GetValue("billiards2DHitBumperTint", Rgba8(200,200,200,255));
	m_wallElasticity = 0.9f;
	m_billiardElasticity = 0.9f;
	m_billiardElasticitySpeed = 2.0f;
	m_billiardColor = Rgba8(34, 65, 115, 255);

	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);
	m_raycastStart = Vec2(worldX * 0.25f, worldY * 0.5f);
	m_raycastEnd = Vec2(worldX * 0.75f, worldY * 0.5f);
}


GameModeBilliards2D::~GameModeBilliards2D()
{
	DeInit();
}


void GameModeBilliards2D::Init()
{
	GenerateRandomBumpers();
}


void GameModeBilliards2D::DeInit()
{

}


void GameModeBilliards2D::Update(float deltaSeconds)
{
	CheckInput(deltaSeconds);
	if (m_clampBilliardElasticity)
	{
		m_billiardElasticity = ClampZeroToOne(m_billiardElasticity);
	}
	RaycastDisks();
	UpdateBilliards(deltaSeconds);
	CheckCollisionsBwBilliards();
	CheckCollisionsBwBilliardsAndBumper();
	CheckCollisionsBwBilliardsAndWalls();
	UpdateVertArray();
}


void GameModeBilliards2D::Render() const
{
	DrawRing(m_raycastStart, m_billiardRadius, 0.2f, m_billiardColor);
	g_theRenderer->DrawVertexArray((int) m_verts.size(), m_verts.data());

	std::string isClamped = m_clampBilliardElasticity ? "(Clamped)" : "(Free)";
	DebugAddScreenText(Stringf("Billiard Elasticity: %f ", m_billiardElasticity) + isClamped, Vec2(0.0f, 0.0f), 0.0f, Vec2(0.0f, 0.0f), 12.0f, Rgba8::MAGENTA);
}


void GameModeBilliards2D::CheckInput(float deltaSeconds)
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

	if (g_inputSystem->IsKeyPressed('E'))
	{
		m_billiardElasticity += m_billiardElasticitySpeed * deltaSeconds;
	}

	if (g_inputSystem->IsKeyPressed('Q'))
	{
		m_billiardElasticity -= m_billiardElasticitySpeed * deltaSeconds;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		m_clampBilliardElasticity = !m_clampBilliardElasticity;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		m_billiardElasticity = 0.9f;
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR))
	{
		Vec2 center = m_raycastStart;
		Vec2 velocity = m_raycastEnd - m_raycastStart;
		GameDisk billiard = {};
		billiard.m_center = center;
		billiard.m_radius = m_billiardRadius;
		billiard.m_velocity = velocity;
		billiard.m_elasticity = m_billiardElasticity;
		billiard.m_color = m_billiardColor;
		m_disksByType[GAME_DISK_TYPE_BILLIARD].push_back(billiard);
	}

	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8))
	{
		m_disksByType[GAME_DISK_TYPE_BILLIARD].clear();
		GenerateRandomBumpers();
	}
}


bool GameModeBilliards2D::ShouldCorrectVelocity(GameDisk diskA, GameDisk diskB) const
{
	Vec2 centerA = diskA.m_center;
	Vec2 centerB = diskB.m_center;
	Vec2 impactNormalA = (centerA - centerB).GetNormalized();
	float aSpeedAlongNormal = DotProduct2D(diskA.m_velocity, impactNormalA);
	float bSpeedAlongNormal = DotProduct2D(diskB.m_velocity, impactNormalA);
	float velocityBRelativeToA = bSpeedAlongNormal - aSpeedAlongNormal;
	return velocityBRelativeToA > 0.0f;
}


bool GameModeBilliards2D::ShouldCorrectPosition(GameDisk diskA, GameDisk diskB) const
{
	Vec2 centerA = diskA.m_center;
	float radiusA = diskA.m_radius;
	Vec2 centerB = diskB.m_center;
	float radiusB = diskB.m_radius;

	return DoDiscsOverlap2D(centerA, radiusA, centerB, radiusB);
}


void GameModeBilliards2D::CheckCollisionsBwBilliards()
{
	DiskList& billiards = m_disksByType[GAME_DISK_TYPE_BILLIARD];
	for (int billiardAIndex = 0; billiardAIndex < billiards.size(); billiardAIndex++)
	{
		for (int billiardBIndex = 0; billiardBIndex < billiards.size(); billiardBIndex++)
		{
			if (billiardAIndex != billiardBIndex)
			{
				GameDisk& billiardA = billiards[billiardAIndex];
				GameDisk& billiardB = billiards[billiardBIndex];
				CheckAndResolveCollisionBwBilliards(billiardA, billiardB);
			}
		}
	}
}


void GameModeBilliards2D::CheckCollisionsBwBilliardsAndWalls()
{
	DiskList& billiards = m_disksByType[GAME_DISK_TYPE_BILLIARD];
	for (int billiardIndex = 0; billiardIndex < billiards.size(); billiardIndex++)
	{
		GameDisk& billiard = billiards[billiardIndex];
		CheckAndResolveCollisionBwBilliardAndWall(billiard);
	}
}


void GameModeBilliards2D::CheckCollisionsBwBilliardsAndBumper()
{
	DiskList& billiards = m_disksByType[GAME_DISK_TYPE_BILLIARD];
	DiskList& bumpers = m_disksByType[GAME_DISK_TYPE_BUMPER];
	
	for (int billiardIndex = 0; billiardIndex < billiards.size(); billiardIndex++)
	{
		for (int bumperIndex = 0; bumperIndex < bumpers.size(); bumperIndex++)
		{
			GameDisk& billiard = billiards[billiardIndex];
			GameDisk& bumper = bumpers[bumperIndex];
			CheckAndResolveCollisionBwBilliardAndBumper(billiard, bumper);
		}
	}
}


void GameModeBilliards2D::UpdateVertArray()
{
	m_verts.clear();
	float lineWidth = 0.3f;
	if (m_doesRaycastHit)
	{
		float impactNormalLength = 5.0f;

		Vec2& impactPoint = m_raycastResult.m_impactPoint;
		Vec2& impactNormal = m_raycastResult.m_impactNormal;
		AddVertsForArrow2DToVector(m_verts, m_raycastStart, m_raycastEnd, Rgba8::YELLOW, lineWidth, 0.05f);
		AddVertsForArrow2DToVector(m_verts, impactPoint, impactPoint + impactNormal * impactNormalLength, Rgba8::GREEN, lineWidth, 0.05f);
		AddVertsForDisk2DToVector(m_verts, impactPoint, 0.5f, Rgba8::CYAN);
	}
	else
	{
		AddVertsForArrow2DToVector(m_verts, m_raycastStart, m_raycastEnd, Rgba8::WHITE, lineWidth, 0.05f);
	}

	for (int diskType = 0; diskType < GAME_DISK_TYPE_COUNT; diskType++)
	{
		DiskList& diskList = m_disksByType[diskType];
		for (int diskIndex = 0; diskIndex < (int) diskList.size(); diskIndex++)
		{
			GameDisk& disk = diskList[diskIndex];
			Rgba8 color = disk.m_color * disk.m_tint;
			AddVertsForDisk2DToVector(m_verts, disk.m_center, disk.m_radius, color);
		}
	}
}


void GameModeBilliards2D::CheckAndResolveCollisionBwBilliards(GameDisk& billiardA, GameDisk& billiardB)
{
	if (!ShouldCorrectPosition(billiardA, billiardB))
		return;

	PushDisksOutOfEachOther2D(billiardA.m_center, billiardA.m_radius, billiardB.m_center, billiardB.m_radius);

	if (!ShouldCorrectVelocity(billiardA, billiardB))
		return;

	Vec2& velocityA = billiardA.m_velocity;
	Vec2 impactNormalA = (billiardA.m_center - billiardB.m_center).GetNormalized();
	Vec2 aVelocityAlongNormal = GetProjectedOnto2D(velocityA, impactNormalA);
	Vec2 aVelocityAlongTangent = velocityA - aVelocityAlongNormal;

	Vec2& velocityB = billiardB.m_velocity;
	Vec2 impactNormalB = -impactNormalA;
	Vec2 bVelocityAlongNormal = GetProjectedOnto2D(velocityB, impactNormalB);
	Vec2 bVelocityAlongTangent = velocityB - bVelocityAlongNormal;

	float elasticityCoefficient = billiardA.m_elasticity * billiardB.m_elasticity;
	velocityA = aVelocityAlongTangent + bVelocityAlongNormal * elasticityCoefficient;
	velocityB = bVelocityAlongTangent + aVelocityAlongNormal * elasticityCoefficient;
}


void GameModeBilliards2D::CheckAndResolveCollisionBwBilliardAndBumper(GameDisk& billiard, GameDisk& bumper)
{
	if (!ShouldCorrectPosition(billiard, bumper))
		return;

	PushDiskOutOfDisk2D(billiard.m_center, billiard.m_radius, bumper.m_center, bumper.m_radius);

	if (!ShouldCorrectVelocity(billiard, bumper))
		return;

	Vec2& velocity = billiard.m_velocity;
	Vec2 impactNormal = (billiard.m_center - bumper.m_center).GetNormalized();
	Vec2 velocityAlongNormal = GetProjectedOnto2D(velocity, impactNormal);
	Vec2 velocityAlongTangent = velocity - velocityAlongNormal;

	float elasticityCoefficient = billiard.m_elasticity * bumper.m_elasticity;
	velocity = velocityAlongTangent - velocityAlongNormal * elasticityCoefficient;
}


void GameModeBilliards2D::CheckAndResolveCollisionBwBilliardAndWall(GameDisk& billiard)
{
	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);
	Vec2& center = billiard.m_center;
	float radius = billiard.m_radius;
	float elasticityCoefficient = billiard.m_elasticity * m_wallElasticity;
	if (center.x < radius)
	{
		center.x = radius;
		Vec2& velocity = billiard.m_velocity;
		Vec2 impactNormal = Vec2(1.0f, 0.0f);
		Vec2 velocityAlongNormal = GetProjectedOnto2D(velocity, impactNormal);
		Vec2 velocityAlongTangent = velocity - velocityAlongNormal;

		velocity = velocityAlongTangent - velocityAlongNormal * elasticityCoefficient;
	}

	if (center.y < radius)
	{
		center.y = radius;
		Vec2& velocity = billiard.m_velocity;
		Vec2 impactNormal = Vec2(0.0f, 1.0f);
		Vec2 velocityAlongNormal = GetProjectedOnto2D(velocity, impactNormal);
		Vec2 velocityAlongTangent = velocity - velocityAlongNormal;

		velocity = velocityAlongTangent - velocityAlongNormal * elasticityCoefficient;
	}

	if (worldX - center.x < radius)
	{
		center.x = worldX - radius;
		Vec2& velocity = billiard.m_velocity;
		Vec2 impactNormal = Vec2(-1.0f, 0.0f);
		Vec2 velocityAlongNormal = GetProjectedOnto2D(velocity, impactNormal);
		Vec2 velocityAlongTangent = velocity - velocityAlongNormal;

		velocity = velocityAlongTangent - velocityAlongNormal * elasticityCoefficient;
	}

	if (worldY - center.y < radius)
	{
		center.y = worldY - radius;
		Vec2& velocity = billiard.m_velocity;
		Vec2 impactNormal = Vec2(0.0f, -1.0f);
		Vec2 velocityAlongNormal = GetProjectedOnto2D(velocity, impactNormal);
		Vec2 velocityAlongTangent = velocity - velocityAlongNormal;

		velocity = velocityAlongTangent - velocityAlongNormal * elasticityCoefficient;
	}
}


void GameModeBilliards2D::GenerateRandomBumpers()
{
	m_disksByType[GAME_DISK_TYPE_BUMPER].clear();
	int minBumperCount = 10;
	int maxBumperCount = 15;
	int bumperCount = g_rng->GetRandomIntInRange(minBumperCount, maxBumperCount);
	for (int bumperIndex = 0; bumperIndex < bumperCount; bumperIndex++)
	{
		GenerateRandomBumper();
	}
}


GameDisk GameModeBilliards2D::GenerateRandomBumper()
{
	float worldX = g_gameConfigBlackboard.GetValue("worldSizeX", 0.0f);
	float worldY = g_gameConfigBlackboard.GetValue("worldSizeY", 0.0f);
	
	Rgba8 elasticColor = Rgba8::GREEN;
	Rgba8 inElasticColor = Rgba8::RED;

	float randomRadius = g_rng->GetRandomFloatInRange(3.0f, 6.0f);
	Vec2 randomCenter = g_rng->GetRandomPositionInAABB2(Vec2(randomRadius, randomRadius), Vec2(worldX - randomRadius, worldY - randomRadius));
	GameDisk bumper = {};
	bumper.m_center = randomCenter;
	bumper.m_radius = randomRadius;
	bumper.m_elasticity = g_rng->GetRandomFloatInRange(0.01f, 0.99f);
	bumper.m_color = Lerp(inElasticColor, elasticColor, bumper.m_elasticity);

	m_disksByType[GAME_DISK_TYPE_BUMPER].push_back(bumper);
	return bumper;
}


void GameModeBilliards2D::RaycastDisks()
{
	m_raycastResult = {};
	m_doesRaycastHit = false;
	Vec2 raycast = m_raycastEnd - m_raycastStart;
	Vec2 raycastDirection = raycast.GetNormalized();
	float raycastDistance = raycast.GetLength();

	float minDistance = 10000.0f;
	int hitDiskType = -1;
	int hitDiskIndex = -1;

	for (int diskType = 0; diskType < GAME_DISK_TYPE_COUNT; diskType++)
	{
		DiskList& diskList = m_disksByType[diskType];
		for (int diskIndex = 0; diskIndex < (int) diskList.size(); diskIndex++)
		{
			GameDisk& disk = diskList[diskIndex];
			BaseRaycastResult2D raycastResult = RaycastVsDisk2D(m_raycastStart, raycastDirection, raycastDistance, disk.m_center, disk.m_radius);
			if (raycastResult.m_didHit)
			{
				float impactDistance = raycastResult.m_impactDistance;
				if (impactDistance < minDistance)
				{
					minDistance = impactDistance;
					hitDiskIndex = diskIndex;
					hitDiskType = diskType;
					m_raycastResult = raycastResult;
				}
			}
		}
	}

	for (int diskType = 0; diskType < GAME_DISK_TYPE_COUNT; diskType++)
	{
		DiskList& diskList = m_disksByType[diskType];
		for (int diskIndex = 0; diskIndex < (int) diskList.size(); diskIndex++)
		{
			GameDisk& disk = diskList[diskIndex];
			disk.m_tint = Rgba8::WHITE;
			if (diskIndex == hitDiskIndex && diskType == hitDiskType)
			{
				disk.m_tint = m_hitDiskTint;
				m_doesRaycastHit = true;
			}
		}
	}
}


void GameModeBilliards2D::UpdateBilliards(float deltaSeconds)
{
	DiskList& billiards = m_disksByType[GAME_DISK_TYPE_BILLIARD];
	for (int billiardIndex = 0; billiardIndex < billiards.size(); billiardIndex++)
	{
		GameDisk& billiard = billiards[billiardIndex];
		Vec2& billiardCenter = billiard.m_center;
		billiardCenter += billiard.m_velocity * deltaSeconds;
	}
}
