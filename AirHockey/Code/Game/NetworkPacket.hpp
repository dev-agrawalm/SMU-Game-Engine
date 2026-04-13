#pragma once


enum GameMessageType : uint32_t
{
	GAME_MESSAGE_JOIN_REQUEST = 0,
	GAME_MESSAGE_JOIN_ACCEPT,
	GAME_MESSAGE_GAME_START,
	GAME_MESSAGE_GAME_UPDATE,
	GAME_MESSAGE_GAME_PAUSE,
	GAME_MESSAGE_GAME_RESUME,
	GAME_MESSAGE_SERVER_DISCONNECT,
	GAME_MESSAGE_SERVER_DISCONNECT_ACK,
};

struct GameMessage
{
public:
	GameMessageType m_type;
	uint32_t m_frameNumber = 0;

	Vec3 m_player1CurPos;
	Vec3 m_player1PrevPos;
	Vec3 m_player1Velocity;
	int m_player1Score = 0;
	
	Vec3 m_player2CurPos;
	Vec3 m_player2PrevPos;
	Vec3 m_player2Velocity;
	int m_player2Score = 0;
	
	Vec3 m_puckCurPos;
	Vec3 m_puckPrevPos;
	Vec3 m_puckVelocity;
	float m_puckAngularVelocity = 0.0f;
	float m_puckOrientation = 0.0f;
	bool m_isPuckDead = false;

	int m_latestScoringPlayerIndex = -1;
	bool m_isGameOver = false;
};
