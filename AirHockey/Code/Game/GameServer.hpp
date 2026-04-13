#pragma once
#include "Engine/Networking/Server.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/CircularBuffer.hpp"
#include "Engine/Core/BufferParser.hpp"
#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class Entity;
class Player;
class Prop;
class PlayerClient;
struct UDPDatagram;
struct GameMessage;
class VertexBuffer;
class ConstantBuffer;

enum TableEdge
{
	EDGE_NORTH				= 1 << 1,
	EDGE_NORTH_EAST			= 1 << 2,
	EDGE_NORTH_WEST			= 1 << 3,
	EDGE_SOUTH				= 1 << 4,
	EDGE_SOUTH_EAST			= 1 << 5,
	EDGE_SOUTH_WEST			= 1 << 6,
};


enum GameServerState
{
	SERVER_STATE_NONE,
	SERVER_STATE_LOBBY,
	SERVER_STATE_GAME
};


constexpr int EDGE_HIT_RESET_FRAME_COUNTER = 10;

class GameServer : public Server
{
public:
	GameServer(IPv4Address const& authServerAddr);
	GameServer(IPv4Address const& remoteServerAddr, IPv4Address const& authServerAddr);

	virtual void Startup() override;
	virtual void BeginFrame();
	virtual void CheckInput();
	virtual void Update();
	virtual void Render();
	virtual void EndFrame();
	virtual void Shutdown() override;

	void SendDisconnectMessageToOtherPlayer();
	void SetServerState(GameServerState newState);
	void TogglePause();
	void PauseGame();
	void UnpauseGame();

	void Startup_Lobby();
	void Update_Lobby(float deltaSeconds);
	void Render_Lobby();
	void Shutdown_Lobby();
	
	void Startup_Game();
	void Update_Game(float deltaSeconds);
	void Render_Game();
	void Shutdown_Game();

	void PlayerScoredPoints(int playerIndex, int wonPts);
	void RevivePuckForPlayer(int playerIndex);
	void SpawnEntity(Entity* entity);
	void PushEntitiesOutOfWalls();
	void PushEntityOutOfWalls_Puck(Entity* puck);
	void PushEntitiesOutOfEachOther();
	void BounceDiskOffSurface(float surfaceElasticity, float surfaceFriction, Vec3 const& impactNormal, float diskRadius, Vec3 const& in_linearVelocity, float in_angularVelocity, Vec3& out_linearVelocity, float& out_angularVelocity, bool reverseNormalVelocity = true);

	void AddMessageToDatagram(GameMessage const& message, UDPDatagram* datagram);
	void ReadMessageFromDatagram(GameMessage& out_message, UDPDatagram* datagram);

	bool IsEdgeHit(TableEdge edge);
	Mat44 GetCameraModelMatrix() const;
	bool IsServerActive() const;
	std::string GetDescription() const;
	bool IsGamePaused();
	bool IsServerRemote();
	AirHockeyTable GetTable();
private:
	void RenderGameScene();
	void RenderGameUI();
	void UpdateNeonShaderConstantBuffer();
	void UpdateTableVertexBuffer();
	void ExtractMessages_AuthServer(UDPDatagram* datagram);
	void ExtractMessages_RemoteServer(UDPDatagram* datagram);
	
	void UpdateShaderWindow();
	void UpdateDebugWindow();
	void UpdatePerformanceStats(float deltaSeconds);
	void InitialiseBuffer(CircularBuffer<float>& buffer);
	void ResizeBuffer(CircularBuffer<float>& buffer, int newSize);
	float GetSimpleWindowedAvgForCurrentFrame();
	float GetRecursiveWindowedAvgForCurrentFrame();
	float GetExponentialAvgForCurrentFrame();
	void InitialiseTableVBO();

private:
	GameServerState m_state = SERVER_STATE_NONE;
	bool m_isGamePaused = false;
	int m_pausingPlayerIndex = 0;
	int m_latestScoringPlayerIndex = -1;
	int m_isGameOver = false;

	uint32_t m_frameNumber = 0;
	bool m_isConnectedToOtherPlayer = false;
	bool m_isActive = false;
	bool m_isRemote = false;
	IPv4Address m_authServerAddr;
	IPv4Address m_remoteServerAddr;

	bool m_showShaderWindow = false;
	bool m_showDebugWindow = false;
	bool m_showFrameStats = false;
	bool m_isSinglePlayer = false;

	Clock* m_gameClock = nullptr;
	float m_physicsTimeStep = 0.015f;
	Stopwatch m_physicsTimer;

	uint8_t m_tableEdgesGotHitFlag = 0;
	int m_edgeHitResetFrameCounter = EDGE_HIT_RESET_FRAME_COUNTER;
	AirHockeyTable m_table;
	std::vector<Entity*> m_entities;
	Prop* m_puck = nullptr;

	Camera m_uiCamera;
	float m_uiCanvasSizeX = 0.0f;
	float m_uiCanvasSizeY = 0.0f;
	
	Camera m_worldCamera;
	Mat44 m_camModelMatrix;

	//performance
	float m_perfFrameCount = 0.0f;
	CircularBuffer<float> m_frames;
	CircularBuffer<float> m_frameTimes;
	CircularBuffer<float> m_simpleWindowedAvgs;
	CircularBuffer<float> m_recursiveWindowedAvgs;
	CircularBuffer<float> m_exponentialAvgs;
	int m_numSamples = -1;
	int m_windowSize = -1;
	int m_numBins = -1;
	float m_exponentialFilter = 0.0f;

	bool m_gameUpdateMsgRcevd = false;

	bool m_renderScreenTexture = true;
	bool m_renderScene = true;
	std::vector<Vertex_PCU> m_tableVerts;
	VertexBuffer* m_tableVBO = nullptr;
	ConstantBuffer* m_neonGlowShaderBuffer = nullptr;
	int m_numVertsPerEdge = 0;
	int m_totalNumVerts = 0;
	int m_southEdgeVertsStartIndex = 0;
	int m_southEastEdgeVertsStartIndex = 0;
	int m_southWestEdgeVertsStartIndex = 0;
	int m_northEdgeVertsStartIndex = 0;
	int m_northEastEdgeVertsStartIndex = 0;
	int m_northWestEdgeVertsStartIndex = 0;

	AudioListener m_playerAudioListener = {};
	SoundPlaybackID m_gameBgMusicPlaybackId = 0;
};
