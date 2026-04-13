#pragma once
#include "Engine/Networking/Client.hpp"
#include "Engine/Math/Vec3.hpp"

class Player;
class GameServer;

class PlayerClient : public Client
{
public:
	static const Vec3 s_INVALID_POS;

public:
	PlayerClient(Server* server, IPv4Address const& clientAddr, IPv4Address const& serverAddr, bool isPlayer1);
	virtual void Startup() override;
	virtual void Shutdown() override;

	void Update(float deltaSeconds);

private:
	bool m_isPlayer1 = false;
	Player* m_player = nullptr;
	GameServer* m_gameServer = nullptr;

	Vec3 m_pos0 = s_INVALID_POS;
	float m_deltaSec1 = 0.0f;
	Vec3 m_pos1 = s_INVALID_POS;
	float m_deltaSec2 = 0.0f;
	Vec3 m_pos2 = s_INVALID_POS;
};
