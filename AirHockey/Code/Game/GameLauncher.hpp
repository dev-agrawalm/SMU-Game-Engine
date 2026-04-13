#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Game/GameCommon.hpp"
#include <vector>
#include <string>
#include "Engine/Audio/AudioSystem.hpp"

enum class GameLauncherState
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
class GameLauncher
{
public: //methods
	GameLauncher();
	~GameLauncher();

	void Startup();
	void ShutDown();
	void LoadAssets();
	void HandleQuitRequest();

	void CheckInputDeveloperCheats();
	void Update();
	void Render();

	void InitAttractMode();
	void CheckInputAttractMode();
	void UpdateAttractMode();
	void RenderAttractMode();
	void DeInitAttractMode();

	void InitLauncher();
	void CheckInputLauncher();
	void UpdateLauncher();
	void RenderLauncher();
	void DeInitLauncher();

	void SetGameState(GameLauncherState gameState);
	//void SetGameTimeScale(float scale);

protected:
	void RenderGrid() const;

public:
	float m_otherPlayerQuitTextFadeoutDuration = 0.0f;
	SoundPlaybackID m_launcherBgMusicPlaybackId = 0;

private: //methods
	GameServer* m_server = nullptr;
	GameLauncherState m_gameState = GameLauncherState::NONE;
	Clock* m_launcherClock;
	
	Camera m_uiCamera;
	float m_uiCanvasSizeX = 0.0f;
	float m_uiCanvasSizeY = 0.0f;

	bool m_showServerWindow = false;
	bool m_showClientWindow = false;
	std::vector<std::string> m_ipAddrs;
	char m_serverIpAddrStr[16] = {""};
	char m_clientIpAddrStr[16] = {""};
	char m_serverPortStr[16] = {""};
	char m_clientPortStr[16] = {""};

};
