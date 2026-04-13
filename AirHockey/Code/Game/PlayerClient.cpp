#include "Game/PlayerClient.hpp"
#include "Game/Player.hpp"
#include "Game/GameServer.hpp"
#include "Engine/Input/InputSystem.hpp"

const Vec3 PlayerClient::s_INVALID_POS = Vec3(-999.0f, -999.0f, -999.0f);

PlayerClient::PlayerClient(Server* server, IPv4Address const& clientAddr, IPv4Address const& serverAddr, bool isPlayer1)
	: Client(server, clientAddr, serverAddr)
	, m_isPlayer1(isPlayer1)
{
}


void PlayerClient::Startup()
{
	Vec3 startPos = m_isPlayer1 ? Vec3(0.0f, -10.0f, 0.0f) : Vec3(0.0f, 10.0f, 0.0f);
	m_player = new Player(startPos);
	m_gameServer = dynamic_cast<GameServer*>(m_server);
	m_gameServer->SpawnEntity(m_player);
}


void PlayerClient::Shutdown()
{
	m_player = nullptr;
	m_gameServer = nullptr;
}


void PlayerClient::Update(float deltaSeconds)
{
	if (m_gameServer->IsGamePaused())
		return;


	bool isServerRemote = m_gameServer->IsServerRemote();
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_SPACE_BAR))
	{
		int playerIndex = isServerRemote ? 2 : 1;
		m_gameServer->RevivePuckForPlayer(playerIndex);
	}

	if (g_inputSystem->WasKeyJustPressed('B') && !isServerRemote)
	{
		m_player->m_score++;
	}

	if (g_inputSystem->WasKeyJustPressed('F') && isServerRemote)
	{
		m_player->m_score++;
	}

	Vec3 originalPlayerPos = m_player->GetPosition();
	Mat44 camModelMatrix = m_gameServer->GetCameraModelMatrix();
	Vec3 newPlayerPos = GetCurrentMousePos_ScreenToWorld(camModelMatrix);
	if(g_gameConfigBlackboard.GetValue("clampPaddleToDeadZone", true))
	{
		AirHockeyTable table = m_gameServer->GetTable();
		float deadZoneHeight = g_gameConfigBlackboard.GetValue("deadZoneHeight", 0.0f);
		if (isServerRemote)
		{
			float maxY = table.m_center.y + table.m_height * 0.5f;
			float minY = table.m_center.y + deadZoneHeight * 0.5f;
			newPlayerPos.y = Clamp(newPlayerPos.y, minY, maxY);
		}
		else
		{
			float minY = table.m_center.y - table.m_height * 0.5f;
			float maxY = table.m_center.y - deadZoneHeight * 0.5f;
			newPlayerPos.y = Clamp(newPlayerPos.y, minY, maxY);
		}
	}

	static float tableWidth = g_gameConfigBlackboard.GetValue("tableWidth", 32.0f);
	static float tableHeight = g_gameConfigBlackboard.GetValue("tableHeight", 128.0f);
	static Vec2 tableMins = Vec2(-tableWidth, -tableHeight) * 0.5f;
	static Vec2 tableMaxs = Vec2(tableWidth, tableHeight) * 0.5f;
	Vec2 newPositionXY = newPlayerPos.GetVec2();
	Vec2 correctedPosXY = newPositionXY;
	float physicsRadius = m_player->GetPhysicsRadius();
	if (newPositionXY.x - physicsRadius < tableMins.x)
	{
		correctedPosXY.x = tableMins.x + physicsRadius;
	}
	if (newPositionXY.x + physicsRadius > tableMaxs.x)
	{
		correctedPosXY.x = tableMaxs.x - physicsRadius;
	}

	if (newPositionXY.y - physicsRadius < tableMins.y)
	{
		correctedPosXY.y = tableMins.y + physicsRadius;
	}
	if (newPositionXY.y + physicsRadius > tableMaxs.y)
	{
		correctedPosXY.y = tableMaxs.y - physicsRadius;
	}
	Vec3 correctPos = correctedPosXY.GetVec3();
	m_player->m_prevPos = originalPlayerPos;
	m_player->SetPosition(correctPos);
	m_player->m_neonTrailPositions.Push(correctPos);

	if (m_pos0 == s_INVALID_POS)
	{
		m_pos0 = newPlayerPos;
		m_player->SetVelocity(Vec3::ZERO);
	}
	else if (m_pos1 == s_INVALID_POS)
	{
		m_pos1 = newPlayerPos;
		m_deltaSec1 = deltaSeconds;
		Vec3 displacement = m_pos1 - m_pos0;
		Vec3 velocity = displacement / deltaSeconds;
		m_player->SetVelocity(velocity);
	}
	else
	{
		m_pos2 = newPlayerPos;
		m_deltaSec2 = deltaSeconds;
		Vec3 displacement = m_pos2 - m_pos0;
		float totalTime = m_deltaSec1 + m_deltaSec2;
		Vec3 velocity = displacement / totalTime;
		m_player->SetVelocity(velocity);
		m_pos0 = m_pos1;
		m_pos1 = m_pos2;
		m_deltaSec1 = m_deltaSec2;
	}
}
