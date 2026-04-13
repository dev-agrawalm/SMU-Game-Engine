#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Game/GameCommon.hpp"
#include <vector>
#include <string>

enum class GameState
{
	NONE,
	GAME_STATE_ATTRACT,
	GAME_STATE_LAUNCHER,
};

class Entity;
class Player;
class Prop;
class Clock;
class GameServer;

//class representing an instance of our game
//it is owned by App
class Game
{
public: //methods
	Game();
	~Game();

	void Startup();
	void ShutDown();
	void LoadAssets();
	void HandleQuitRequest();

	void CheckInputDeveloperCheats();
	void Update();
	void Render();

	void InitAttractMode();
	void CheckInputAttractMode();
	void UpdateAttractMode(float deltaSeconds);
	void RenderAttractMode();
	void DeInitAttractMode();

	void InitLauncher();
	void CheckInputLauncher();
	void UpdateLauncher(float deltaSeconds);
	void RenderLaunder();
	void DeInitLauncher();

	void SetGameState(GameState gameState);
	void SetGameTimeScale(float scale);
	Clock* GetClock() const;

protected:
	void RenderGrid() const;


private: //methods
	GameServer* m_server = nullptr;
	GameState m_gameState = GameState::NONE;
	
	Camera m_uiCamera;
	float m_uiCanvasSizeX = 0.0f;
	float m_uiCanvasSizeY = 0.0f;

	MouseInputData m_inputData;
	float m_movementCatchupT = 0.0f;

	Clock* m_gameClock = nullptr;

	bool m_showServerWindow = false;
	bool m_showClientWindow = false;
	std::vector<std::string> m_ipAddrs;
	char m_serverIpAddrStr[16] = {""};
	char m_clientIpAddrStr[16] = {""};
	char m_serverPortStr[16] = {""};
	char m_clientPortStr[16] = {""};
};
